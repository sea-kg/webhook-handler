#!/bin/bash

curl -X POST -H "Content-Type: application/json" -d @./github-webhook.json http://localhost:8002/wh/github_test