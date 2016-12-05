#!/bin/sh

gcc -I include/ src/joe_proto.c fmq.c -lczmq -lzmq -o fmq
