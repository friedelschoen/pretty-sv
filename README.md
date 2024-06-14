# psvstat - Pretty Service Status

`psvstat` is a command-line utility for checking and printing the status of `runit` services. This tool reads the service status from the `supervise/status` file of each specified `runit` service and displays relevant information in a user-friendly format.

## Features

- Displays whether the service is a user service or a system service.
- Shows the service name, desired state, and current state.
- Indicates if the service is paused, down, running, or finished.
- Provides the time elapsed since the last status change.
- Displays the process ID and command name of the running service.

## Compilation

To compile the program, run the following command:

```sh
$ make
$ make PREFIX=$HOME/.local install
```

## Usage

The `psvstat` program takes one or more arguments, each representing the path to a `runit` service directory. For example:

```sh
psvstat /etc/service/service1 /etc/service/service2
```

### Arguments

- **service_path**: The path to the `runit` service directory. The program expects the status file to be located at `service_path/supervise/status`.

## Output

The program prints the status of each specified service in a formatted manner. The output contains the following columns:

1. **Type**: Indicates if the service is a user service (`user`) or a system service (`sys`).
2. **Name**: The name of the service.
3. **Desired State**: Indicates if the service's desired state matches its current state.
   - `=`: The service's desired state matches its current state.
   - `v`: The service is up but should be down.
   - `^`: The service is down but should be up.
4. **Current State**: The current state of the service.
   - `paus`: The service is paused.
   - `down`: The service is down.
   - `run`: The service is running.
   - `fin`: The service has finished.
   - `???`: Unknown state.
5. **Time Since Last Change**: The time elapsed since the last status change.
6. **PID**: The process ID of the service if it is running.
7. **Command**: The command name of the running process if available.

### Example Output

```sh
sys   service1            = run   2 hours     1234   myservice
sys   service2            ^ down  5 minutes   ---    ---
```

In this example:
- `service1` is a system service (`sys`), its desired state matches its current state (`=`), it is currently running (`run`), the status changed 2 hours ago, its PID is 1234, and the command name is `myservice`.
- `service2` is a system service (`sys`), its desired state does not match its current state (`v`), it should be down but is up, the status changed 5 minutes ago, it is not running (`---`), and no command name is available (`---`).

## Error Handling

The program handles several error conditions and prints appropriate messages to `stderr`:

- If it is unable to open the `supervise/status` file, it prints:
  ```
  <service_path>: unable to open supervise/status
  ```
- If it is unable to read the `supervise/status` file, it prints:
  ```
  <service_path>: unable to read status
  ```

## Environment Variables

- **HOME**: Used to determine if a service is a user service.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please open an issue or submit a pull request on GitHub.

## Author

Written by Friedel Schon.

## Acknowledgments

Thanks to the developers and community of `runit` for their excellent software and documentation.