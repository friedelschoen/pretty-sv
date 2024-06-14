#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

struct service_serial {
	uint8_t status_change[8];
	uint8_t status_change_ms[4];
	uint8_t pid[4];
	uint8_t paused;
	uint8_t wantsup;
	uint8_t terminated;
	uint8_t state;
};

void printstatus(const char* path, struct service_serial* buffer) {
	const char* name;
	if ((name = strrchr(path, '/'))) {
		if (!strcmp(name, "/log")) {
			while (--name > path) {
				if (*name == '/')
					break;
			}
			name++;
		} else {
			name++;
		}
	} else
		name = path;

	char* home   = getenv("HOME");
	int   isuser = home != NULL && strncmp(home, path, strlen(home)) == 0;

	if (isuser) {
		printf("user  ");
	} else {
		printf("sys   ");
	}

	printf("%-20s ", name);

	// wants up and is up
	// wants down and is down
	if ((buffer->wantsup == 'd') == (buffer->state == 0))
		printf("= ");
	// wants down and is up
	else if (buffer->wantsup == 'd')
		printf("v ");
	// wants up and is down
	else if (buffer->state == 0)
		printf("^ ");

	if (buffer->paused)
		printf("paus  ");
	else if (buffer->state == 0)
		printf("down  ");
	else if (buffer->state == 1)
		printf("run   ");
	else if (buffer->state == 2)
		printf("fin   ");
	else
		printf("???   ");

	uint64_t tai = ((uint64_t) buffer->status_change[0] << 56) |
	               ((uint64_t) buffer->status_change[1] << 48) |
	               ((uint64_t) buffer->status_change[2] << 40) |
	               ((uint64_t) buffer->status_change[3] << 32) |
	               ((uint64_t) buffer->status_change[4] << 24) |
	               ((uint64_t) buffer->status_change[5] << 16) |
	               ((uint64_t) buffer->status_change[6] << 8) |
	               ((uint64_t) buffer->status_change[7] << 0);

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

	if (buffer->state == 1) {
		pid_t pid = (buffer->pid[0] << 0) |
		            (buffer->pid[1] << 8) |
		            (buffer->pid[2] << 16) |
		            (buffer->pid[3] << 24);

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

int main(int argc, char** argv) {
	char  path[PATH_MAX];
	int   fd;
	char* basename;

	struct service_serial statusbuf;

	for (int i = 1; i < argc; i++) {
		snprintf(path, sizeof path, "%s/supervise/status", argv[i]);
		if ((fd = open(path, O_RDONLY)) == -1) {
			fprintf(stderr, "%s: unable to open supervise/status\n", argv[i]);
			continue;
		}
		if (read(fd, &statusbuf, sizeof statusbuf) != sizeof statusbuf) {
			fprintf(stderr, "%s: unable to read status\n", argv[i]);
			continue;
		}
		close(fd);

		printstatus(argv[i], &statusbuf);
	}
}
