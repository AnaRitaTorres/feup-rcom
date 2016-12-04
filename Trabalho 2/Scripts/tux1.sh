#!/bin/bash

ifconfig eth0 up 172.16.40.1/24
route add -net 172.16.41.0/24 gw 172.16.40.254
route add default gw 172.16.40.254