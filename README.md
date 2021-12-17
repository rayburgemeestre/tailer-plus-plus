## `tailer++` - tail entire directories at once using inotify

Conceptually `tailer++` is equivalent to `tail -F -n 0 *`, which also tails
multiple files at once, but with one limitation, it won't pick up newly created
files. That limitation was the main reason for making this utility.

```
Usage: ./tailer <directory | file> [ <server:port> [ <timeserver> ] ]
  e.g. ./tailer /var/log
  e.g. ./tailer /var/log 127.0.0.1:3456
  e.g. ./tailer /var/log 127.0.0.1:3456 time.xyz.com:123


When providing a server, an NTP time is fetched so we can
properly timestamp our logs as we send them to the server.
Default timeserver used is: time.google.com:123.
```

This program does not have any CPU usage, only when it reacts to inotify events.


### Fork

Forked my own project `tailer` (https://github.com/rayburgemeestre/tailer),
in order to keep that project bloat-free, and add opinionated features such as
centralized-logging capabilities to `tailer++`.

Thanks to Erik Zenker's:

https://github.com/erikzenker/inotify-cpp

Thanks to Angelos Plastropoulos':

https://github.com/plusangel/NTP-client

Lastly, using my own simple socket helper library:

https://github.com/rayburgemeestre/beej-plus-plus

Everything is statically linked, so we don't introduce any dependencies.


## Implementation

Additionally to `tailer`'s implementation, we support centralized logging if specified via the commandline.

For centralized logging, the program first queries an NTP server to get the correct time at startup.
Note that system time won't be affected. This query is used to make sure we can prepend seconds with, with microsecond precision to each log line, so the centralized logging server can in theory sort log messages from various sources in chronological order somewhat (in case of lag).

To integrate such centralized logging in a program, you can use the `test_server` example created here: https://github.com/rayburgemeestre/beej-plus-plus/.
This example by itself is sufficient to receive log messages, as there is no fancy encoding/decoding necessary.


## Binary distribution

I've tagged version v-1.0.0. and uploaded a release on GitHub:

https://github.com/rayburgemeestre/tailer-plus-plus/releases/tag/v-1.0.0

If you trust me you can use the binary, otherwise you have to compile it
yourself, with instructions below. :)


## Compile instructions for Ubuntu 20.04

Sorry, currently only tested on Ubuntu 20.04. If you get this thing working on
another distro, please let me know how, so I can extend the instructions.

    git clone --recursive https://github.com/rayburgemeestre/tailer-plus-plus tailer++
    cd tailer++
    make prepare
    make build
    make install


## Usage example

Additionally to the usage example from the `tailer` project, `tailer++` supports:

	trigen@ideapad:~/projects/tailer++[master]> tailer++ /var/log localhost:10000
	Log Server: localhost, 10000
	Time Server host: time.google.com, 216.239.35.4
	Time Server port: 123
	NTP Time: 1639772323472.315186
	Our Host Name: ideapad

Then a server listening on this port:

	trigen@ideapad:~/projects/beej++[master]> ./build/test_server
	Received line: 2096.851318 [ideapad] Initialized 296 files.
	Received line: 2100.044189 [ideapad] Listening with inotify.. Press Control+C to stop the process.
	Received line: 2100.143311 [ideapad] ---
	Received line: 2665.495117 [ideapad] syslog: Dec 17 21:20:22 ideapad wpa_supplicant[1426]: wlo1: CTRL-EVENT-SIGNAL-CHANGE above=0 signal=-76 noise=9999 txrate=43300
	Received line: 0395.988281 [ideapad] syslog: Dec 17 21:20:30 ideapad wpa_supplicant[1426]: wlo1: CTRL-EVENT-SIGNAL-CHANGE above=1 signal=-65 noise=9999 txrate=14400
	Received line: 6248.958252 [ideapad] syslog: Dec 17 21:20:56 ideapad earlyoom[1367]: mem avail: 11196 of 13900 MiB (80 %), swap free: 24558 of 24575 MiB (99 %)
	Received line: 7579.637207 [ideapad] syslog: Dec 17 21:20:57 ideapad wpa_supplicant[1426]: wlo1: CTRL-EVENT-SIGNAL-CHANGE above=0 signal=-81 noise=9999 txrate=28900
	Received line: 1202.739258 [ideapad] auth.log: Dec 17 21:21:01 ideapad CRON[957960]: pam_unix(cron:session): session opened for user trigen by (uid=0)
	Received line: 1208.259277 [ideapad] syslog: Dec 17 21:21:01 ideapad CRON[957962]: (trigen) CMD (/home/trigen/.bin/xprof primary 6500 +0 1 1 1 2>&1 1>/tmp/test.log)
	Received line: 1411.695312 [ideapad] auth.log: Dec 17 21:21:01 ideapad CRON[957960]: pam_unix(cron:session): session closed for user trigen
	Received line: 1411.791260 [ideapad] syslog: Dec 17 21:21:01 ideapad CRON[957960]: (CRON) info (No MTA installed, discarding output)
	Received line: 5299.497314 [ideapad] syslog: Dec 17 21:21:05 ideapad wpa_supplicant[1426]: wlo1: CTRL-EVENT-SIGNAL-CHANGE above=1 signal=-64 noise=9999 txrate=39000

The program is stopped with CONTROL+C.
