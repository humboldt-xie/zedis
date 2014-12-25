#!/bin/bash
#===============================================================================
#
#          FILE:  restart.sh
# 
#         USAGE:  ./restart.sh 
# 
#   DESCRIPTION:  
# 
#       OPTIONS:  ---
#  REQUIREMENTS:  ---
#          BUGS:  ---
#         NOTES:  ---
#        AUTHOR:  humboldt (), humboldt-xie@163.com
#       COMPANY:  humboldt
#       VERSION:  1.0
#       CREATED:  11/27/2014 12:01:57 AM CST
#      REVISION:  ---
#===============================================================================

make
killall zedis
./zedis -f ./test2 &

./zedis_cli tcp://127.0.0.1:4279 set time `date +%s` 
./zedis_cli tcp://127.0.0.1:4279 set time1 `date +%s` 
./zedis_cli tcp://127.0.0.1:4279 get time
./zedis_cli tcp://127.0.0.1:4279 get time1
./zedis_cli tcp://127.0.0.1:4279 sync "" 2
./zedis_cli tcp://127.0.0.1:4279 sync "time1" 2
./zedis_cli tcp://127.0.0.1:4279 sync "time1" 20
./zedis_cli tcp://127.0.0.1:4279 del time1
./zedis_cli tcp://127.0.0.1:4279 get time1

