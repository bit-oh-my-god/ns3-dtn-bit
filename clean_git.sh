#!/bin/bash

DATE=`date +%Y-%m-%d`
DIR1="./ns3dtn-bit_tmp"
DIR2="./ns-allinone-3.26"
# delete ns-3 source prepare for git
function deletefuncion {
    D=$1
    if [ -d $D ]; then
        rm -rf $D
    else
        echo "no directory named ${D}"
    fi
}

while true; do
    echo 'do you want to clean ns3 folder, it will not be commit by git since the .gitignore file ?'
    read -p "yes or no :" yn
    case $yn in
        [Yy]* ) deletefuncion "${DIR2}"; break;;
        [Nn]* ) break;;
        * ) echo "Please answer y or n.";;
    esac
done

if [ -d $DIR1 ]; then
    rm -rf ./ns3dtn-bit
    mv $DIR1 ./ns3dtn-bit
else
    echo 'no tmp directory'
fi
# git push
git add --all
git commit -m "${DATE} local commit"

echo 'normal exit'
