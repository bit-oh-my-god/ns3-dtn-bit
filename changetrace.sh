#!/bin/bash

echo "move file 0$1_current_trace to curretn_trace.tcl"
cp "./box/current_trace/0$1_current_trace.tcl" "./box/current_trace/current_trace.tcl"
echo "move file 0$1_teg.txt to teg.txt"
cp "./box/current_trace/0$1_teg.txt" "./box/current_trace/teg.txt"

