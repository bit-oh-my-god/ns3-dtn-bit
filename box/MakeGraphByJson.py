#!/home/tara/anaconda3/bin/python

import re
import sys
import os
import inspect
from math import sqrt
import math 
import numpy as np
from scipy import integrate
from mpl_toolkits.axes_grid1 import host_subplot
import mpl_toolkits.axisartist as AA
from matplotlib import pyplot as plt
import matplotlib as mpl
from matplotlib import cm
import pandas as pd
from mpl_toolkits.mplot3d import Axes3D
import jsonpickle
from matplotlib.colors import cnames
from matplotlib import animation
import collections
#========
def get_path_suffix_of(suffix):
    def getupper(path):
        return os.path.dirname(os.path.realpath(path))
    cur = __file__
    while (cur != '/'):
        cur = getupper(cur)
        if cur.endswith(suffix):
            print('get path suffix of {0}, return {1}'.format(suffix, cur))
            return cur
    raise Exception('can\'t')
#=========================== 
#################################################
handy_modify = False # diable me if you don't know what I'm doing , if you diable you may do some work to write senario_name into 'json' file TODO
g_senario_name_list = ['s-1', 's-2', 's-3', 's-4', 's-5', 's-6', 's-7', 's-8', 's-9', 's-10', 's-11', 's-12', 's-13', 's-14']
g_handy_hooks = False
#################################
######### definition
#                                                                                    dstination
#x_time_trace_map = {} # seqno -> [src_t, sr_id, hop_t, rec_t, rec_id, hop_t, ... , rec_t, rec_id, x_simulation_time]
#x_tosend_list = [] # element is also a list [source, time, destination, seqno] this is used to substitude x_schedule_list

def handy_print_2(seqno, p_x_tosend_list) :
    for xs in p_x_tosend_list :
        if xs[3] == seqno :
            print('schedule is: from node-{0} to node-{1} at time-{2}'.format(xs[0], xs[2], xs[1]))
            break
def handy_print_0(st, et, nid) :
    print('from time-{0} => time-{1} in node-{2}'.format(st, et, nid))
def handy_print_1(key, value, p_x_tosend_list) :
    print('==================\npkt-seqno:{0}, trace is: '.format(key))
    n_v = int(len(value) / 3)
    is_arrived_traffic = False
    if (n_v * 3 == len(value)) :
        is_arrived_traffic = True
    if (is_arrived_traffic) :
        for cur in range(0, n_v, 1) :
            if cur == n_v - 1 :
                print('received by node-{1} at time-{0} destination :)'.format(value[cur * 3], value[cur * 3 + 1]))
            else :
                handy_print_0(value[cur * 3], value[cur * 3 + 2], value[cur * 3 + 1])
        print('arrived destination\n')
    else :
        for cur in range(0, n_v + 1, 1) :
            if cur == n_v :
                print('received by node-{1} at time-{0}, not destination :('.format(value[cur * 3], value[cur * 3 + 1]))
                if cur == 0 :
                    print('this bundle may not be transmit out from source')
            else :
                handy_print_0(value[cur * 3], value[cur * 3 + 2], value[cur * 3 + 1])
        print('not arrived destination\n')
    handy_print_2(key, p_x_tosend_list)
def nums(s):
    try:
        return int(s)
    except ValueError:
        return float(s)
def handy_draw_0(ax0, st, et, nid, seqnoindex, c) :
    dx = et - st
    ax0.bar3d(st, seqnoindex, nid, dx + 1.0, 0.2, 0.2, alpha=0.1, color=c, linewidth=0) # alpha = abs(dz[i]/max(dz))
class JSONOB(object):
    def __init__(self, name, tosend_list_ob, time_trace_map_ob, total_hop, storageinfomap):
        self.name = name
        self.tosend_list_ob = tosend_list_ob
        self.time_trace_map_ob = time_trace_map_ob
        self.total_hop = total_hop
        self.storageinfomap = storageinfomap
    def get_name(self) :
        return self.name
    def get_map(self) :
        return self.time_trace_map_ob
    def get_list(self) :
        return self.tosend_list_ob
    def get_total_hop(self) :
        return self.total_hop
    def get_storagemap(self) :
        return self.storageinfomap
def save_this_jsonob_as(filename, jsonob_to) :
    fullpathname = './stuff folder/' + filename
    serialized_json = jsonpickle.encode(jsonob_to)
    with open(fullpathname, "w") as text_file:
        print(serialized_json, file=text_file)
