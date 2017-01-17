#!/bin/bash

LOG_FILE="/home/dtn-012345/ns-3_build/ns3-dtn-bit/tmp.txt"
DATE=`date +%Y-%m-%d`
CXXFLAGS='-std=c++11'
cp -rf ./ns-allinone-3.25/ns-3.25/src/ns3dtn-bit ./ns3dtn-bit_tmp
echo "${LOG_FILE}---${DATE}"
echo '***************** cp working code to root/xx_tmp *********' 
# configure and run test
cd ./ns-allinone-3.25/ns-3.25
echo '***************** gonna to configure'
./waf configure --enable-examples --enable-tests
echo '***************** gonna to build' 
./waf build 
echo '***************** gonna to test' 
./test.py 
