#!/bin/bash

find ./ns3dtn-bit/ -type f | xargs grep "TODO"
find ./ns3dtn-bit/ -type f | xargs grep "Important"
