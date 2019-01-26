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

HOST = '127.0.0.1'
PORT = 8001

try:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # s.settimeout(1)
    s.connect((HOST, PORT))
    s.send("")
    result = s.recv(1024)
    print(result)
    s.close()
except Exception as e:
    print(e)