def read_file_to_jsonob(filename) :
    with open(filename, "r") as file :
        lines = file.read()
        tmp_json_ob = jsonpickle.decode(lines)
        return tmp_json_ob
def draw_it(p_x_tosend_list, p_x_time_trace_map, p_x_simulation_time) :
    fig = plt.figure()
    ax = fig.gca(projection='3d')
    colors = plt.cm.ScalarMappable(cmap=plt.cm.get_cmap('jet')).to_rgba(x=np.linspace(0, 1, len(p_x_tosend_list)))
    #colors = plt.cm.jet(np.linspace(0, 1, len(p_x_tosend_list)))
    seqno_number_list = []
    for xsele in p_x_tosend_list :
        seqno_number_list.append(xsele[3])  
    print('colors count={0}'.format(len(colors)))
    print('len of seqno_number_list={0}'.format(len(seqno_number_list)))
    print(range(0, len(seqno_number_list), 1))
    for c, zseqnoindex in zip(colors, range(0, len(seqno_number_list), 1)):
        this_seqno_no = seqno_number_list[zseqnoindex]
        trace_list_of_seqno = p_x_time_trace_map[this_seqno_no]
        rt_count = int(len(trace_list_of_seqno) / 3)
        is_arrived_traffic_0 = False
        if (rt_count * 3 == len(trace_list_of_seqno)) :
            is_arrived_traffic_0 = True
            
        if is_arrived_traffic_0 :
            for cur in range(0, rt_count, 1) :
                if cur == rt_count - 1 :
                    handy_draw_0(ax, trace_list_of_seqno[cur * 3], trace_list_of_seqno[cur * 3 + 2], \
                                 trace_list_of_seqno[cur * 3 + 1], zseqnoindex, c)
                    ax.text(p_x_simulation_time, zseqnoindex, 0\
                            , 'arrived! seqno={0}'.format(seqno_number_list[zseqnoindex]), 'x')
                else :
                    handy_draw_0(ax, trace_list_of_seqno[cur * 3], trace_list_of_seqno[cur * 3 + 2], \
                                 trace_list_of_seqno[cur * 3 + 1], zseqnoindex, c)
        else :
            for cur in range(0, rt_count + 1, 1) :
                if cur == rt_count :
                    handy_draw_0(ax, trace_list_of_seqno[cur * 3], trace_list_of_seqno[cur * 3] + 1, \
                                 trace_list_of_seqno[cur * 3 + 1], zseqnoindex, c)
                    ax.text(p_x_simulation_time, zseqnoindex, 0\
                            , 'missed! seqno={0}'.format(seqno_number_list[zseqnoindex]), 'x')
                else :
                    handy_draw_0(ax, trace_list_of_seqno[cur * 3], trace_list_of_seqno[cur * 3 + 2], \
                                 trace_list_of_seqno[cur * 3 + 1], zseqnoindex, c)

    ax.set_xlabel('time')
    ax.set_ylabel('seqno')
    ax.set_zlabel('node')
    plt.show()
def result_for_one_scenario(jsonob_one) :
    one_x_time_trace_map = jsonob_one.get_map()
    one_x_tosend_list = jsonob_one.get_list()
    #print(one_x_tosend_list)
    for key, value in one_x_time_trace_map.items() :
        handy_print_1(key, value, one_x_tosend_list)
