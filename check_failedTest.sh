#!/bin/sh

if [ `cat testfailed.txt |wc -c` -ne 0 ]; then 
  exit 1
fi
