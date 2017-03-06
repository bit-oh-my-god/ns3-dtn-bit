#!/bin/bash

function function1 {
./bm -hm RandomWaypoint
echo '
./bm -f default_scenario RandomWaypoint -n 20 -d 600 -R 2231 -x 9999 -y 9999 -z 9999 -J 5 -h 100 -l 10 -p 20 -o 5 > /dev/null
'
./bm -f default_scenario RandomWaypoint -n 20 -d 600 -R 2231 -x 9999 -y 9999 -z 9999 -J 5 -h 100 -l 10 -p 20 -o 5 > /dev/null
./bm NSFile -f default_scenario > /dev/null
mv *.ns_movements ~/ns-3_build/ns3-dtn-bit/box/current_trace/
cd ~/ns-3_build/ns3-dtn-bit/box/current_trace
count=1;
for mov in `find ./ -name 'default_scenario.ns_movements' -type f`
do
    new=current_trace.${mov##*.}
    echo "Renaming $mov to $new"
    mv "$mov" "$new"
    let count++
done
}

function function2 {
echo "revert the 'cmd' to user, remember move the .ns_movements file to box/current_trace/ and rename it as current_trace.ns_movements"
return
}


####################################
# script begin
###################################
cd ../ns3-dtn-bit/bonnmotion-3.0.1/bin

echo "*******************************************"
echo "bm help"
./bm -h

echo "*******************************************"
echo "bm help"
./bm -hm


echo "*******************************************"
echo "bm work!! some modle support 3d-movement, check more in 'readme.pdf'"
##./bm -f my_scenario while true; do

while true; do
    echo 'do you want default_scenario or your self scenario, yes is default?'
    read -p "yes or no :" yn
    case $yn in
        [Yy]* ) function1; break;;
        [Nn]* ) function2; break;;
        * ) echo "Please answer y or n.";;
    esac
done

cd ~/ns-3_build/ns3-dtn-bit

echo "*******************************************"
echo "end!"