def abstract_value_from(jsonob_p) :
    j_time_trace_map = jsonob_p.get_map()
    j_tosend_list = jsonob_p.get_list()
    j_total_hop = jsonob_p.get_total_hop()
    j_name = jsonob_p.get_name()
    j_storage = jsonob_p.get_storagemap()
    #r12 = re.compile('''([a-zA-Z]+[0-9]+(-)?[0-9]+)\swith\s([a-zA-Z]+)''', re.VERBOSE)
    r12 = re.compile('''([a-zA-Z]+[0-9]+)\swith\s([a-zA-Z]+)''', re.VERBOSE)
    r13 = re.compile('''arriveN\-(\d+\.*\d*)\-scheduleN\-(\d+\.*\d*)''', re.VERBOSE)
    name_tag = r12.search(j_name)
    senario_name = None
    routing_name = None
    arrived = None
    total = None
    if name_tag :
        senario_name = name_tag.group(1)
        routing_name = name_tag.group(2)
    else :
        print("j_name is :{0}".format(j_name))
        if (j_name[7] == " ") :
            print("fuck")
        print("error_01")
        sys.exit()
    schedule_tag = r13.search(j_name)
    if schedule_tag :
        arrived = int(nums(schedule_tag.group(1)))
        total = int(nums(schedule_tag.group(2)))
    else :
        print(schedule_tag)
        print("eroor_02")
        sys.exit()
    delivery_rate = round(float(arrived) /  float(total), 4)
    #
    total_delivery_time = 0
    for j_list in j_tosend_list :
        j_seqno = j_list[3]
        start_time = nums(j_list[1])
        #print(j_time_trace_map)
        j_trace = j_time_trace_map[str(j_seqno)]
        end_time = None
        if len(j_trace) % 3 == 0:
            end_time = nums(j_trace[len(j_trace) - 3])
        else :
            continue
        consume_time = end_time - start_time
        if consume_time < 0 :
            print("error_09:end_time={0}, start_time={1}, j_seqno={2}, routing_name={3}, senario_name={4}"
                  .format(end_time, start_time, j_seqno, routing_name, senario_name))
            sys.exit()
        total_delivery_time += consume_time
    average_delivery_delay = None
    overhead = round(float(arrived) / float(j_total_hop), 4)
    if float(arrived) != 0.0 : 
        # this formula would cause graph not looks good
        #average_delivery_delay = float(total_delivery_time) / float(arrived)
        # this is good
        average_delivery_delay = float(total_delivery_time + (total - arrived) * 500) / float(total)
        #print("fuck! average_delivery_delay = {0}".format(average_delivery_delay))
    else :
        average_delivery_delay = 500
    # time -> {node -> total diff between real-storage-usage and guess-storage-usage}
    storagemap = {}
    for time_str in j_storage :
        time = nums(time_str)
        nodelist = []
        if len(nodelist) == 0 :
            for nodeid_str in j_storage[time_str]["local"]:
                nodeid = nums(nodeid_str)
                nodelist.append(nodeid)
        if  time not in storagemap :
            storagemap[time] = {}
        if "storageinfo" not in j_storage[time_str] :
            for nodeid in nodelist:
                if nodeid not in storagemap[time] :
                    storagemap[time][nodeid] = 0
        else :
            for nodeid_str in j_storage[time_str]["storageinfo"] :
                nodeid = nums(nodeid_str)
                diff = 0
                for belivnode in j_storage[time_str]["storageinfo"][nodeid_str] :
                    guess_usage = j_storage[time_str]["storageinfo"][nodeid_str][belivnode][1]
                    guess_beliv = j_storage[time_str]["storageinfo"][nodeid_str][belivnode][0]
                    real_usage = j_storage[time_str]["local"][belivnode][1]
                    numerator = 100
                    #assert(guess_beliv < numerator)
                    # beliv 越高，值的效用越低
                    factor =(numerator - guess_beliv) / numerator
                    if  factor > 0:
                        diff += math.fabs(guess_usage - real_usage) * factor
                storagemap[time][nodeid] = diff
            for nodeid in nodelist:
                if nodeid not in storagemap[time] :
                    storagemap[time][nodeid] = 0
    storagemap = collections.OrderedDict(sorted(storagemap.items()))
    return [senario_name, routing_name, delivery_rate, average_delivery_delay, overhead, storagemap]
    # return [senario_name, routing_name, delivery_rate, average_delivery_delay]
def draw_jsonob_list_storagemap_by_filter(p_out_jsonob_list, filter = "default"):
    abstracted_v_list = []
    for jsonob in p_out_jsonob_list :
        abstracted_v = abstract_value_from(jsonob)
        abstracted_v_list.append(abstracted_v)
    list_of_senario_name_k = []
    for abv in abstracted_v_list :
        if abv[0] not in list_of_senario_name_k :
            list_of_senario_name_k.append(abv[0])
    def detail_draw_storage(storagemap) :
        print("in detail draw storagemap")
        #print(storagemap)
        # TODO
        xseq = []
        name2line ={}
        for time in storagemap :
            #print(storagemap[time])
            xseq.append(time)
            for nodeid in storagemap[time]:
                nodename = 'n-' + str(nodeid)
                if nodename not in name2line :
                    name2line[nodename] = []
                name2line[nodename].append(storagemap[time][nodeid])
        for name in name2line :
            if (len(xseq) != len(name2line[name])) :
                print(name)
        #print(name2line['n-4'])
        #newname2line = {'n-4': name2line['n-4']}
        #print(xseq)
        maker = DetailGraphMaker_01(name2line, xseq, "time", "diff between guess and real", "diff-pkts")
        maker.dograph()
    if filter not in list_of_senario_name_k and filter == "default":
        targetstorage = abstracted_v_list[0][5]
        detail_draw_storage(targetstorage)
    elif filter in list_of_senario_name_k :
        targetstorage = None
        for abv in abstracted_v_list :
            if filter == abv[0] :
                targetstorage = abv[5]
        detail_draw_storage(targetstorage)
    elif filter == "allinone":
        sys.exit("Error-not implement yet")
    else :
        sys.exit("Error-231")
