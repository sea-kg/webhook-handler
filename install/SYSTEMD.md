# SYSTEMD

## copy (or replace) mecd.service

```
$ sudo cp ./SYSTEMD/mecd.service  /etc/systemd/system/mecd.service
$ sudo cp ../mecd /usr/bin/
```

## Create directories

```
$ sudo mkdir /opt/mecd
$ sudo cp -rf ../data/* /opt/mecd
```

## Reload and restart

```
$ sudo systemctl daemon-reload
$ sudo systemctl enable mecd
$ sudo systemctl restart mecd.service
```

## Uninstall

```
$ sudo systemctl stop mecd.service
$ sudo systemctl disable mecd.service
$ sudo rm /etc/systemd/system/mecd.service
$ sudo systemctl daemon-reload
$ sudo systemctl reset-failed
```
