# My Easy Continues Delivery (mecd)

## Build on debian/ubuntu

Install requriments
```
$ apt-get -y update && apt-get install -y \
    make cmake \
    g++ \
    pkg-config \
    libcurl4-openssl-dev
```

Build
```
$ ./clean.sh && ./build_simple.sh
```

## Run

```
./mecd --dir ./data start
```

## Configure examples

File ./data/conf.d/server.conf:

```
http_port = 8001
sleep_between_run_scripts_in_sec = 1
threads_for_scripts = 1
```
Where

* sleep_between_run_scripts_in_sec - How much time sleep between scripts (max time before run script after got webhook)
* threads_for_scripts - How much threads for processing scripts 
* http_port - http port


File ./data/conf.d/%dir%/webhook.conf:

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

# unique id of webhook, 
# Webhook will like http://localhost:%port%/wh/%webhookid%