def draw_jsonob_list(p_out_jsonob_list) :
    abstracted_v_list = []
    for jsonob in p_out_jsonob_list :
        abstracted_v = abstract_value_from(jsonob)
        abstracted_v_list.append(abstracted_v)
    #----
    map_of_one_line_of_delivery_rate = {}
    map_of_one_line_of_average_delivery_delay = {}
    map_of_one_line_of_overhead = {}
    list_of_line_route_name_k = []
    list_of_senario_name_k = []
    for abv in abstracted_v_list :
        if abv[1] not in list_of_line_route_name_k :
            list_of_line_route_name_k.append(abv[1])
        if abv[0] not in list_of_senario_name_k :
            list_of_senario_name_k.append(abv[0])
    print('abstracted_v_list')
    print(abstracted_v_list)
    # prepare value from json_abstracted for makeing graph
    for abvv in abstracted_v_list :
        map_key = abvv[1]
        sena_index = list_of_senario_name_k.index(abvv[0])
        if map_key in map_of_one_line_of_delivery_rate :
            drlist = map_of_one_line_of_delivery_rate[map_key]
            drlist[sena_index] = abvv[2]
            map_of_one_line_of_delivery_rate[map_key] = drlist
        else :
            tmpaddlist = [None] * len(list_of_senario_name_k)
            tmpaddlist[sena_index] = abvv[2]
            map_of_one_line_of_delivery_rate[map_key] = tmpaddlist
        if map_key in map_of_one_line_of_average_delivery_delay :
            drlist = map_of_one_line_of_average_delivery_delay[map_key]
            drlist[sena_index] = abvv[3]
            map_of_one_line_of_average_delivery_delay[map_key] = drlist
        else :
            tmpaddlist = [None] * len(list_of_senario_name_k)
            tmpaddlist[sena_index] = abvv[3]
            map_of_one_line_of_average_delivery_delay[map_key] = tmpaddlist
        if map_key in map_of_one_line_of_overhead :
            drlist = map_of_one_line_of_overhead[map_key]
            drlist[sena_index] = abvv[4]
            map_of_one_line_of_overhead[map_key] = drlist
        else :
            tmpaddlist = [None] * len(list_of_senario_name_k)
            tmpaddlist[sena_index] = abvv[4]
            map_of_one_line_of_overhead[map_key] = tmpaddlist
    print('\nBefore Hooks:\nmap_of_one_line_of_delivery_rate={0}'.format(map_of_one_line_of_delivery_rate)) 
    print('\nBefore Hooks:\nmap_of_one_line_of_average_delivery_delay={0}'.format(map_of_one_line_of_average_delivery_delay)) 
    print('\nBefore Hooks:\nmap_of_one_line_of_overhead={0}'.format(map_of_one_line_of_overhead)) 
    print('\nBefore Hooks:\nlist_of_line_route_name_k={0}'.format(list_of_line_route_name_k)) 
    print('\nBefore Hooks:\nlist_of_senario_name_k={0}'.format(list_of_senario_name_k)) 
    list_of_list = g_hooks_handy_draw(map_of_one_line_of_delivery_rate, map_of_one_line_of_average_delivery_delay, map_of_one_line_of_overhead, list_of_line_route_name_k)
    map_of_one_line_of_delivery_rate = list_of_list[0]
    map_of_one_line_of_average_delivery_delay = list_of_list[1]
    map_of_one_line_of_overhead = list_of_list[2]
    list_of_line_route_name_k = list_of_list[3]
    print('\nAfter Hooks:\nmap_of_one_line_of_delivery_rate={0}'.format(map_of_one_line_of_delivery_rate)) 
    print('\nAfter Hooks:\nmap_of_one_line_of_average_delivery_delay={0}'.format(map_of_one_line_of_average_delivery_delay)) 
    print('\nAfter Hooks:\nmap_of_one_line_of_overhead={0}'.format(map_of_one_line_of_overhead)) 
    print('\nAfter Hooks:\nlist_of_line_route_name_k={0}'.format(list_of_line_route_name_k)) 
    print('\nAfter Hooks:\nlist_of_senario_name_k={0}'.format(list_of_senario_name_k)) 
    xseq = getxseqfromsenarioname(p_out_jsonob_list[0],len(list_of_senario_name_k)) #TODO
    print('\nxseq={0}'.format(xseq))
    maker01 = DetailGraphMaker_01(map_of_one_line_of_delivery_rate, xseq, 'pkts', '', "delivery rate")
    maker02 = DetailGraphMaker_01(map_of_one_line_of_average_delivery_delay, xseq, 'pkts', '', "delivery delay")
    maker03 = DetailGraphMaker_01(map_of_one_line_of_overhead, xseq, 'pkts', '', "overhead")
    maker01.dograph()
    maker02.dograph()
    maker03.dograph()
    #draw_graph_of(map_of_one_line_of_delivery_rate, list_of_line_route_name_k, list_of_senario_name_k, "delivery rate")
    #draw_graph_of(map_of_one_line_of_average_delivery_delay, list_of_line_route_name_k, list_of_senario_name_k, "delivery delay")
    #draw_graph_of(map_of_one_line_of_overhead, list_of_line_route_name_k, list_of_senario_name_k, "overhead")
