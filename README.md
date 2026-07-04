# reme

This is a small reminder tool written in C. Its structure is mainly client and server (daemon).

| reme                                                              | remed                                          |
| ----------------------------------------------------------------- | ---------------------------------------------- |
| This is the binary that sends your reminder prompt to the server. | This is the server that listens to the client. |

## Requirements

- Linux or UNIX-like system
- gcc
- make
- POSIX threads
- notify-send
- systemd
- any notification daemon

## Installation

Clone the repository:

```bash
git clone https://github.com/pinxau1/reme.git
cd reme
```

Inside the directory:

```bash
make install
```

This installs the binaries and sets up the systemd user service.
To start using do the this:

```bash
make enable
```

To uninstall:

```bash
make uninstall
```

To disable

```bash
make disable
```

## Usage

Flags and examples:

| Flag | Example                       | Description                                                                                       |
| ---- | ----------------------------- | ------------------------------------------------------------------------------------------------- |
| -t   | reme -t "take a bath" 11:00am | Adds a reminder that triggers on 11:00am. Removing -t flag allows non-meridiem format (no AM/PM). |
| -l   | reme -l                       | Shows a list of reminders yet to be triggered. (Use prior to deletion).                           |
| -d   | reme -d 1                     | Deletes number 1 in the `reme -l` list.                                                           |
| -h   | reme -h                       | Shows help to remind you of the usage.                                                            |

## Notifications

When a reminder is triggered, it sends through `notify-send`. You can customize the appearance of a `reme` notification. The specified app-name is "reme".

Example for mako notification:

```ini
[app-name="reme"]
anchor=center
```
