import os
import sys

file_name = 'current_trace.tcl'
speed=20;
move_start=(1000,1000);
move_end=(6100,1000);
step=100;
length=2000
time_span=100;

def move_fun(src,length):
    node1=src;
    node2=(src[0]+length,src[1],0);
    node3=(src[0]+length,src[1]+length,0)
    node4 = (src[0] , src[1] + length,0)
    node5=(src[0]+length/2 , src[1] + length/2,0);
    return node1,node2,node3,node4,node5
axis=["X_","Y_","Z_"]
src_node=();
def wright_fun(file,node,node_index,time,speed,director):
    if(director=='r'):
        for i in range(3):
            if(i==0):
                str_s="$node_("+str(node_index)+") set "+axis[i]+" "+ str(node[i])+"\n";
            else:
                str_s="$node_("+str(node_index)+") set "+axis[i]+" "+ str(node[i])+"\n";
            file.write(str_s)
        for j in range(8):
            str_s="$ns_ at "+str(time*j)+" ";
            str_t="$node_("+str(node_index)+") setdest "+str(node[0]+time*j)+" "+str(node[1])+" "+str(node[2])+" "+str(speed)
            str_s=str_s+'"%s"' % str_t+"\n";
            file.write(str_s)
    if(director=='d'):
        for i in range(3):
            if(i==1):
                str_s="$node_("+str(node_index)+") set "+axis[i]+" "+ str(node[i]-100)+"\n";
            else:
                str_s="$node_("+str(node_index)+") set "+axis[i]+" "+ str(node[i])+"\n";
            file.write(str_s)
        for j in range(8):
            str_s="$ns_ at "+str(time*j)+" ";
            str_t="$node_("+str(node_index)+") setdest "+str(node[0])+" "+str(node[1]+time*j)+" "+str(node[2])+" "+str(speed)
            str_s=str_s+'"%s"' % str_t+"\n";
            file.write(str_s)
    if(director=='l'):
        for i in range(3):
            if(i==0):
                str_s="$node_("+str(node_index)+") set "+axis[i]+" "+ str(node[i]+100)+"\n";
            else:
                str_s="$node_("+str(node_index)+") set "+axis[i]+" "+ str(node[i])+"\n";
            file.write(str_s)
        for j in range(8):
            str_s="$ns_ at "+str(time*j)+" ";
            str_t="$node_("+str(node_index)+") setdest "+str(node[0]-time*j)+" "+str(node[1])+" "+str(node[2])+" "+str(speed)
            str_s=str_s+'"%s"' % str_t+"\n";
            file.write(str_s)
    if(director=='u'):
        for i in range(3):
            if(i==1):
                str_s="$node_("+str(node_index)+") set "+axis[i]+" "+ str(node[i]+100)+"\n";
            else:
                str_s="$node_("+str(node_index)+") set "+axis[i]+" "+ str(node[i])+"\n";
            file.write(str_s)
        for j in range(8):
            str_s="$ns_ at "+str(time*j)+" ";
            str_t="$node_("+str(node_index)+") setdest "+str(node[0])+" "+str(node[1]-time*j)+" "+str(node[2])+" "+str(speed)
            str_s=str_s+'"%s"' % str_t+"\n";
            file.write(str_s)

def wright_fun_s(file,node,node_index,time,speed):
    for i in range(3):
        str_s="$node_("+str(node_index)+") set "+axis[i]+" "+ str(node[i])+"\n";
        file.write(str_s)
    for j in range(8):
        str_s="$ns_ at "+str(time*j)+" ";
        str_t="$node_("+str(node_index)+") setdest "+str(node[0])+" "+str(node[1])+" "+str(node[2])+" "+str(speed)
        str_s=str_s+'"%s"' % str_t+"\n";
        file.write(str_s)

fileobj=open(file_name,"w")
def produce(move_start,move_end):
    #-------
    for node in range(move_start[0],move_end[0],step):
        tmp = (node, move_start[1], 0);
        result=move_fun(tmp,length);
        src_node=result[0]
        for i in range(5):
            wright_fun(fileobj, result[i], i, time_span, speed,'r')
    print(src_node)

    #---------down
    for node in range(src_node[1],src_node[0]+step,step):
        tmp = (src_node[0], node, 0);
        result=move_fun(tmp,length);
        src_node=result[0]
        for i in range(5):
            wright_fun(fileobj, result[i], i, time_span, speed,'d')
    print(src_node)
    #-----------left
    for node in range(6000,1000-step,-step):
        # print(src_node[0])
        tmp = (node,6000, 0);
        result=move_fun(tmp,length);
        src_node=result[0]
        for i in range(5):
            wright_fun(fileobj, result[i], i, time_span, speed,'l')
    print(src_node)
    #---------------------up
    for node in range(6000,1000-step,-step):
        tmp = (1000,node, 0);
        result=move_fun(tmp,length);
        src_node=result[0]
        for i in range(5):
            wright_fun(fileobj, result[i], i, time_span, speed,'u')
    print(src_node)
produce(move_start,move_end);

def produce():
    #-------
    for node in range(14000,9000-step,-step):
        tmp = (node, 1000, 0);
        result=move_fun(tmp,length);
        src_node=result[0]
        for i in range(5):
            wright_fun(fileobj, result[i], i+5, time_span, speed,'l')
    print(src_node)

    #---------down
    for node in range(1000,6000+step,step):
        tmp = (8000, node, 0);
        result=move_fun(tmp,length);
        src_node=result[0]
        for i in range(5):
            wright_fun(fileobj, result[i], i+5, time_span, speed,'d')
    print(src_node)
    #-----------right
    for node in range(9000,14000+step,step):
        # print(src_node[0])
        tmp = (node,6000, 0);
        result=move_fun(tmp,length);
        src_node=result[0]
        for i in range(5):
            wright_fun(fileobj, result[i], i+5, time_span, speed,'r')
    print(src_node)
    #---------------------up
    for node in range(6000,1000-step,-step):
        tmp = (13000,node, 0);
        result=move_fun(tmp,length);
        src_node=result[0]
        for i in range(5):
            wright_fun(fileobj, result[i], i+5, time_span, speed,'u')
    print(src_node)
produce()

for node in range(14000, 9000 - step, -step):
    # print(src_node[0])
    tmp = (1000, 9000, 0);
    result = move_fun(tmp, length);
    print(result)
    # src_node = result[0]
    for i in range(5):
        wright_fun_s(fileobj, result[i], i + 10, time_span, 0)
for node in range(14000, 9000 - step, -step):
        # print(src_node[0])
    tmp = (9000, 9000, 0);
    result = move_fun(tmp, length);
    # src_node = result[0]
    for i in range(5):
        wright_fun_s(fileobj, result[i], i + 15, time_span, 0)
# move_start=(8000,0);
# move_end=(13100,0);
# produce(move_start,move_end)
fileobj.close();