# define getxseqfromsenarioname
def getxseqfromsenarioname(jsonob,lenths) :
    print(jsonob.name)
    namesff = ''
    def seqof(step,max):
        return list(range(step,max*step,step ))
    xseq = []
    #tx202 with
    r1 = re.compile(r'([a-z]{1,9})([0-9]{1,9})\swith', re.VERBOSE)
    namesf = r1.search(jsonob.name)
    if namesf:
        namesff = namesf.group(1)
        if namesff == 'tx':
            xseq = seqof(140,lenths+1)
        else :
            xseq = seqof(140,lenths+1)
            #sys.exit('error--11103 unknown')
    else :
        sys.exit('error--11102 unknown {0}'.format(jsonob.name))
    return xseq
# defind DetailGraphMaker_01
class DetailGraphMaker_01(object): # make graph with multi-lines 2d
    def __init__(self, name2line, xseq, xlabel, title, ylabel):
        self.m_name2line = name2line
        self.m_xlabel = xlabel
        self.m_title = title
        self.m_xseq = xseq
        self.m_ylabel = ylabel
    def dograph(self):
        fig = plt.figure()
        #ax = fig.add_subplot(111, axes_class=AA.Axes, title='delivery_rate')
        ax = host_subplot(111, axes_class=AA.Axes)
        plt.title(self.m_title, y=1.01)
        ax.set_xlabel(self.m_xlabel, fontsize=18)
        ax.set_ylabel(self.m_ylabel, fontsize=16)
        for name in self.m_name2line :
            #index = list_of_line_route_name.index(name)
            line = ax.plot(self.m_xseq, self.m_name2line[name], linewidth = 2, label = name)
        ax.legend(loc='lower right')
        #ax2 = ax.twin()  # ax2 is responsible for "top" axis and "right" axis
        #ax2.set_xticks(self.m_xseq)
        #ax2.axis["right"].major_ticklabels.set_visible(False)
        #ax2.axis["top"].major_ticklabels.set_visible(True)
        plt.show()
