# SYSTEMD

## copy (or replace) webhook-handler.service

```
$ sudo cp ./SYSTEMD/webhook-handler.service  /etc/systemd/system/webhook-handler.service
$ sudo cp ../webhook-handler /usr/bin/
```

## Create directories

```
$ sudo mkdir /opt/webhook-handler
$ sudo cp -rf ../data/* /opt/webhook-handler
```

## Reload and restart

```
$ sudo systemctl daemon-reload
$ sudo systemctl enable webhook-handler
$ sudo systemctl restart webhook-handler.service
```

## Uninstall

```
$ sudo systemctl stop webhook-handler.service
$ sudo systemctl disable webhook-handler.service
$ sudo rm /etc/systemd/system/webhook-handler.service
$ sudo systemctl daemon-reload
$ sudo systemctl reset-failed
```
