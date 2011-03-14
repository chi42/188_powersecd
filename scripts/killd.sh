#!/bin/bash

# kills the daemon

kill $(cat /var/run/powersecd.pid)
