#!/bin/bash

ROOT_PATH=${PWD}
LOG_FILE=${ROOT_PATH}/box/dtn_simulation_result/dtnrunninglog.txt

function one_loop_func {
    echo -e "\033[96m********\n****************\n*********************** one loop *************\n****************\n******\n"
    # if we don't add this, run excutable would get "cannot open shared object file: No such file or directory"
    LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./ns-allinone-3.26/ns-3.26/build/src/ns3dtn-bit/examples/:
    LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./ns-allinone-3.26/ns-3.26/build/:
    export LD_LIBRARY_PATH
    ./ns-allinone-3.26/ns-3.26/build/src/ns3dtn-bit/examples/ns3.26-ns3dtn-bit-your-example-debug $1 $2 &>${LOG_FILE}
    ./box/ParseLogInLoops.py $3
}

#TEG CGR Spray Heuristic
function cycle_func {
    for x_running_argument_0 in {101..114..1}
    do 
        x_running_argument_1="TEG"
        x_parse_argument=loop_cycle-$x_running_argument_0\swith\sx_running_argument_1
        one_loop_func $x_running_argument_0 $x_running_argument_1 $x_parse_argument
        x_running_argument_1="CGR"
        x_parse_argument=loop_cycle-$x_running_argument_0\swith\sx_running_argument_1
        one_loop_func $x_running_argument_0 $x_running_argument_1 $x_parse_argument
        x_running_argument_1="Spray"
        x_parse_argument=loop_cycle-$x_running_argument_0\swith\sx_running_argument_1
        one_loop_func $x_running_argument_0 $x_running_argument_1 $x_parse_argument
        x_running_argument_1="Heuristic"
        x_parse_argument=loop_cycle-$x_running_argument_0\swith\sx_running_argument_1
        one_loop_func $x_running_argument_0 $x_running_argument_1 $x_parse_argument
    done
}

function tx1_func {
    echo "in tx1_func"
    for x_running_argument_0 in {201..201..1}
    do
        echo "one"
        x_running_argument_1="TEG"
        x_parse_argument=tx1-$x_running_argument_0\swith\sx_running_argument_1
        one_loop_func $x_running_argument_0 $x_running_argument_1 $x_parse_argument
    #    x_running_argument_1="CGR"
    #    x_parse_argument=tx1-$x_running_argument_0\swith\sx_running_argument_1
    #    one_loop_func $x_running_argument_0 $x_running_argument_1 $x_parse_argument
    #    x_running_argument_1="Spray"
    #    x_parse_argument=tx1-$x_running_argument_0\swith\sx_running_argument_1
    #    one_loop_func $x_running_argument_0 $x_running_argument_1 $x_parse_argument
    #    x_running_argument_1="Heuristic"
    #    x_parse_argument=tx1-$x_running_argument_0\swith\sx_running_argument_1
    #    one_loop_func $x_running_argument_0 $x_running_argument_1 $x_parse_argument
    done
}

tx1_func
