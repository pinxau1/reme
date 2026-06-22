# reme

`reme` is a small reminder tool written in C. It has a command line client and
a background daemon.

The client is `reme`. It sends reminder requests to the daemon.

The daemon is `remed`. It stores reminders, waits for them to become due, and
sends desktop notifications.

## Requirements

- A Linux or Unix-like system
- `gcc`
- `make`
- POSIX threads
- `notify-send`
- Mako or another desktop notification daemon

## Build

Run `make` from the project directory.

This builds two programs:

- `reme`
- `remed`

## Running

Start `remed` before using `reme`.

By default, `remed` runs as a daemon. Use verbose mode when you want it to stay
attached to the terminal.

The client talks to the daemon through this Unix socket:

```text
/tmp/reme.sock
```

If the daemon is not running, the client cannot add, list, or delete reminders.

## Commands

`reme` can add a reminder with a message, time, and optional date.

Times use 24-hour format by default:

```text
HH:MM
```

Dates use this format:

```text
MM/DD/YYYY
```

The `-t` flag enables 12-hour time input with `AM` or `PM`.

The `-l` flag lists active reminders. It can also filter reminders by message
text.

The `-d` flag deletes an active reminder by its list number.

The `-h` flag prints the built-in help text.

The `-e` flag exists in the client, but edit support is not implemented yet.

## Notifications

When a reminder is due, `remed` prints the reminder message and calls
`notify-send`.

The notification app name is `reme`, and the notification title is `Reme`.

For Mako, this rule centers `reme` notifications:

```ini
[app-name="reme"]
anchor=center
```

## Storage

Reminders are stored in `reminders.txt` in the directory where `remed` is run.

Each line uses this format:

```text
state|message|HH:MM|MM/DD/YYYY
```

The state is `a` for active or `d` for inactive.

Deleted reminders are marked inactive. Fired reminders are also marked inactive.
They stay in the file instead of being removed.

## Notes

`reme` rejects reminders in the past.

Invalid dates and times are rejected before they are sent to the daemon.

The daemon removes any old `/tmp/reme.sock` file when it starts.

Notification delivery depends on the desktop session and notification daemon.