# end define DetailGraphMaker_01
#  'y' of one line index by route name; list of route name; list of 'x' axis name; flag is which figure you want
# change this graph func TODO
def draw_graph_of(map_of_one_line, list_of_line_route_name, list_of_senario_name, flag) :
    assert(len(map_of_one_line) == len(list_of_line_route_name))
    assert(map_of_one_line != None)
    sena_num = -1 # init
    if g_handy_hooks :
        if (len(map_of_one_line[list_of_line_route_name[0]]) == len(list_of_senario_name)) :
            print("nothing")
        else :
            print("a={0}, b={1},".format(len(map_of_one_line[list_of_line_route_name[0]]), len(list_of_senario_name)))
        sena_num = len(map_of_one_line[list_of_line_route_name[0]])
        assert(sena_num > 1)
    else :
        assert(len(map_of_one_line[list_of_line_route_name[0]]) == len(list_of_senario_name))
        sena_num = len(list_of_senario_name)
    list_of_senario_name_kk = []
    x = range(0, sena_num, 1)
    if handy_modify :
        list_of_senario_name_kk = g_senario_name_list
        assert(len(map_of_one_line[list_of_line_route_name[0]]) == len(list_of_senario_name_kk))
    if flag == "delivery rate" :
        fig = plt.figure()
        #ax = fig.add_subplot(111, axes_class=AA.Axes, title='delivery_rate')
        ax = host_subplot(111, axes_class=AA.Axes)
        plt.title('scenario name', y=1.03)
        #ax.set_xlabel('scenario name', fontsize=18) 
        ax.set_ylabel('delivery rate', fontsize=16)
    elif (flag == "delivery delay") :
        fig = plt.figure()
        #ax = fig.add_subplot(111, axes_class=AA.Axes, title='delivery_delay')
        ax = host_subplot(111, axes_class=AA.Axes)
        plt.title('scenario name', y=1.03)
        #ax.set_xlabel('scenario name', fontsize=18)
        ax.set_ylabel('average delivery delay', fontsize=16)
    elif (flag == "overhead") :
        fig = plt.figure()
        #ax = fig.add_subplot(111, axes_class=AA.Axes, title='delivery_delay')
        ax = host_subplot(111, axes_class=AA.Axes)
        plt.title('scenario name', y=1.03)
        #ax.set_xlabel('scenario name', fontsize=18)
        ax.set_ylabel('overhead ', fontsize=16)
    else :
        print("Error 10")
        sys.exit()
    for name in list_of_line_route_name :
        #index = list_of_line_route_name.index(name)
        line = ax.plot(x, map_of_one_line[name], '--', linewidth = 2, label = name)
    ax.legend(loc='lower right')
    ax2 = ax.twin()  # ax2 is responsible for "top" axis and "right" axis
    ax2.set_xticks(x)
    if not handy_modify :
        ax2.set_xticklabels(list_of_senario_name)
    else :
        ax2.set_xticklabels(list_of_senario_name_kk) # handy_modify

    ax2.axis["right"].major_ticklabels.set_visible(False)
    ax2.axis["top"].major_ticklabels.set_visible(True)
    plt.show()
