#!/bin/bash


# delete ns-3 source prepare for git
rm -rf ./ns-allinone-3.25
rm -rf ./ns3dtn_bit
mv ./ns3dtn_bit_tmp ./ns3dtn_bit
# git push
git add --all
git commit -m "${DATE} local commit"
