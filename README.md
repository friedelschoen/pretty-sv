# psvstat – Pretty Service Status

`psvstat` is a small command-line utility that displays the status of [runit](http://smarden.org/runit/) services in a readable, concise format. It reads the binary `supervise/status` file of each given service directory and extracts details like current state, desired state, time since last change, PID, and command line.

## Features

- Distinguishes between user and system services
- Displays:
  - Service name
  - Desired and actual state
  - Paused state
  - Time since last status change
  - PID and command (if running)
- Supports sorting system/user services first
- Optionally includes log sub-services
- Compact output, easy to parse visually or with tools

## Compilation

To build and install `psvstat`, run:

```sh
make
make PREFIX=$HOME/.local install
````

This installs the binary into `$PREFIX/bin` and the man page into `$PREFIX/share/man`.

## Usage

```sh
psvstat [options] [directories...]
```

Each argument should be a path to a `runit`-style service directory containing a `supervise/status` file.

### Options

| Option      | Description                                    |
| ----------- | ---------------------------------------------- |
| `-h`        | Show help and exit                             |
| `-H <path>` | Set home path to determine user services       |
| `-l`        | Include `log` subdirectories as services       |
| `-s`        | Sort with system services first                |
| `-u`        | Sort with user services first                  |
| `-c <n>`    | Set max bytes to read from `/proc/PID/cmdline` |

Note: `-s` and `-u` are mutually exclusive.

## Output

Each service line contains:

1. **Type**: `usr` for user, `sys` for system
2. **Name**: basename of the service directory
3. **Desired state**: one of:

   * `=` – desired and actual match
   * `v` – wants down but is up
   * `^` – wants up but is down
4. **Current state**: one of:

   * `run`, `down`, `fin` (finished), or `???` (unknown)
5. **Paused**: shows `paus` if paused
6. **Since**: time since last state change
7. **PID**: if running
8. **Command**: if available from `/proc/PID/cmdline`

### Example

```sh
sys   webserver            = run   2 hours     1234   nginx -g daemon off;
usr   editor-service       ^ down  5 minutes   ---    ---
```

* `webserver` is a system service, currently running as expected.
* `editor-service` is a user service that is down but should be running.

## Error Handling

Errors are reported to stderr, for example:

```sh
/etc/service/myapp: unable to open status-file: No such file or directory
/home/user/svc/abc: unable to read status-file: Input/output error
```

## Motivation

I'm using [`stw`](https://github.com/sineemore/stw/) with `psvstat` to list my services. I'm using runit
as my system service-supervisor ([Void Linux](https://voidlinux.org/)) and as user supervisor for my programs
like my [dwm](http://dwm.suckless.org/) or applets.

## License

This project is licensed under the Zlib License. See the [LICENSE](LICENSE) file for details.

## Author

Written by Friedel Schon.

## Acknowledgments

Thanks to the `runit` developers for their clean, simple design. This tool builds on the reliability of that foundation.