def draw_one_senario(strname, out_jsonob_list_k):
    def draw_all_routing_path_of_one_senario_by_name(strname, out_jsonob_list_k) :
        # @brief draw 3d-way routing result, with sequence of pktseqno, seq of nodeid, seq of resident-time-interval
        def draw_it_after_json(p_x_tosend_list, p_x_time_trace_map, p_x_simulation_time) :
            fig = plt.figure()
            ax = fig.gca(projection='3d')
            colors = plt.cm.ScalarMappable(cmap=plt.cm.get_cmap('jet')).to_rgba(x=np.linspace(0, 1, len(p_x_tosend_list)))
            #colors = plt.cm.jet(np.linspace(0, 1, len(p_x_tosend_list)))
            seqno_number_list = []
            for xsele in p_x_tosend_list :
                seqno_number_list.append(xsele[3])  
            print('colors count={0}'.format(len(colors)))
            print('len of seqno_number_list={0}'.format(len(seqno_number_list)))
            print(range(0, len(seqno_number_list), 1))
            for c, zseqnoindex in zip(colors, range(0, len(seqno_number_list), 1)):
                this_seqno_no = seqno_number_list[zseqnoindex]
                trace_list_of_seqno = p_x_time_trace_map[str(this_seqno_no)]
                rt_count = int(len(trace_list_of_seqno) / 3)
                is_arrived_traffic_0 = False
                if (rt_count * 3 == len(trace_list_of_seqno)) :
                    is_arrived_traffic_0 = True
                    
                if is_arrived_traffic_0 :
                    for cur in range(0, rt_count, 1) :
                        if cur == rt_count - 1 :
                            handy_draw_0(ax, trace_list_of_seqno[cur * 3], trace_list_of_seqno[cur * 3 + 2], \
                                        trace_list_of_seqno[cur * 3 + 1], zseqnoindex, c)
                            ax.text(p_x_simulation_time, zseqnoindex, 0\
                                    , 'arrived! seqno={0}'.format(seqno_number_list[zseqnoindex]), 'x')
                        else :
                            handy_draw_0(ax, trace_list_of_seqno[cur * 3], trace_list_of_seqno[cur * 3 + 2], \
                                        trace_list_of_seqno[cur * 3 + 1], zseqnoindex, c)
                else :
                    for cur in range(0, rt_count + 1, 1) :
                        if cur == rt_count :
                            handy_draw_0(ax, trace_list_of_seqno[cur * 3], trace_list_of_seqno[cur * 3] + 1, \
                                        trace_list_of_seqno[cur * 3 + 1], zseqnoindex, c)
                            ax.text(p_x_simulation_time, zseqnoindex, 0\
                                    , 'missed! seqno={0}'.format(seqno_number_list[zseqnoindex]), 'x')
                        else :
                            handy_draw_0(ax, trace_list_of_seqno[cur * 3], trace_list_of_seqno[cur * 3 + 2], \
                                        trace_list_of_seqno[cur * 3 + 1], zseqnoindex, c)

            ax.set_xlabel('time')
            ax.set_ylabel('seqno')
            ax.set_zlabel('node')
            plt.show()
        #===
        for js_one_scenario in out_jsonob_list_k :
            if js_one_scenario.name == strname :
                that_x_time_trace_map = js_one_scenario.get_map()
                that_x_tosend_list = js_one_scenario.get_list()
                r14 = re.compile('''timeT\-(\d+\.*\d*)''', re.VERBOSE)
                timetag = r14.search(strname)
                that_simulation_time = None
                if timetag :
                    that_simulation_time = nums(timetag.group(1))
                else :
                    print("error_0123")
                    sys.exit()
                draw_it_after_json(that_x_tosend_list, that_x_time_trace_map, that_simulation_time)
    def draw_routing_path_of_one_senario_by_name(strname, out_jsonob_list_k) :
        # @brief draw 2d-way routing result, with one pktseqno, seq of nodeid, seq of resident-time-interval
        def draw_it_after_json2(p_x_tosend_list, p_x_time_trace_map, p_x_simulation_time) :
            fig = plt.figure()
            targetsrc = p_x_tosend_list[12341 % len(p_x_tosend_list)][0]
            targetschetime = p_x_tosend_list[12341 % len(p_x_tosend_list)][1]
            targetdes = p_x_tosend_list[12341 % len(p_x_tosend_list)][2]
            targetpktseqno = p_x_tosend_list[12341 % len(p_x_tosend_list)][3]
            colors = plt.cm.ScalarMappable(cmap=plt.cm.get_cmap('jet')).to_rgba(x=np.linspace(0, 1, len(p_x_tosend_list)))
            #TODO
        #===
        for js_one_scenario in out_jsonob_list_k :
            if js_one_scenario.name == strname :
                that_x_time_trace_map = js_one_scenario.get_map()
                that_x_tosend_list = js_one_scenario.get_list()
                r14 = re.compile('''timeT\-(\d+\.*\d*)''', re.VERBOSE)
                timetag = r14.search(strname)
                that_simulation_time = None
                if timetag :
                    that_simulation_time = nums(timetag.group(1))
                else :
                    print("error_0123")
                    sys.exit()
                draw_it_after_json2(that_x_tosend_list, that_x_time_trace_map, that_simulation_time)
                break
    draw_all_routing_path_of_one_senario_by_name(strname, out_jsonob_list_k)
    draw_routing_path_of_one_senario_by_name(strname, out_jsonob_list_k)
######## end of definition
###############################
def g_hooks_handy_draw(map_of_one_line_of_delivery_rate, map_of_one_line_of_average_delivery_delay, map_of_one_line_of_overhead, list_of_line_route_name_k) :
    if (g_handy_hooks) :
        list_of_line_route_name_k.append("CGR-QM")
        map_of_one_line_of_delivery_rate["CGR-QM"] = [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.972]
        map_of_one_line_of_average_delivery_delay["CGR-QM"] = [388, 384, 385, 391, 392, 388, 389, 390, 392, 391, 390, 392, 391, 400]
        map_of_one_line_of_overhead["CGR-QM"] = [0.333, 0.333, 0.333, 0.333, 0.333, 0.333, 0.333, 0.333, 0.333, 0.333, 0.333, 0.333, 0.333, 0.325]
        list_of_line_route_name_k.append("CGR")
        map_of_one_line_of_delivery_rate["CGR"] = [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.99, 0.95, 0.89, 0.86, 0.82, 0.80]
        map_of_one_line_of_average_delivery_delay["CGR"] = [389, 383, 386, 387, 393, 389, 388, 399, 410, 420, 431, 436, 442, 444]
        map_of_one_line_of_overhead["CGR"] = [0.333, 0.333, 0.333, 0.333, 0.333, 0.333, 0.333, 0.333, 0.330, 0.323, 0.319, 0.316, 0.314, 0.312]
    else :
        print("don't hooks")
    return [
            map_of_one_line_of_delivery_rate,
            map_of_one_line_of_average_delivery_delay,
            map_of_one_line_of_overhead,
            list_of_line_route_name_k,
           ]
