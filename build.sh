#!/bin/bash


LOG_FILE="/home/dtn-012345/ns-3_build/ns3-dtn-bit/tmp.txt"
DATE=`date +%Y-%m-%d`
cp -rf ./ns-allinone-3.25/ns-3.25/src/ns3dtn-bit ./ns3dtn-bit_tmp
echo "${LOG_FILE}---${DATE}"
echo 'cp working code to root/xx_tmp' 
#> ${LOG_FILE} 
# configure and run test
cd ./ns-allinone-3.25/ns-3.25
echo '***************** gonna to configure' 
#>> ${LOG_FILE} 2>&1
./waf configure --enable-examples --enable-tests 
#>> ${LOG_FILE} 2>&1
echo '***************** gonna to build' 
#>> ${LOG_FILE} 2>&1
./waf build 
#> ${LOG_FILE} 2>&1
echo '***************** gonna to test' 
#>> ${LOG_FILE} 2>&1
./test.py 
#>> ${LOG_FILE} 2>&1
