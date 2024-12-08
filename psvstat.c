#include "arg.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAXSERVICES 100


struct serviceserial {
	uint8_t status_change[8];
	uint8_t status_change_ms[4];
	uint8_t pid[4];
	uint8_t paused;
	uint8_t wantsup;
	uint8_t terminated;
	uint8_t state;
};

struct serviceinfo {
	char*                name;
	int                  isuser;
	struct serviceserial serial;
};


static const char*        home = NULL;
static struct serviceinfo services[MAXSERVICES];
static int                nservices = 0;
static int                sortuser = 0, sortsys = 0;

static int getservice(struct serviceinfo* info, const char* service) {
	char        path[PATH_MAX];
	FILE*       fp;
	const char* name;

	/* read status */
	snprintf(path, sizeof path, "%s/supervise/status", service);
	if (!(fp = fopen(path, "r"))) {
		if (errno != ENOENT && errno != EISDIR)
			fprintf(stderr, "%s: unable to open status-file: %s\n", service, strerror(errno));
		return -1;
	}
	if (fread(&info->serial, sizeof(info->serial), 1, fp) != 1) {
		fprintf(stderr, "%s: unable to read status-file: %s\n", service, strerror(ferror(fp)));
		return -1;
	}
	fclose(fp);

	/* isuser */
	if (!realpath(service, path)) {
		fprintf(stderr, "%s: unable to get absolute path of: %s\n", service, strerror(errno));
		return -1;
	}

	info->isuser = home != NULL && !strncmp(path, home, strlen(home));

	if ((name = strrchr(service, '/'))) {
		if (!strcmp(name, "/log")) {
			while (--name > service) {
				if (*name == '/')
					break;
			}
		}
		name++;
	} else
		name = service;

	info->name = strdup(name);
	return 0;
}

static void printstatus(struct serviceinfo* service) {
	if (service->isuser)
		printf("user  ");
	else
		printf("sys   ");

	printf("%-20s ", service->name);

	// wants up and is up
	// wants down and is down
	if ((service->serial.wantsup == 'd') == (service->serial.state == 0))
		printf("= ");
	// wants down and is up
	else if (service->serial.wantsup == 'd')
		printf("v ");
	// wants up and is down
	else if (service->serial.state == 0)
		printf("^ ");

	if (service->serial.state == 0)
		printf("down  ");
	else if (service->serial.state == 1)
		printf("run   ");
	else if (service->serial.state == 2)
		printf("fin   ");
	else
		printf("???   ");

	if (service->serial.paused)
		printf("paus ");
	else
		printf("     ");

	uint64_t tai = ((uint64_t) service->serial.status_change[0] << 56) |
	               ((uint64_t) service->serial.status_change[1] << 48) |
	               ((uint64_t) service->serial.status_change[2] << 40) |
	               ((uint64_t) service->serial.status_change[3] << 32) |
	               ((uint64_t) service->serial.status_change[4] << 24) |
	               ((uint64_t) service->serial.status_change[5] << 16) |
	               ((uint64_t) service->serial.status_change[6] << 8) |
	               ((uint64_t) service->serial.status_change[7] << 0);

	time_t      timediff  = time(NULL) - tai + 4611686018427387914ULL;
	const char* timediffu = timediff == 1 ? "second" : "seconds";
	if (timediff >= 60) {
		timediff /= 60;
		timediffu = timediff == 1 ? "minute" : "minutes";
		if (timediff >= 60) {
			timediff /= 60;
			timediffu = timediff == 1 ? "hour" : "hours";
			if (timediff >= 24) {
				timediff /= 24;
				timediffu = timediff == 1 ? "day" : "days";
			}
		}
	}
	char timediffstr[20];
	snprintf(timediffstr, sizeof timediffstr, "%ld %s", timediff, timediffu);

	printf("%-11s ", timediffstr);

	if (service->serial.state == 1) {
		pid_t pid = (service->serial.pid[0] << 0) |
		            (service->serial.pid[1] << 8) |
		            (service->serial.pid[2] << 16) |
		            (service->serial.pid[3] << 24);

		printf("%-5d  ", pid);

		int  procfd;
		char procpath[PATH_MAX];
		char cmdline[1024];
		int  nread;
		snprintf(procpath, sizeof procpath, "/proc/%d/comm", pid);

		if ((procfd = open(procpath, O_RDONLY)) != -1) {
			nread = read(procfd, cmdline, sizeof cmdline);
			if (nread < 0) nread = 0;
			if (nread == sizeof cmdline) {
				strcpy(&cmdline[sizeof cmdline - 4], "...");
			} else {
				nread--;
				cmdline[nread] = '\0';
			}
			close(procfd);
			printf("%s", cmdline);
		} else {
			printf("---");
		}
	} else {
		printf("---    ---");
	}
	printf("\n");
}

static int scanservices(const char* directory) {
	char           path[PATH_MAX];
	DIR*           dp;
	struct dirent* ep;

	if (!getservice(&services[nservices], directory))
		nservices++;

	if (!(dp = opendir(directory))) {
		fprintf(stderr, "%s: unable to open directory: %s\n", directory, strerror(errno));
		return -1;
	}

	while ((ep = readdir(dp))) {
		if (ep->d_name[0] == '.')
			continue;

		snprintf(path, sizeof(path), "%s/%s", directory, ep->d_name);
		if (!getservice(&services[nservices], path))
			nservices++;
	}

	closedir(dp);
	return 0;
}

static int servicecmp(const void* pleft, const void* pright) {
	const struct serviceinfo* left  = pleft;
	const struct serviceinfo* right = pright;

	if ((sortuser || sortsys) && left->isuser != right->isuser)
		return sortsys
		         ? left->isuser - right->isuser
		         : right->isuser - left->isuser;

	return strcmp(left->name, right->name);
}

static __attribute__((noreturn)) void usage(int exitcode) {
	fprintf(stderr, "usage: psvstat [-hsu] [-H home] [directories...]\n");

	exit(exitcode);
}

int main(int argc, char** argv) {
	char  path[PATH_MAX];
	FILE* fp;

	ARGBEGIN
	switch (OPT) {
		case 'h':
			usage(0);
			break;
		case 'H':
			home = EARGF(usage(1));
			break;
		case 's':
			sortsys = 1;
			break;
		case 'u':
			sortuser = 1;
			break;
	}
	ARGEND

	if (!home)
		home = getenv("HOME"); /* fill global variable */

	for (int i = 0; i < argc; i++)
		scanservices(argv[i]);

	qsort(services, nservices, sizeof(*services), servicecmp);

	for (int i = 0; i < nservices; i++)
		printstatus(&services[i]);

	for (int i = 0; i < nservices; i++)
		free(services[i].name);
}
