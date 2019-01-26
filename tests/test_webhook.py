#!/usr/bin/env python
# -*- coding: utf-8 -*-

import time
import os
import sys
import subprocess
import signal
import math 
import socket
import random
import errno
import requests

HOST = 'localhost'
PORT = 8001
jsonData = {
    "some": "some",
    "some1": "some1",
    "some2": "some2",
    "some3": "some3",
    "some4": "some4",
    "some5": "some5",
    "some6": "some5",
    "some7": "some5",
    "some8": "some5",
    "some9": "some5",
    "some10": "some5",
    "some11": "some5",
    "some12": "some5",
}
try:
    r = requests.post('http://' + HOST + ':' + str(PORT) + '/wh/eSdR93Sq1w', json=jsonData)
    print("Status Code: " + str(r.status_code))
    print("Body:\n>>>\n" + r.text + "\n<<<")

except Exception as e:
    print(e)
