#!/bin/bash


LOG_FILE="./tmp.txt"
DATE=`date +%Y-%m-%d`
cp -rf ./ns-allinone-3.25/ns-3.25/src/ns3dtn_bit ./ns3dtn_bit_tmp
echo 'cp working code to root/xx_tmp' > ${LOG_FILE} 2>&1
# configure and run test
cd ./ns-allinone-3.25/ns-3.25/waf
echo ' gonna to configure' > ${LOG_FILE} 2>&1
./waf configure --enable-examples --enable-tests > ${LOG_FILE} 2>&1
echo ' gonna to build' > ${LOG_FILE} 2>&1
./waf build > ${LOG_FILE} 2>&1
echo ' gonna to test' > ${LOG_FILE} 2>&1
./test.py > ${LOG_FILE} 2>&1
# delete ns-3 source prepare for git
rm -rf ./ns-allinone-3.25
rm -rf ./ns3dtn_bit
mv ./ns3dtn_bit_tmp ./ns3dtn_bit
# git push
git add --all
git commit -m "${DATE} local commit"
# create environment
tar -xjvf ~/Downloads/ns-allinone-3.25.tar.bz2
cp -rf ./ns3dtn_bit ./ns-allinone-3.25/ns-3.25/src/
