#!/bin/sh
ps aux | awk '$3 > 30 {print $2}' | xargs kill -9
# �屼�Y�귽 > 30 �� process
