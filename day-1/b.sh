#!/bin/sh

gcc -std=c99 -D_GNU_SOURCE -I include/ -lczmq -lzmq -o src/fmq src/joe_proto.c src/fmq.c 
