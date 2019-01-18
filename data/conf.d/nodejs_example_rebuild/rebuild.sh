#!/bin/bash

cd /some/path
git pull
npm run build
# TODO if need check build
cp -f dist/* /var/www/html/some_proj
# wget http://webhook.com/somehook