#!/bin/bash

ROOT_PATH=${PWD}
LOG_FILE=${ROOT_PATH}/box/dtn_simulation_result/dtnrunninglog.txt

function one_loop_func {
    echo -e "\033[96m********\n****************\n*********************** one loop *************\n****************\n******\n"
    echo "argument are $1 $2 $3 $4 $5"
    newargu="$3 $4 $5" # 405 CGR ran405\swith\sCGR
    # if we don't add this, run excutable would get "cannot open shared object file: No such file or directory"
    LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./ns-allinone-3.26/ns-3.26/build/src/ns3dtn-bit/examples/:
    LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./ns-allinone-3.26/ns-3.26/build/:
    export LD_LIBRARY_PATH
    #export NS_LOG=UdpL4Protocol
    echo "./ns-allinone-3.26/ns-3.26/build/src/ns3dtn-bit/examples/ns3.26-ns3dtn-bit-your-example-debug $1 $2 &>${LOG_FILE}"
    ./ns-allinone-3.26/ns-3.26/build/src/ns3dtn-bit/examples/ns3.26-ns3dtn-bit-your-example-debug $1 $2 &>${LOG_FILE}
    #valgrind --track-fds=yes --leak-check=full --undef-value-errors=yes ./ns-allinone-3.26/ns-3.26/build/src/ns3dtn-bit/examples/ns3.26-ns3dtn-bit-your-example-debug $1 $2 &>${LOG_FILE}
    ./box/ParseLogInLoops.py $newargu
}

#TEG CGR Spray Heuristic
function cycle_func {
    echo "in cycle_func"
    prefix="loopcycle" # should be [a-z]{1,9}
    for x_running_argument_0 in {101..114..1}
    do 
        x_running_argument_1="TEG"
        # blabla1 with blabla2, blabla1 must be [a-z,A-Z]+[0-9]+
        x_parse_argument="$prefix$x_running_argument_0 with $x_running_argument_1"
        one_loop_func $x_running_argument_0 $x_running_argument_1 $x_parse_argument
        x_running_argument_1="CGR"
        x_parse_argument="$prefix$x_running_argument_0 with $x_running_argument_1"
        one_loop_func $x_running_argument_0 $x_running_argument_1 $x_parse_argument
        x_running_argument_1="Spray"
        x_parse_argument="$prefix$x_running_argument_0 with $x_running_argument_1"
        one_loop_func $x_running_argument_0 $x_running_argument_1 $x_parse_argument
        x_running_argument_1="Heuristic"
        x_parse_argument="$prefix$x_running_argument_0 with $x_running_argument_1"
        one_loop_func $x_running_argument_0 $x_running_argument_1 $x_parse_argument
    done
}

# this run would run TEG and CGR, where both should achive 100% delivery rate
function test_target_simulation_run_func {
    echo "in tx1_func"
    ./changetrace.sh 200
    prefix="tx" # should be [a-z]{1,9}
    for x_running_argument_0 in {201..201..1}
    do
        echo "one"
        x_running_argument_1="TEG"
        x_parse_argument="$prefix$x_running_argument_0 with $x_running_argument_1"
        one_loop_func $x_running_argument_0 $x_running_argument_1 $x_parse_argument
        x_running_argument_1="CGR"
        x_parse_argument="$prefix$x_running_argument_0 with $x_running_argument_1"
        one_loop_func $x_running_argument_0 $x_running_argument_1 $x_parse_argument
    done
}