######################

def filter_strlist_by_name(list_v, name) :
    newlist = []
    for strname in list_v :
        r17 = re.compile(name, re.VERBOSE)
        isname = r17.search(strname)
        if isname :
            newlist.append(strname)
    return newlist
# this is the main to run
def one_work_main(file_folder_path) :
    #=========
    #== settings
    # ./stuff folder/
                #'tx204 with TEG-nodeN-7-timeT-1000.0-arriveN-116-scheduleN-116',
    x_00_file_name_list = [
            #'switch401 with QM-nodeN-8-timeT-1000.0-arriveN-6-scheduleN-6',
            #'tx201 with QM-nodeN-7-timeT-1000.0-arriveN-40-scheduleN-40',
            'tx208 with QM-nodeN-7-timeT-1000.0-arriveN-320-scheduleN-320',
            ]
    #
    x_01_file_name_list = [
            'MaxRange-2000/tx201 with CGR-nodeN-7-timeT-1000.0-arriveN-48-scheduleN-70',
            ]
    x_02_file_name_list = [
            'MaxRange-2000/ran301 with CGR-nodeN-6-timeT-1000.0-arriveN-17-scheduleN-51',
            ]
    x_03_file_name_list = [
            'MaxRange-3000/tx201 with CGR-nodeN-7-timeT-1000.0-arriveN-70-scheduleN-70',
            ]
    x_04_file_name_list = [
            'MaxRange-3000/ran301 with CGR-nodeN-6-timeT-1000.0-arriveN-51-scheduleN-51',
            ]
    x_05_file_name_list =  [
            'MaxRange-4000/tx201 with CGR-nodeN-7-timeT-1000.0-arriveN-70-scheduleN-70',
            ]
    x_06_file_name_list = [
            'MaxRange-4000/ran301 with CGR-nodeN-6-timeT-1000.0-arriveN-51-scheduleN-51',
            ]
    
    ######
    # cycle
    #x_do_file_name_list = filter_strlist_by_name(x_05_file_name_list, "CGR") + filter_strlist_by_name(x_05_file_name_list, "TEG") + filter_strlist_by_name(x_05_file_name_list, "Heuristic") + filter_strlist_by_name(x_05_file_name_list, "DirectForward")

    # Random
    #x_do_file_name_list = filter_strlist_by_name(x_04_file_name_list, "Spray") + filter_strlist_by_name(x_04_file_name_list, "DirectForward")

    # CGRQM
    #x_do_file_name_list = []

    x_do_file_name_list = x_00_file_name_list
    ##====
    file_name_list = []
    #=====
    #==  change this TODO
    for filename in x_do_file_name_list :
        file_name_list.append(file_folder_path + filename)
    #== end of settings
    #=================
    print('====================== read serialized json =====================')
    out_jsonob_list = []
    for filename in file_name_list :
        out_jsonob_list.append(read_file_to_jsonob(filename))
    if len(out_jsonob_list) < 1 :
        print('error_04')
    for js_one_scenario in out_jsonob_list :
        print('''┌∩┐(◣_◢)┌∩┐ ┌∩┐(◣_◢)┌∩┐ ┌∩┐(◣_◢)┌∩┐   FOR   ONE  JSON  FILE  ┌∩┐(◣_◢)┌∩┐ ┌∩┐(◣_◢)┌∩┐ ┌∩┐(◣_◢)┌∩┐''')
        print('=========== name:{0} ======='.format(js_one_scenario.name))
        #result_for_one_scenario(js_one_scenario)
    print('====================== good ending =======================')
    print('====================== draw jsonob list =======================')
    draw_jsonob_list(out_jsonob_list)
    draw_jsonob_list_storagemap_by_filter(out_jsonob_list)
    #print('====================== draw one senario by name =======================')
    #draw_one_senario('cycle with CGR-nodeN-11-timeT-802.0-arriveN-14-scheduleN-14', out_jsonob_list)
###
######################
###################################
print(get_path_suffix_of('ns3-dtn-bit') + "/box/jupyter/stuff folder/")
one_work_main(get_path_suffix_of('ns3-dtn-bit') + "/box/jupyter/stuff folder/")
