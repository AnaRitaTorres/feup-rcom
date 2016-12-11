#!/bin/bash

ifconfig eth0 up 172.16.41.1/24
route add -net 172.16.40.0/24 gw 172.16.40.253
route add default gw 172.16.41.254