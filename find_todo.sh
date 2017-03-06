#!/bin/bash

find ./ns3dtn-bit/ -type f | xargs grep "TODO" -in
find ./ns3dtn-bit/ -type f | xargs grep "Important" -in
