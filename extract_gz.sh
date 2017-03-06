#!/bin/bash


NS3TAR="/home/dtn-012345/Downloads/ns-allinone-3.26.tar.bz2"
# create environment
tar -xjvf $NS3TAR &> /dev/null
cp -rf ./ns3dtn-bit ./ns-allinone-3.26/ns-3.26/src/
