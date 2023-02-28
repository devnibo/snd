#!/bin/bash

SECONDS=0
snd -i 202.61.254.236 -p 80 -d "GET / H"
echo $SECONDS
