#!/bin/bash


NS3TAR="/home/dtn-012345/Downloads/ns-allinone-3.25.tar.bz2"
# create environment
tar -xjvf $NS3TAR &> /dev/null
cp -rf ./ns3dtn_bit ./ns-allinone-3.25/ns-3.25/src/
