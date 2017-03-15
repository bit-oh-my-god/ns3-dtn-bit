#!/bin/bash

ROOT_PATH=${PWD}
LOG_FILE=${ROOT_PATH}/box/dtn_simulation_result/dtnrunninglog.txt
DATE=`date +%Y-%m-%d`
DIR1="./ns3dtn-bit_tmp"
if [ -d $DIR1 ]; then
    rm -rf ./ns3dtn-bit_tmp
else
    echo 'no tmp directory'
fi
cp -rf ./ns-allinone-3.26/ns-3.26/src/ns3dtn-bit ./ns3dtn-bit_tmp
cp -rf ./ns-allinone-3.25/ns-3.25/src/ns3dtn-bit ./ns3dtn-bit_tmp
echo '***************** cp working code to root/xx_tmp *********' 
# configure and run test
cd ./ns-allinone-3.26/ns-3.26
echo '***************** gonna to configure'
#CXXFLAGS=${CXXFLAGS} 
./waf configure -d debug --enable-examples --enable-tests
echo '***************** gonna to build' 
./waf build 
#exit 1
echo '============================== Dividing =========================='
echo "This script would write running log into ${LOG_FILE}---${DATE}"
echo '***************** gonna to test' 
# this command can show you all the 'test-project name'
#./test.py --list
#./test.py --example=ns3dtn-bit-example --text=results.txt --verbose 2>&1 >${LOG_FILE}
./test.py --example=ns3dtn-bit-your-example --text=results.txt --verbose

while true; do
    echo 'do you want gdb ?'
    read -p "yes or no, most of time you should say no :" yn
    case $yn in
        [Yy]* ) break;;
        [Nn]* ) exit 0;break;;
        * ) echo "Please answer y or n.";;
    esac
done

./waf --command-template="gdb %s" --run ns3dtn-bit-example
# debug with gdb https://www.nsnam.org/wiki/HOWTO_use_gdb_to_debug_program_errors
#./waf --command-template="gdb %s" --run third
#(gdb) run --help > output.txt 2>&1
