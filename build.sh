#!/bin/bash

ROOT_PATH=${PWD}
LOG_FILE=${ROOT_PATH}/box/dtn_simulation_result/dtnrunninglog.txt
DATE=`date +%Y-%m-%d`
CXX="g++"
CXXFLAGS="-Wno-error"
cd ./ns-allinone-3.26/ns-3.26/src
LINK_NAME=${PWD}/ns3dtn-bit
cd ../../../

function config_func {
    echo -e "\033[96m ***************** config.txt *********"
    rm ./ns3dtn-bit/config.txt
    echo "static std::string root_path = \"${PWD}\";" > ./ns3dtn-bit/config.txt
}

function symbol_func {
    echo -e "\033[96m ***************** symbol link *********"
    # configure and run test
    cd ./ns-allinone-3.26/ns-3.26/src
    if [[ -L "$LINK_NAME" && -d "$LINK_NAME" ]]
    then 
        echo -e "\033[96m ${LINK_NAME} exist and is a symbol link"
    else 
        echo -e "\033[96m ${LINK_NAME} do not exist or is not symbol link, create one"
        ln -s ${LINK_NAME} ${ROOT_PATH}/ns3dtn-bit
    fi
}

function gdb_func {
    while true; do
        echo -e "\033[96m do you want gdb ? \033[0m"
        read -p "yes or no, most of time you should say no :" yn
        case $yn in
            [Yy]* ) break;;
            [Nn]* ) exit 0;break;;
            * ) echo "Please answer y or n.";;
        esac
    done

    # debug with gdb https://www.nsnam.org/wiki/HOWTO_use_gdb_to_debug_program_errors
    ./waf --command-template="gdb %s" --run ns3dtn-bit-your-example 
}

function build_func {

    echo -e "\033[96m ***************** gonna to configure ****************"
    cd ../
    CXX="${CXX}" ./waf configure -d debug --disable-python --enable-examples --enable-tests
    #./waf configure -d debug --enable-examples --enable-tests

    echo -e "\033[96m ***************** gonna to build **************"
    #./waf build --boost-lib=/usr/lib/x86_64-linux-gnu
    #./waf --run bonnmotion-ns2-example --visualize
    #./waf --run examples/wireless/wifi-adhoc --visualize 
    #./waf --run ns3dtn-bit-your-example --verbose 2>&1 >${LOG_FILE}
    ./waf build
    #exit 1
    echo -e "\033[96m ============================== Dividing =========================="
    echo -e "This script would write running log into ${LOG_FILE}---${DATE}"
    echo -e "***************** gonna to test ***************"
    # this command can show you all the 'test-project name'
    #./test.py --list
    #./test.py --example=ns3dtn-bit-example --text=results.txt --verbose 2>&1 >${LOG_FILE}
    #./test.py --example=ns3dtn-bit-your-example --verbose 2>&1 >${LOG_FILE}
    #./test.py --example=ns3dtn-bit-your-example --verbose
}

#=== =====================
# begin
config_func

symbol_func

build_func

#gdb_func
