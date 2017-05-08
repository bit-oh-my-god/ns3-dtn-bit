import os
import sys

file_name = 'current_trace.tcl'

fileobj=open(file_name,"w")
speed=100;
move_start=[1000,1000,0];
length=5000
timer=10;
timer_span=50
circle_num=4
width=2000
axis=["X_","Y_","Z_"]
result=[];
bias=0

def move_fun(src,length):
    node1=src;
    node2=[src[0]+length,src[1],0];
    node5=[src[0]+length,src[1]+length,0]
    node4 = [src[0] , src[1] + length,0]
    node3=[src[0]+length/2 , src[1] + length/2,0]
    return [node1,node2,node3,node4,node5]
def write_fun(file,node,node_index,speed,num):
    for j in range(int(num)):
        global timer;
        str_s="$ns_ at "+str(timer)+" ";
        str_t="$node_("+str(node_index)+") setdest "+str(node[0])+" "+str(node[1])+" "+str(node[2])+" "+str(speed)
        str_s=str_s+'"%s"' % str_t+"\n";
        file.write(str_s)
        timer=timer+timer_span;

def wirte_node(file,node,node_index,):
    for i in range(3):
        if (i == 0):
            str_s = "$node_(" + str(node_index) + ") set " + axis[i] + " " + str(node[i]-bias) + "\n";
        else:
            str_s = "$node_(" + str(node_index) + ") set " + axis[i] + " " + str(node[i]) + "\n";
        file.write(str_s)
    write_fun(file,node,node_index,float(bias/timer_span),1)


def produce(file,move_start,length,node_index):
    wirte_node(file,move_start,node_index);
    num=1;
    for i in range(circle_num):
        move_start[0]=move_start[0]+length;
        write_fun(file,move_start,node_index,speed,num);
        move_start[1]=move_start[1]+length;
        write_fun(file, move_start, node_index,speed, num);
        move_start[0]=move_start[0]-length;
        write_fun(file, move_start, node_index,speed, num);
        move_start[1] = move_start[1] - length;
        write_fun(file, move_start, node_index,speed, num);


def produce_r(file,move_start,length,node_index):
    wirte_node(file,move_start,node_index);
    num=1;
    for i in range(circle_num):
        move_start[0]=move_start[0]-length;
        write_fun(file,move_start,node_index,speed,num);
        move_start[1]=move_start[1]+length;
        write_fun(file, move_start, node_index,speed, num);
        move_start[0]=move_start[0]+length;
        write_fun(file, move_start, node_index,speed, num);
        move_start[1] = move_start[1] - length;
        write_fun(file, move_start, node_index,speed, num);
result=move_fun(move_start,width)
for i in range(5):
    timer=0;
    produce(fileobj,result[i],length,i);

move_start=[15000,1000,0]
result=move_fun(move_start,width)

for i in range(0,5):
    timer=0;
    produce_r(fileobj,result[i],length,i+5);


def produce_s(file,move_start,node_index):

    num=1;
    for i in range(5):
        global timer;
        timer=0
        wirte_node(file, move_start[i], node_index+i);
        for j in range(circle_num*5):
            write_fun(file,move_start[i],i+node_index,0,num);

move_start=[1000,9000,0]
result=move_fun(move_start,width)
produce_s(fileobj,result,10);

move_start=[15000,9000,0]
result=move_fun(move_start,width)
produce_s(fileobj,result,15);

fileobj.close();