# one source one destination scenario
function simulation_run_tx1_func {
    echo "in tx1_func"
    ./changetrace.sh 200
    prefix="tx" # should be [a-z]{1,9}
    for x_running_argument_0 in {201..201..1}
    do
        echo "one"
        x_running_argument_1="TEG"
        x_parse_argument="$prefix$x_running_argument_0 with $x_running_argument_1"
        one_loop_func $x_running_argument_0 $x_running_argument_1 $x_parse_argument
        #x_running_argument_1="CGR"
        #x_parse_argument="$prefix$x_running_argument_0 with $x_running_argument_1"
        #one_loop_func $x_running_argument_0 $x_running_argument_1 $x_parse_argument
        #x_running_argument_1="Spray"
        #x_parse_argument="$prefix$x_running_argument_0 with $x_running_argument_1"
        #one_loop_func $x_running_argument_0 $x_running_argument_1 $x_parse_argument
        #x_running_argument_1="Heuristic"
        #x_parse_argument="$prefix$x_running_argument_0 with $x_running_argument_1"
        #one_loop_func $x_running_argument_0 $x_running_argument_1 $x_parse_argument
        #x_running_argument_1="DirectForward"
        #x_parse_argument="$prefix$x_running_argument_0 with $x_running_argument_1"
        #one_loop_func $x_running_argument_0 $x_running_argument_1 $x_parse_argument
    done
}

# random synthetic scenario
function simulation_run_ran_func {
    echo "in ran_func"
    prefix="ran" # should be [a-z]{1,9}
    ./changetrace.sh 300
    for x_running_argument_0 in {305..314..1}
    do
        echo "one"
        #x_running_argument_1="TEG"
        #x_parse_argument="$prefix$x_running_argument_0 with $x_running_argument_1"
        #one_loop_func $x_running_argument_0 $x_running_argument_1 $x_parse_argument
        #x_running_argument_1="CGR"
        #x_parse_argument="$prefix$x_running_argument_0 with $x_running_argument_1"
        #one_loop_func $x_running_argument_0 $x_running_argument_1 $x_parse_argument
        x_running_argument_1="Spray"
        x_parse_argument="$prefix$x_running_argument_0 with $x_running_argument_1"
        one_loop_func $x_running_argument_0 $x_running_argument_1 $x_parse_argument
        x_running_argument_1="Heuristic"
        x_parse_argument="$prefix$x_running_argument_0 with $x_running_argument_1"
        one_loop_func $x_running_argument_0 $x_running_argument_1 $x_parse_argument
        x_running_argument_1="DirectForward"
        x_parse_argument="$prefix$x_running_argument_0 with $x_running_argument_1"
        one_loop_func $x_running_argument_0 $x_running_argument_1 $x_parse_argument
    done
}

function simulation_run_switch_func {
    echo "in switch_func"
    prefix="switch" # should be [a-z]{1,9}
    ./changetrace.sh 400
    for x_running_argument_0 in {405..405..1}
    do
        echo "one"
        #x_running_argument_1="TEG"
        #x_parse_argument="$prefix$x_running_argument_0 with $x_running_argument_1"
        #one_loop_func $x_running_argument_0 $x_running_argument_1 $x_parse_argument
        x_running_argument_1="CGR"
        x_parse_argument="$prefix$x_running_argument_0 with $x_running_argument_1"
        one_loop_func $x_running_argument_0 $x_running_argument_1 $x_parse_argument
        ##x_running_argument_1="CGR-QM"
        #x_parse_argument="$prefix$x_running_argument_0 with $x_running_argument_1"
        #one_loop_func $x_running_argument_0 $x_running_argument_1 $x_parse_argument
        #x_running_argument_1="Spray"
        #x_parse_argument="$prefix$x_running_argument_0 with $x_running_argument_1"
        #one_loop_func $x_running_argument_0 $x_running_argument_1 $x_parse_argument
        #x_running_argument_1="Heuristic"
        #x_parse_argument="$prefix$x_running_argument_0 with $x_running_argument_1"
        #one_loop_func $x_running_argument_0 $x_running_argument_1 $x_parse_argument
        #x_running_argument_1="DirectForward"
        #x_parse_argument="$prefix$x_running_argument_0 with $x_running_argument_1"
        #one_loop_func $x_running_argument_0 $x_running_argument_1 $x_parse_argument
    done
}

#####################################

#test_target_simulation_run_func
simulation_run_tx1_func
#simulation_run_ran_func
#simulation_run_switch_func
