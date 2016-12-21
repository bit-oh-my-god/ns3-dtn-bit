#!/bin/bash

DATE=`date +%Y-%m-%d`
DIR1="./ns3dtn_bit_tmp"
DIR2="./ns-allinone-3.25"
# delete ns-3 source prepare for git
if [ -d $DIR2 ]; then
    rm -rf $DIR2
else
    echo 'no ns3 directory'
fi
if [ -d $DIR1 ]; then
    rm -rf ./ns3dtn_bit
    mv $DIR1 ./ns3dtn_bit
else
    echo 'no tmp directory'
fi
# git push
git add --all
git commit -m "${DATE} local commit"

echo 'exit'
