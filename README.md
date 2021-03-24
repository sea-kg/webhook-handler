# webhook-handler

HTTP endpoints (hooks) - no ssl configuration (use a http-server with proxy)

<!-- 
Look here https://github.com/topics/webhook
-->

[![Build Status](https://api.travis-ci.com/sea-kg/webhook-handler.svg?branch=master)](https://travis-ci.com/sea-kg/webhook-handler.svg) [![Github Stars](https://img.shields.io/github/stars/sea-kg/webhook-handler.svg?label=github%20%E2%98%85)](https://github.com/sea-kg/webhook-handler) [![Github Stars](https://img.shields.io/github/contributors/sea-kg/webhook-handler.svg)](https://github.com/sea-kg/webhook-handler) [![Github Forks](https://img.shields.io/github/forks/sea-kg/webhook-handler.svg?label=github%20forks)](https://github.com/sea-kg/webhook-handler/network/members) [![Total alerts](https://img.shields.io/lgtm/alerts/g/sea-kg/webhook-handler.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/sea-kg/webhook-handler/alerts/) [![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/sea-kg/webhook-handler.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/sea-kg/webhook-handler/context:cpp)

## Install

### Ubuntu PPA
[Ubuntu. PPA](https://launchpad.net/~sea5kg/+archive/ubuntu/webhook-handler)

```
$ sudo add-apt-repository ppa:sea5kg/webhook-handler
$ sudo apt-get update
$ sudo apt-get install webhook-handler
```


## Build on debian/ubuntu

Install requriments
```
$ apt-get -y update && apt-get install -y \
    make cmake \
    g++ \
    pkg-config
```

Build
```
$ ./clean.sh && ./build_simple.sh
```

## Run

```
./webhook-handler --dir ./data start
```

## Configuration examples

File ./data/webhook-handler-conf.yml (or server will try search in /etc/webhook-handler):

```yaml
# server configuration
server:
  port: 8002
  wait-seconds-between-run-scripts: 60
  max-script-threads: 1
  max-deque: 100
  allow-status-page: yes
  status-page-url-path: "/wh/status" # will work if 'allow-status-page: yes'
  log-dir: "./logs"
```

Where

* `server/port` - number, web port (default 8002)
* `server/allow-status-page` - boolean, enable or disable status page (default: no)
* `server/status-page-url-path` - string, path to page with status of server (default: "/wh/status")
* `server/log-dir` - string, path to log directory (default: "/var/log/webhook-handler/")
* `server/max-script-threads` - number, max count of threads for handling webhooks
* `server/wait-seconds-between-run-scripts` - number, 
* `server/max-deque` - number, max value of possible webhooks handlers


* `webhook-handlers/%webhookid%/commands` - list of commands to execute (Variables: %WEBHOOK_DATA_FILE_PATH% - path to file with income data with webhooks)

File ./data/%dir%/webhook.conf:

```
id = %webhookid%
script_path = ./rebuild.sh
script_wait_in_sec = 600
enabled = yes
```

* id - unique id of webhook (replace on randoms string [A-Za-z0-9])
* script_path - path to script, relative
* script_wait_in_sec - how much time wait


After start server will be here

unique id of webhook, 
Webhook will like http://localhost:%port%/%webhook-url-path%







