#!/bin/bash

ROOT_PATH=${PWD}
LOG_FILE=${ROOT_PATH}/box/dtn_simulation_result/dtnrunninglog.txt
DATE=`date +%Y-%m-%d`
DIR1="./ns3dtn-bit-back-up"
CXX="g++"
CXXFLAGS="-Wno-error"

if [ -d $DIR1 ]; then
    echo 'back up!'
    rm -rf ${DIR1}
    cp -rf ./ns3dtn-bit ${DIR1}
else
    echo 'no back_up, copy one'
    cp -rf ./ns3dtn-bit ${DIR1}
    return
fi
echo '***************** cp working code to root/xx_tmp *********' 
# configure and run test
cd ./ns-allinone-3.26/ns-3.26/src
if [[ -L "$LINK_NAME" && -d "$LINK_NAME" ]]
then 
    echo "${LINK_NAME} exist and is a symbol link"
else 
    echo "${LINK_NAME} do not exist or is not symbol link, create one"
    ln -s ${LINK_NAME} ../../../ns3dtn-bit
fi
echo '***************** gonna to configure'
cd ../
CXX="${CXX}" ./waf configure -d debug --disable-python --enable-examples --enable-tests
#./waf configure -d debug --enable-examples --enable-tests
echo '***************** gonna to build' 
#./waf build --boost-lib=/usr/lib/x86_64-linux-gnu
#./waf --run bonnmotion-ns2-example --visualize
#./waf --run examples/wireless/wifi-adhoc --visualize 
#./waf --run ns3dtn-bit-your-example --verbose 2>&1 >${LOG_FILE}
./waf build
#exit 1
echo '============================== Dividing =========================='
echo "This script would write running log into ${LOG_FILE}---${DATE}"
echo '***************** gonna to test' 
# this command can show you all the 'test-project name'
#./test.py --list
#./test.py --example=ns3dtn-bit-example --text=results.txt --verbose 2>&1 >${LOG_FILE}
./test.py --example=ns3dtn-bit-your-example --text=results.txt --verbose 2>&1 >${LOG_FILE}

while true; do
    echo 'do you want gdb ?'
    read -p "yes or no, most of time you should say no :" yn
    case $yn in
        [Yy]* ) break;;
        [Nn]* ) exit 0;break;;
        * ) echo "Please answer y or n.";;
    esac
done

./waf --command-template="gdb %s" --run ns3dtn-bit-your-example
# debug with gdb https://www.nsnam.org/wiki/HOWTO_use_gdb_to_debug_program_errors
#./waf --command-template="gdb %s" --run third
#(gdb) run --help > output.txt 2>&1
