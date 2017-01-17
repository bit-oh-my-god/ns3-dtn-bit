#!/bin/bash

LOG_FILE="/home/dtn-012345/ns-3_build/ns3-dtn-bit/tmp.txt"
DATE=`date +%Y-%m-%d`
DIR1="./ns3dtn-bit_tmp"
CXXFLAGS='-std=c++11 -Wall'
if [ -d $DIR1 ]; then
    rm -rf ./ns3dtn-bit_tmp
    cp -rf ./ns-allinone-3.26/ns-3.26/src/ns3dtn-bit ./ns3dtn-bit_tmp
    cp -rf ./ns-allinone-3.25/ns-3.25/src/ns3dtn-bit ./ns3dtn-bit_tmp
else
    cp -rf ./ns-allinone-3.26/ns-3.26/src/ns3dtn-bit ./ns3dtn-bit_tmp
    cp -rf ./ns-allinone-3.25/ns-3.25/src/ns3dtn-bit ./ns3dtn-bit_tmp
    echo 'no tmp directory'
fi
echo "${LOG_FILE}---${DATE}"
echo '***************** cp working code to root/xx_tmp *********' 
# configure and run test
cd ./ns-allinone-3.26/ns-3.26
echo '***************** gonna to configure'
CXXFLAGS=${CXXFLAGS} ./waf configure --enable-examples --enable-tests
echo '***************** gonna to build' 
./waf build 
echo '***************** gonna to test' 
./test.py 
