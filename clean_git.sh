#!/bin/bash

DATE=`date +%Y-%m-%d`
DIR1="./ns3dtn-bit"
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
    read -p "yes or no, most of time you should say no :" yn
    case $yn in
        [Yy]* ) deletefuncion "${DIR2}"; break;;
        [Nn]* ) break;;
        * ) echo "Please answer y or n.";;
    esac
done

if [ -d $DIR1 ]; then
    echo "$DIR1 exist, do git"
else
    echo 'no tmp directory'
fi
# git push
git add --all && git commit -a -m "${DATE} local commit"

echo 'normal exit'
