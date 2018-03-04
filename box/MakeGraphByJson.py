#!/home/dtn-012345/miniconda3/bin/python

import re
import sys
import os
import inspect
from math import sqrt
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

#################################################
handy_modify = True # diable me if you don't know what I'm doing , if you diable you may do some work to write senario_name into 'json' file TODO
g_senario_name_list = ['s-1', 's-2', 's-3', 's-4', 's-5', 's-6', 's-7', 's-8', 's-9', 's-10', 's-11', 's-12', 's-13', 's-14']
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
    def __init__(self, name, tosend_list_ob, time_trace_map_ob, total_hop):
        self.name = name
        self.tosend_list_ob = tosend_list_ob
        self.time_trace_map_ob = time_trace_map_ob
        self.total_hop = total_hop
    def get_name(self) :
        return self.name
    def get_map(self) :
        return self.time_trace_map_ob
    def get_list(self) :
        return self.tosend_list_ob
    def get_total_hop(self) :
        return self.total_hop
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
    colors = plt.cm.jet(np.linspace(0, 1, len(p_x_tosend_list)))
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
        print("fuck! average_delivery_delay = {0}".format(average_delivery_delay))
    else :
        average_delivery_delay = 500
    return [senario_name, routing_name, delivery_rate, average_delivery_delay, overhead]
    # return [senario_name, routing_name, delivery_rate, average_delivery_delay]
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
    print('\nmap_of_one_line_of_delivery_rate')
    print(map_of_one_line_of_delivery_rate)
    print('\nmap_of_one_line_of_average_delivery_delay')
    print(map_of_one_line_of_average_delivery_delay)
    print('\nlist_of_overhead')
    print(map_of_one_line_of_overhead)
    print('\nlist_of_line_route_name_k')
    print(list_of_line_route_name_k)
    print('\nlist_of_senario_name_k')
    print(list_of_senario_name_k)
    list_of_list = g_hooks_handy_draw(map_of_one_line_of_delivery_rate, map_of_one_line_of_average_delivery_delay, map_of_one_line_of_overhead, list_of_line_route_name_k)
    map_of_one_line_of_delivery_rate = list_of_list[0]
    map_of_one_line_of_average_delivery_delay = list_of_list[1]
    map_of_one_line_of_overhead = list_of_list[2]
    list_of_line_route_name_k = list_of_list[3]
    print('\nmap_of_one_line_of_delivery_rate')
    print(map_of_one_line_of_delivery_rate)
    print('\nmap_of_one_line_of_average_delivery_delay')
    print(map_of_one_line_of_average_delivery_delay)
    print('\nlist_of_overhead')
    print(map_of_one_line_of_overhead)
    print('\nlist_of_line_route_name_k')
    print(list_of_line_route_name_k)
    print('\nlist_of_senario_name_k')
    print(list_of_senario_name_k)
    draw_graph_of(map_of_one_line_of_delivery_rate, list_of_line_route_name_k, list_of_senario_name_k, "delivery rate")
    draw_graph_of(map_of_one_line_of_average_delivery_delay, list_of_line_route_name_k, list_of_senario_name_k, "delivery delay")
    draw_graph_of(map_of_one_line_of_overhead, list_of_line_route_name_k, list_of_senario_name_k, "overhead")

#               'y' of one line index by route name; list of route name; list of 'x' axis name; flag is which figure you want
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
def draw_it_after_json(p_x_tosend_list, p_x_time_trace_map, p_x_simulation_time) :
    fig = plt.figure()
    ax = fig.gca(projection='3d')
    colors = plt.cm.jet(np.linspace(0, 1, len(p_x_tosend_list)))
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
def draw_one_senario_by_name(strname, out_jsonob_list_k) :
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
def draw_statistic_of_one_senario_by_name(strname, out_jsonob_list_k) :
    # TODO
    print("nothing")
######## end of definition
###############################
#################################################

###################
g_handy_hooks = False
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
                    "cycle1 with CGR-nodeN-12-timeT-802.0-arriveN-4-scheduleN-4",
                    "cycle1 with Heuristic-nodeN-12-timeT-802.0-arriveN-1-scheduleN-4",
                    "cycle1 with Spray-nodeN-12-timeT-802.0-arriveN-4-scheduleN-4",
                    "cycle1 with TEG-nodeN-12-timeT-802.0-arriveN-4-scheduleN-4",
                    "cycle2 with CGR-nodeN-12-timeT-802.0-arriveN-16-scheduleN-16",
                    "cycle2 with Heuristic-nodeN-12-timeT-802.0-arriveN-5-scheduleN-16",
                    "cycle2 with Spray-nodeN-12-timeT-802.0-arriveN-15-scheduleN-16",
                    "cycle2 with TEG-nodeN-12-timeT-802.0-arriveN-16-scheduleN-16",
                    "cycle3 with CGR-nodeN-12-timeT-802.0-arriveN-36-scheduleN-36",
                    "cycle3 with Heuristic-nodeN-12-timeT-802.0-arriveN-24-scheduleN-36",
                    "cycle3 with Spray-nodeN-12-timeT-802.0-arriveN-31-scheduleN-36",
                    "cycle3 with TEG-nodeN-12-timeT-802.0-arriveN-36-scheduleN-36",
                    "cycle4 with CGR-nodeN-12-timeT-802.0-arriveN-59-scheduleN-59",
                    "cycle4 with Heuristic-nodeN-12-timeT-802.0-arriveN-36-scheduleN-59",
                    "cycle4 with Spray-nodeN-12-timeT-802.0-arriveN-44-scheduleN-59",
                    "cycle4 with TEG-nodeN-12-timeT-802.0-arriveN-59-scheduleN-59",
                    "cycle5 with CGR-nodeN-12-timeT-802.0-arriveN-92-scheduleN-92",
                    "cycle5 with Heuristic-nodeN-12-timeT-802.0-arriveN-66-scheduleN-92",
                    "cycle5 with Spray-nodeN-12-timeT-802.0-arriveN-74-scheduleN-92",
                    "cycle5 with TEG-nodeN-12-timeT-802.0-arriveN-92-scheduleN-92",
                    "cycle6 with CGR-nodeN-12-timeT-802.0-arriveN-146-scheduleN-146",
                    "cycle6 with Heuristic-nodeN-12-timeT-802.0-arriveN-88-scheduleN-146",
                    "cycle6 with Spray-nodeN-12-timeT-802.0-arriveN-113-scheduleN-146",
                    "cycle6 with TEG-nodeN-12-timeT-802.0-arriveN-146-scheduleN-146",
                    "cycle7 with CGR-nodeN-12-timeT-802.0-arriveN-217-scheduleN-217",
                    "cycle7 with Heuristic-nodeN-12-timeT-802.0-arriveN-106-scheduleN-217",
                    "cycle7 with Spray-nodeN-12-timeT-802.0-arriveN-152-scheduleN-217",
                    "cycle7 with TEG-nodeN-12-timeT-802.0-arriveN-217-scheduleN-217",
                    "cycle8 with CGR-nodeN-12-timeT-802.0-arriveN-336-scheduleN-336",
                    "cycle8 with Heuristic-nodeN-12-timeT-802.0-arriveN-169-scheduleN-336",
                    "cycle8 with Spray-nodeN-12-timeT-802.0-arriveN-240-scheduleN-336",
                    "cycle8 with TEG-nodeN-12-timeT-802.0-arriveN-336-scheduleN-336",
                    "cycle9 with CGR-nodeN-12-timeT-802.0-arriveN-504-scheduleN-504",
                    "cycle9 with Heuristic-nodeN-12-timeT-802.0-arriveN-267-scheduleN-504",
                    "cycle9 with Spray-nodeN-12-timeT-802.0-arriveN-322-scheduleN-504",
                    "cycle9 with TEG-nodeN-12-timeT-802.0-arriveN-504-scheduleN-504",
                    "cycle10 with CGR-nodeN-12-timeT-802.0-arriveN-656-scheduleN-672",
                    "cycle10 with Heuristic-nodeN-12-timeT-802.0-arriveN-400-scheduleN-672",
                    "cycle10 with Spray-nodeN-12-timeT-802.0-arriveN-408-scheduleN-672",
                    "cycle10 with TEG-nodeN-12-timeT-802.0-arriveN-640-scheduleN-672",
                    "cycle11 with CGR-nodeN-12-timeT-802.0-arriveN-937-scheduleN-1024",
                    "cycle11 with Heuristic-nodeN-12-timeT-802.0-arriveN-557-scheduleN-1024",
                    "cycle11 with Spray-nodeN-12-timeT-802.0-arriveN-559-scheduleN-1024",
                    "cycle11 with TEG-nodeN-12-timeT-802.0-arriveN-902-scheduleN-1024",
                    "cycle12 with CGR-nodeN-12-timeT-802.0-arriveN-1039-scheduleN-1360",
                    "cycle12 with Heuristic-nodeN-12-timeT-802.0-arriveN-580-scheduleN-1360",
                    "cycle12 with Spray-nodeN-12-timeT-802.0-arriveN-641-scheduleN-1360",
                    "cycle12 with TEG-nodeN-12-timeT-802.0-arriveN-1070-scheduleN-1360",
                    ]

    #
    x_01_file_name_list = [
            'MaxRange-2000/tx201 with CGR-nodeN-7-timeT-1000.0-arriveN-48-scheduleN-70',
            'MaxRange-2000/tx201 with DirectForward-nodeN-7-timeT-1000.0-arriveN-42-scheduleN-70',
            'MaxRange-2000/tx201 with Heuristic-nodeN-7-timeT-1000.0-arriveN-49-scheduleN-70',
            'MaxRange-2000/tx201 with Spray-nodeN-7-timeT-1000.0-arriveN-44-scheduleN-70',
            'MaxRange-2000/tx201 with TEG-nodeN-7-timeT-1000.0-arriveN-70-scheduleN-70',
            'MaxRange-2000/tx202 with CGR-nodeN-7-timeT-1000.0-arriveN-95-scheduleN-140',
            'MaxRange-2000/tx202 with DirectForward-nodeN-7-timeT-1000.0-arriveN-91-scheduleN-140',
            'MaxRange-2000/tx202 with Heuristic-nodeN-7-timeT-1000.0-arriveN-97-scheduleN-140',
            'MaxRange-2000/tx202 with Spray-nodeN-7-timeT-1000.0-arriveN-68-scheduleN-140',
            'MaxRange-2000/tx202 with TEG-nodeN-7-timeT-1000.0-arriveN-140-scheduleN-140',
            'MaxRange-2000/tx203 with CGR-nodeN-7-timeT-1000.0-arriveN-143-scheduleN-210',
            'MaxRange-2000/tx203 with DirectForward-nodeN-7-timeT-1000.0-arriveN-107-scheduleN-210',
            'MaxRange-2000/tx203 with Heuristic-nodeN-7-timeT-1000.0-arriveN-153-scheduleN-210',
            'MaxRange-2000/tx203 with Spray-nodeN-7-timeT-1000.0-arriveN-127-scheduleN-210',
            'MaxRange-2000/tx203 with TEG-nodeN-7-timeT-1000.0-arriveN-210-scheduleN-210',
            'MaxRange-2000/tx204 with CGR-nodeN-7-timeT-1000.0-arriveN-191-scheduleN-280',
            'MaxRange-2000/tx204 with DirectForward-nodeN-7-timeT-1000.0-arriveN-182-scheduleN-280',
            'MaxRange-2000/tx204 with Heuristic-nodeN-7-timeT-1000.0-arriveN-190-scheduleN-280',
            'MaxRange-2000/tx204 with Spray-nodeN-7-timeT-1000.0-arriveN-154-scheduleN-280',
            'MaxRange-2000/tx204 with TEG-nodeN-7-timeT-1000.0-arriveN-280-scheduleN-280',
            'MaxRange-2000/tx205 with CGR-nodeN-7-timeT-1000.0-arriveN-240-scheduleN-350',
            'MaxRange-2000/tx205 with DirectForward-nodeN-7-timeT-1000.0-arriveN-219-scheduleN-350',
            'MaxRange-2000/tx205 with Heuristic-nodeN-7-timeT-1000.0-arriveN-233-scheduleN-350',
            'MaxRange-2000/tx205 with Spray-nodeN-7-timeT-1000.0-arriveN-236-scheduleN-350',
            'MaxRange-2000/tx205 with TEG-nodeN-7-timeT-1000.0-arriveN-350-scheduleN-350',
            'MaxRange-2000/tx206 with CGR-nodeN-7-timeT-1000.0-arriveN-290-scheduleN-420',
            'MaxRange-2000/tx206 with DirectForward-nodeN-7-timeT-1000.0-arriveN-241-scheduleN-420',
            'MaxRange-2000/tx206 with Heuristic-nodeN-7-timeT-1000.0-arriveN-282-scheduleN-420',
            'MaxRange-2000/tx206 with Spray-nodeN-7-timeT-1000.0-arriveN-285-scheduleN-420',
            'MaxRange-2000/tx206 with TEG-nodeN-7-timeT-1000.0-arriveN-420-scheduleN-420',
            'MaxRange-2000/tx207 with CGR-nodeN-7-timeT-1000.0-arriveN-341-scheduleN-490',
            'MaxRange-2000/tx207 with DirectForward-nodeN-7-timeT-1000.0-arriveN-347-scheduleN-490',
            'MaxRange-2000/tx207 with Heuristic-nodeN-7-timeT-1000.0-arriveN-313-scheduleN-490',
            'MaxRange-2000/tx207 with Spray-nodeN-7-timeT-1000.0-arriveN-324-scheduleN-490',
            'MaxRange-2000/tx207 with TEG-nodeN-7-timeT-1000.0-arriveN-490-scheduleN-490',
            'MaxRange-2000/tx208 with CGR-nodeN-7-timeT-1000.0-arriveN-394-scheduleN-560',
            'MaxRange-2000/tx208 with DirectForward-nodeN-7-timeT-1000.0-arriveN-323-scheduleN-560',
            'MaxRange-2000/tx208 with Heuristic-nodeN-7-timeT-1000.0-arriveN-360-scheduleN-560',
            'MaxRange-2000/tx208 with Spray-nodeN-7-timeT-1000.0-arriveN-358-scheduleN-560',
            'MaxRange-2000/tx208 with TEG-nodeN-7-timeT-1000.0-arriveN-560-scheduleN-560',
            'MaxRange-2000/tx209 with CGR-nodeN-7-timeT-1000.0-arriveN-449-scheduleN-630',
            'MaxRange-2000/tx209 with DirectForward-nodeN-7-timeT-1000.0-arriveN-401-scheduleN-630',
            'MaxRange-2000/tx209 with Heuristic-nodeN-7-timeT-1000.0-arriveN-436-scheduleN-630',
            'MaxRange-2000/tx209 with Spray-nodeN-7-timeT-1000.0-arriveN-348-scheduleN-630',
            'MaxRange-2000/tx209 with TEG-nodeN-7-timeT-1000.0-arriveN-630-scheduleN-630',
            'MaxRange-2000/tx210 with CGR-nodeN-7-timeT-1000.0-arriveN-505-scheduleN-700',
            'MaxRange-2000/tx210 with DirectForward-nodeN-7-timeT-1000.0-arriveN-433-scheduleN-700',
            'MaxRange-2000/tx210 with Heuristic-nodeN-7-timeT-1000.0-arriveN-402-scheduleN-700',
            'MaxRange-2000/tx210 with Spray-nodeN-7-timeT-1000.0-arriveN-344-scheduleN-700',
            'MaxRange-2000/tx210 with TEG-nodeN-7-timeT-1000.0-arriveN-700-scheduleN-700',
            'MaxRange-2000/tx211 with CGR-nodeN-7-timeT-1000.0-arriveN-564-scheduleN-770',
            'MaxRange-2000/tx211 with DirectForward-nodeN-7-timeT-1000.0-arriveN-489-scheduleN-770',
            'MaxRange-2000/tx211 with Heuristic-nodeN-7-timeT-1000.0-arriveN-418-scheduleN-770',
            'MaxRange-2000/tx211 with Spray-nodeN-7-timeT-1000.0-arriveN-465-scheduleN-770',
            'MaxRange-2000/tx211 with TEG-nodeN-7-timeT-1000.0-arriveN-770-scheduleN-770',
            'MaxRange-2000/tx212 with CGR-nodeN-7-timeT-1000.0-arriveN-625-scheduleN-840',
            'MaxRange-2000/tx212 with DirectForward-nodeN-7-timeT-1000.0-arriveN-399-scheduleN-840',
            'MaxRange-2000/tx212 with Heuristic-nodeN-7-timeT-1000.0-arriveN-521-scheduleN-840',
            'MaxRange-2000/tx212 with Spray-nodeN-7-timeT-1000.0-arriveN-461-scheduleN-840',
            'MaxRange-2000/tx212 with TEG-nodeN-7-timeT-1000.0-arriveN-840-scheduleN-840',
            'MaxRange-2000/tx213 with CGR-nodeN-7-timeT-1000.0-arriveN-687-scheduleN-910',
            'MaxRange-2000/tx213 with DirectForward-nodeN-7-timeT-1000.0-arriveN-490-scheduleN-910',
            'MaxRange-2000/tx213 with Heuristic-nodeN-7-timeT-1000.0-arriveN-520-scheduleN-910',
            'MaxRange-2000/tx213 with Spray-nodeN-7-timeT-1000.0-arriveN-525-scheduleN-910',
            'MaxRange-2000/tx213 with TEG-nodeN-7-timeT-1000.0-arriveN-910-scheduleN-910',
            'MaxRange-2000/tx214 with CGR-nodeN-7-timeT-1000.0-arriveN-749-scheduleN-980',
            'MaxRange-2000/tx214 with DirectForward-nodeN-7-timeT-1000.0-arriveN-510-scheduleN-980',
            'MaxRange-2000/tx214 with Heuristic-nodeN-7-timeT-1000.0-arriveN-584-scheduleN-980',
            'MaxRange-2000/tx214 with Spray-nodeN-7-timeT-1000.0-arriveN-527-scheduleN-980',
            'MaxRange-2000/tx214 with TEG-nodeN-7-timeT-1000.0-arriveN-980-scheduleN-980',
            ]
    x_02_file_name_list = [
            'MaxRange-2000/ran301 with CGR-nodeN-6-timeT-1000.0-arriveN-17-scheduleN-51',
            'MaxRange-2000/ran301 with DirectForward-nodeN-6-timeT-1000.0-arriveN-2-scheduleN-51',
            'MaxRange-2000/ran301 with Heuristic-nodeN-6-timeT-1000.0-arriveN-2-scheduleN-51',
            'MaxRange-2000/ran301 with Spray-nodeN-6-timeT-1000.0-arriveN-2-scheduleN-51',
            'MaxRange-2000/ran301 with TEG-nodeN-6-timeT-1000.0-arriveN-17-scheduleN-51',
            'MaxRange-2000/ran302 with CGR-nodeN-6-timeT-1000.0-arriveN-68-scheduleN-102',
            'MaxRange-2000/ran302 with DirectForward-nodeN-6-timeT-1000.0-arriveN-24-scheduleN-102',
            'MaxRange-2000/ran302 with Heuristic-nodeN-6-timeT-1000.0-arriveN-24-scheduleN-102',
            'MaxRange-2000/ran302 with Spray-nodeN-6-timeT-1000.0-arriveN-24-scheduleN-102',
            'MaxRange-2000/ran302 with TEG-nodeN-6-timeT-1000.0-arriveN-68-scheduleN-102',
            'MaxRange-2000/ran303 with CGR-nodeN-6-timeT-1000.0-arriveN-119-scheduleN-153',
            'MaxRange-2000/ran303 with DirectForward-nodeN-6-timeT-1000.0-arriveN-60-scheduleN-153',
            'MaxRange-2000/ran303 with Heuristic-nodeN-6-timeT-1000.0-arriveN-60-scheduleN-153',
            'MaxRange-2000/ran303 with Spray-nodeN-6-timeT-1000.0-arriveN-60-scheduleN-153',
            'MaxRange-2000/ran303 with TEG-nodeN-6-timeT-1000.0-arriveN-119-scheduleN-153',
            'MaxRange-2000/ran304 with CGR-nodeN-6-timeT-1000.0-arriveN-153-scheduleN-204',
            'MaxRange-2000/ran304 with DirectForward-nodeN-6-timeT-1000.0-arriveN-94-scheduleN-204',
            'MaxRange-2000/ran304 with Heuristic-nodeN-6-timeT-1000.0-arriveN-94-scheduleN-204',
            'MaxRange-2000/ran304 with Spray-nodeN-6-timeT-1000.0-arriveN-94-scheduleN-204',
            'MaxRange-2000/ran304 with TEG-nodeN-6-timeT-1000.0-arriveN-153-scheduleN-204',
            'MaxRange-2000/ran305 with CGR-nodeN-6-timeT-1000.0-arriveN-170-scheduleN-255',
            'MaxRange-2000/ran305 with DirectForward-nodeN-6-timeT-1000.0-arriveN-111-scheduleN-255',
            'MaxRange-2000/ran305 with Heuristic-nodeN-6-timeT-1000.0-arriveN-111-scheduleN-255',
            'MaxRange-2000/ran305 with Spray-nodeN-6-timeT-1000.0-arriveN-111-scheduleN-255',
            'MaxRange-2000/ran305 with TEG-nodeN-6-timeT-1000.0-arriveN-170-scheduleN-255',
            'MaxRange-2000/ran306 with CGR-nodeN-6-timeT-1000.0-arriveN-187-scheduleN-306',
            'MaxRange-2000/ran306 with DirectForward-nodeN-6-timeT-1000.0-arriveN-120-scheduleN-306',
            'MaxRange-2000/ran306 with Heuristic-nodeN-6-timeT-1000.0-arriveN-120-scheduleN-306',
            'MaxRange-2000/ran306 with Spray-nodeN-6-timeT-1000.0-arriveN-120-scheduleN-306',
            'MaxRange-2000/ran306 with TEG-nodeN-6-timeT-1000.0-arriveN-187-scheduleN-306',
            'MaxRange-2000/ran307 with CGR-nodeN-6-timeT-1000.0-arriveN-221-scheduleN-357',
            'MaxRange-2000/ran307 with DirectForward-nodeN-6-timeT-1000.0-arriveN-166-scheduleN-357',
            'MaxRange-2000/ran307 with Heuristic-nodeN-6-timeT-1000.0-arriveN-166-scheduleN-357',
            'MaxRange-2000/ran307 with Spray-nodeN-6-timeT-1000.0-arriveN-166-scheduleN-357',
            'MaxRange-2000/ran307 with TEG-nodeN-6-timeT-1000.0-arriveN-221-scheduleN-357',
            'MaxRange-2000/ran308 with CGR-nodeN-6-timeT-1000.0-arriveN-266-scheduleN-408',
            'MaxRange-2000/ran308 with DirectForward-nodeN-6-timeT-1000.0-arriveN-184-scheduleN-408',
            'MaxRange-2000/ran308 with Heuristic-nodeN-6-timeT-1000.0-arriveN-184-scheduleN-408',
            'MaxRange-2000/ran308 with Spray-nodeN-6-timeT-1000.0-arriveN-184-scheduleN-408',
            'MaxRange-2000/ran308 with TEG-nodeN-6-timeT-1000.0-arriveN-266-scheduleN-408',
            'MaxRange-2000/ran309 with CGR-nodeN-6-timeT-1000.0-arriveN-283-scheduleN-459',
            'MaxRange-2000/ran309 with DirectForward-nodeN-6-timeT-1000.0-arriveN-201-scheduleN-459',
            'MaxRange-2000/ran309 with Heuristic-nodeN-6-timeT-1000.0-arriveN-201-scheduleN-459',
            'MaxRange-2000/ran309 with Spray-nodeN-6-timeT-1000.0-arriveN-201-scheduleN-459',
            'MaxRange-2000/ran309 with TEG-nodeN-6-timeT-1000.0-arriveN-283-scheduleN-459',
            'MaxRange-2000/ran310 with CGR-nodeN-6-timeT-1000.0-arriveN-333-scheduleN-510',
            'MaxRange-2000/ran310 with DirectForward-nodeN-6-timeT-1000.0-arriveN-217-scheduleN-510',
            'MaxRange-2000/ran310 with Heuristic-nodeN-6-timeT-1000.0-arriveN-217-scheduleN-510',
            'MaxRange-2000/ran310 with Spray-nodeN-6-timeT-1000.0-arriveN-217-scheduleN-510',
            'MaxRange-2000/ran310 with TEG-nodeN-6-timeT-1000.0-arriveN-333-scheduleN-510',
            'MaxRange-2000/ran311 with CGR-nodeN-6-timeT-1000.0-arriveN-382-scheduleN-561',
            'MaxRange-2000/ran311 with DirectForward-nodeN-6-timeT-1000.0-arriveN-228-scheduleN-561',
            'MaxRange-2000/ran311 with Heuristic-nodeN-6-timeT-1000.0-arriveN-228-scheduleN-561',
            'MaxRange-2000/ran311 with Spray-nodeN-6-timeT-1000.0-arriveN-228-scheduleN-561',
            'MaxRange-2000/ran311 with TEG-nodeN-6-timeT-1000.0-arriveN-382-scheduleN-561',
            'MaxRange-2000/ran312 with CGR-nodeN-6-timeT-1000.0-arriveN-432-scheduleN-612',
            'MaxRange-2000/ran312 with DirectForward-nodeN-6-timeT-1000.0-arriveN-243-scheduleN-612',
            'MaxRange-2000/ran312 with Heuristic-nodeN-6-timeT-1000.0-arriveN-243-scheduleN-612',
            'MaxRange-2000/ran312 with Spray-nodeN-6-timeT-1000.0-arriveN-243-scheduleN-612',
            'MaxRange-2000/ran312 with TEG-nodeN-6-timeT-1000.0-arriveN-432-scheduleN-612',
            'MaxRange-2000/ran313 with CGR-nodeN-6-timeT-1000.0-arriveN-481-scheduleN-663',
            'MaxRange-2000/ran313 with DirectForward-nodeN-6-timeT-1000.0-arriveN-258-scheduleN-663',
            'MaxRange-2000/ran313 with Heuristic-nodeN-6-timeT-1000.0-arriveN-258-scheduleN-663',
            'MaxRange-2000/ran313 with Spray-nodeN-6-timeT-1000.0-arriveN-258-scheduleN-663',
            'MaxRange-2000/ran313 with TEG-nodeN-6-timeT-1000.0-arriveN-481-scheduleN-663',
            'MaxRange-2000/ran314 with CGR-nodeN-6-timeT-1000.0-arriveN-515-scheduleN-714',
            'MaxRange-2000/ran314 with DirectForward-nodeN-6-timeT-1000.0-arriveN-286-scheduleN-714',
            'MaxRange-2000/ran314 with Heuristic-nodeN-6-timeT-1000.0-arriveN-286-scheduleN-714',
            'MaxRange-2000/ran314 with Spray-nodeN-6-timeT-1000.0-arriveN-286-scheduleN-714',
            'MaxRange-2000/ran314 with TEG-nodeN-6-timeT-1000.0-arriveN-515-scheduleN-714',
            ]
    x_03_file_name_list = [
            'MaxRange-3000/tx201 with CGR-nodeN-7-timeT-1000.0-arriveN-70-scheduleN-70',
            'MaxRange-3000/tx201 with DirectForward-nodeN-7-timeT-1000.0-arriveN-42-scheduleN-70',
            'MaxRange-3000/tx201 with Heuristic-nodeN-7-timeT-1000.0-arriveN-43-scheduleN-70',
            'MaxRange-3000/tx201 with Spray-nodeN-7-timeT-1000.0-arriveN-42-scheduleN-70',
            'MaxRange-3000/tx201 with TEG-nodeN-7-timeT-1000.0-arriveN-70-scheduleN-70',
            'MaxRange-3000/tx202 with CGR-nodeN-7-timeT-1000.0-arriveN-140-scheduleN-140',
            'MaxRange-3000/tx202 with DirectForward-nodeN-7-timeT-1000.0-arriveN-73-scheduleN-140',
            'MaxRange-3000/tx202 with Heuristic-nodeN-7-timeT-1000.0-arriveN-98-scheduleN-140',
            'MaxRange-3000/tx202 with Spray-nodeN-7-timeT-1000.0-arriveN-78-scheduleN-140',
            'MaxRange-3000/tx202 with TEG-nodeN-7-timeT-1000.0-arriveN-140-scheduleN-140',
            'MaxRange-3000/tx203 with CGR-nodeN-7-timeT-1000.0-arriveN-210-scheduleN-210',
            'MaxRange-3000/tx203 with DirectForward-nodeN-7-timeT-1000.0-arriveN-125-scheduleN-210',
            'MaxRange-3000/tx203 with Heuristic-nodeN-7-timeT-1000.0-arriveN-146-scheduleN-210',
            'MaxRange-3000/tx203 with Spray-nodeN-7-timeT-1000.0-arriveN-136-scheduleN-210',
            'MaxRange-3000/tx203 with TEG-nodeN-7-timeT-1000.0-arriveN-210-scheduleN-210',
            'MaxRange-3000/tx204 with CGR-nodeN-7-timeT-1000.0-arriveN-280-scheduleN-280',
            'MaxRange-3000/tx204 with DirectForward-nodeN-7-timeT-1000.0-arriveN-166-scheduleN-280',
            'MaxRange-3000/tx204 with Heuristic-nodeN-7-timeT-1000.0-arriveN-189-scheduleN-280',
            'MaxRange-3000/tx204 with Spray-nodeN-7-timeT-1000.0-arriveN-183-scheduleN-280',
            'MaxRange-3000/tx204 with TEG-nodeN-7-timeT-1000.0-arriveN-280-scheduleN-280',
            'MaxRange-3000/tx205 with CGR-nodeN-7-timeT-1000.0-arriveN-350-scheduleN-350',
            'MaxRange-3000/tx205 with DirectForward-nodeN-7-timeT-1000.0-arriveN-213-scheduleN-350',
            'MaxRange-3000/tx205 with Heuristic-nodeN-7-timeT-1000.0-arriveN-252-scheduleN-350',
            'MaxRange-3000/tx205 with Spray-nodeN-7-timeT-1000.0-arriveN-212-scheduleN-350',
            'MaxRange-3000/tx205 with TEG-nodeN-7-timeT-1000.0-arriveN-350-scheduleN-350',
            'MaxRange-3000/tx206 with CGR-nodeN-7-timeT-1000.0-arriveN-420-scheduleN-420',
            'MaxRange-3000/tx206 with DirectForward-nodeN-7-timeT-1000.0-arriveN-282-scheduleN-420',
            'MaxRange-3000/tx206 with Heuristic-nodeN-7-timeT-1000.0-arriveN-284-scheduleN-420',
            'MaxRange-3000/tx206 with Spray-nodeN-7-timeT-1000.0-arriveN-285-scheduleN-420',
            'MaxRange-3000/tx206 with TEG-nodeN-7-timeT-1000.0-arriveN-420-scheduleN-420',
            'MaxRange-3000/tx207 with CGR-nodeN-7-timeT-1000.0-arriveN-490-scheduleN-490',
            'MaxRange-3000/tx207 with DirectForward-nodeN-7-timeT-1000.0-arriveN-286-scheduleN-490',
            'MaxRange-3000/tx207 with Heuristic-nodeN-7-timeT-1000.0-arriveN-318-scheduleN-490',
            'MaxRange-3000/tx207 with Spray-nodeN-7-timeT-1000.0-arriveN-313-scheduleN-490',
            'MaxRange-3000/tx207 with TEG-nodeN-7-timeT-1000.0-arriveN-490-scheduleN-490',
            'MaxRange-3000/tx208 with CGR-nodeN-7-timeT-1000.0-arriveN-560-scheduleN-560',
            'MaxRange-3000/tx208 with DirectForward-nodeN-7-timeT-1000.0-arriveN-347-scheduleN-560',
            'MaxRange-3000/tx208 with Heuristic-nodeN-7-timeT-1000.0-arriveN-412-scheduleN-560',
            'MaxRange-3000/tx208 with Spray-nodeN-7-timeT-1000.0-arriveN-355-scheduleN-560',
            'MaxRange-3000/tx208 with TEG-nodeN-7-timeT-1000.0-arriveN-560-scheduleN-560',
            'MaxRange-3000/tx209 with CGR-nodeN-7-timeT-1000.0-arriveN-630-scheduleN-630',
            'MaxRange-3000/tx209 with DirectForward-nodeN-7-timeT-1000.0-arriveN-403-scheduleN-630',
            'MaxRange-3000/tx209 with Heuristic-nodeN-7-timeT-1000.0-arriveN-446-scheduleN-630',
            'MaxRange-3000/tx209 with Spray-nodeN-7-timeT-1000.0-arriveN-422-scheduleN-630',
            'MaxRange-3000/tx209 with TEG-nodeN-7-timeT-1000.0-arriveN-630-scheduleN-630',
            'MaxRange-3000/tx210 with CGR-nodeN-7-timeT-1000.0-arriveN-700-scheduleN-700',
            'MaxRange-3000/tx210 with DirectForward-nodeN-7-timeT-1000.0-arriveN-439-scheduleN-700',
            'MaxRange-3000/tx210 with Heuristic-nodeN-7-timeT-1000.0-arriveN-478-scheduleN-700',
            'MaxRange-3000/tx210 with Spray-nodeN-7-timeT-1000.0-arriveN-437-scheduleN-700',
            'MaxRange-3000/tx210 with TEG-nodeN-7-timeT-1000.0-arriveN-700-scheduleN-700',
            'MaxRange-3000/tx211 with CGR-nodeN-7-timeT-1000.0-arriveN-770-scheduleN-770',
            'MaxRange-3000/tx211 with DirectForward-nodeN-7-timeT-1000.0-arriveN-479-scheduleN-770',
            'MaxRange-3000/tx211 with Heuristic-nodeN-7-timeT-1000.0-arriveN-551-scheduleN-770',
            'MaxRange-3000/tx211 with Spray-nodeN-7-timeT-1000.0-arriveN-489-scheduleN-770',
            'MaxRange-3000/tx211 with TEG-nodeN-7-timeT-1000.0-arriveN-770-scheduleN-770',
            'MaxRange-3000/tx212 with CGR-nodeN-7-timeT-1000.0-arriveN-840-scheduleN-840',
            'MaxRange-3000/tx212 with DirectForward-nodeN-7-timeT-1000.0-arriveN-545-scheduleN-840',
            'MaxRange-3000/tx212 with Heuristic-nodeN-7-timeT-1000.0-arriveN-516-scheduleN-840',
            'MaxRange-3000/tx212 with Spray-nodeN-7-timeT-1000.0-arriveN-551-scheduleN-840',
            'MaxRange-3000/tx212 with TEG-nodeN-7-timeT-1000.0-arriveN-840-scheduleN-840',
            'MaxRange-3000/tx213 with CGR-nodeN-7-timeT-1000.0-arriveN-910-scheduleN-910',
            'MaxRange-3000/tx213 with DirectForward-nodeN-7-timeT-1000.0-arriveN-500-scheduleN-910',
            'MaxRange-3000/tx213 with Heuristic-nodeN-7-timeT-1000.0-arriveN-647-scheduleN-910',
            'MaxRange-3000/tx213 with Spray-nodeN-7-timeT-1000.0-arriveN-510-scheduleN-910',
            'MaxRange-3000/tx213 with TEG-nodeN-7-timeT-1000.0-arriveN-910-scheduleN-910',
            'MaxRange-3000/tx214 with CGR-nodeN-7-timeT-1000.0-arriveN-980-scheduleN-980',
            'MaxRange-3000/tx214 with DirectForward-nodeN-7-timeT-1000.0-arriveN-588-scheduleN-980',
            'MaxRange-3000/tx214 with Heuristic-nodeN-7-timeT-1000.0-arriveN-692-scheduleN-980',
            'MaxRange-3000/tx214 with Spray-nodeN-7-timeT-1000.0-arriveN-602-scheduleN-980',
            'MaxRange-3000/tx214 with TEG-nodeN-7-timeT-1000.0-arriveN-980-scheduleN-980',
            ]
    x_04_file_name_list = [
            'MaxRange-3000/ran301 with CGR-nodeN-6-timeT-1000.0-arriveN-51-scheduleN-51',
            'MaxRange-3000/ran301 with DirectForward-nodeN-6-timeT-1000.0-arriveN-23-scheduleN-51',
            'MaxRange-3000/ran301 with Heuristic-nodeN-6-timeT-1000.0-arriveN-23-scheduleN-51',
            'MaxRange-3000/ran301 with Spray-nodeN-6-timeT-1000.0-arriveN-23-scheduleN-51',
            'MaxRange-3000/ran301 with TEG-nodeN-6-timeT-1000.0-arriveN-51-scheduleN-51',
            'MaxRange-3000/ran302 with CGR-nodeN-6-timeT-1000.0-arriveN-102-scheduleN-102',
            'MaxRange-3000/ran302 with DirectForward-nodeN-6-timeT-1000.0-arriveN-57-scheduleN-102',
            'MaxRange-3000/ran302 with Heuristic-nodeN-6-timeT-1000.0-arriveN-57-scheduleN-102',
            'MaxRange-3000/ran302 with Spray-nodeN-6-timeT-1000.0-arriveN-57-scheduleN-102',
            'MaxRange-3000/ran302 with TEG-nodeN-6-timeT-1000.0-arriveN-102-scheduleN-102',
            'MaxRange-3000/ran303 with CGR-nodeN-6-timeT-1000.0-arriveN-153-scheduleN-153',
            'MaxRange-3000/ran303 with DirectForward-nodeN-6-timeT-1000.0-arriveN-88-scheduleN-153',
            'MaxRange-3000/ran303 with Heuristic-nodeN-6-timeT-1000.0-arriveN-88-scheduleN-153',
            'MaxRange-3000/ran303 with Spray-nodeN-6-timeT-1000.0-arriveN-88-scheduleN-153',
            'MaxRange-3000/ran303 with TEG-nodeN-6-timeT-1000.0-arriveN-153-scheduleN-153',
            'MaxRange-3000/ran304 with CGR-nodeN-6-timeT-1000.0-arriveN-204-scheduleN-204',
            'MaxRange-3000/ran304 with DirectForward-nodeN-6-timeT-1000.0-arriveN-96-scheduleN-204',
            'MaxRange-3000/ran304 with Heuristic-nodeN-6-timeT-1000.0-arriveN-96-scheduleN-204',
            'MaxRange-3000/ran304 with Spray-nodeN-6-timeT-1000.0-arriveN-96-scheduleN-204',
            'MaxRange-3000/ran304 with TEG-nodeN-6-timeT-1000.0-arriveN-194-scheduleN-204',
            'MaxRange-3000/ran305 with CGR-nodeN-6-timeT-1000.0-arriveN-255-scheduleN-255',
            'MaxRange-3000/ran305 with DirectForward-nodeN-6-timeT-1000.0-arriveN-150-scheduleN-255',
            'MaxRange-3000/ran305 with Heuristic-nodeN-6-timeT-1000.0-arriveN-150-scheduleN-255',
            'MaxRange-3000/ran305 with Spray-nodeN-6-timeT-1000.0-arriveN-150-scheduleN-255',
            'MaxRange-3000/ran305 with TEG-nodeN-6-timeT-1000.0-arriveN-243-scheduleN-255',
            'MaxRange-3000/ran306 with CGR-nodeN-6-timeT-1000.0-arriveN-287-scheduleN-306',
            'MaxRange-3000/ran306 with DirectForward-nodeN-6-timeT-1000.0-arriveN-157-scheduleN-306',
            'MaxRange-3000/ran306 with Heuristic-nodeN-6-timeT-1000.0-arriveN-157-scheduleN-306',
            'MaxRange-3000/ran306 with Spray-nodeN-6-timeT-1000.0-arriveN-157-scheduleN-306',
            'MaxRange-3000/ran306 with TEG-nodeN-6-timeT-1000.0-arriveN-272-scheduleN-306',
            'MaxRange-3000/ran307 with CGR-nodeN-6-timeT-1000.0-arriveN-339-scheduleN-357',
            'MaxRange-3000/ran307 with DirectForward-nodeN-6-timeT-1000.0-arriveN-193-scheduleN-357',
            'MaxRange-3000/ran307 with Heuristic-nodeN-6-timeT-1000.0-arriveN-193-scheduleN-357',
            'MaxRange-3000/ran307 with Spray-nodeN-6-timeT-1000.0-arriveN-193-scheduleN-357',
            'MaxRange-3000/ran307 with TEG-nodeN-6-timeT-1000.0-arriveN-315-scheduleN-357',
            'MaxRange-3000/ran308 with CGR-nodeN-6-timeT-1000.0-arriveN-386-scheduleN-408',
            'MaxRange-3000/ran308 with DirectForward-nodeN-6-timeT-1000.0-arriveN-235-scheduleN-408',
            'MaxRange-3000/ran308 with Heuristic-nodeN-6-timeT-1000.0-arriveN-235-scheduleN-408',
            'MaxRange-3000/ran308 with Spray-nodeN-6-timeT-1000.0-arriveN-235-scheduleN-408',
            'MaxRange-3000/ran308 with TEG-nodeN-6-timeT-1000.0-arriveN-360-scheduleN-408',
            'MaxRange-3000/ran309 with CGR-nodeN-6-timeT-1000.0-arriveN-437-scheduleN-459',
            'MaxRange-3000/ran309 with DirectForward-nodeN-6-timeT-1000.0-arriveN-287-scheduleN-459',
            'MaxRange-3000/ran309 with Heuristic-nodeN-6-timeT-1000.0-arriveN-287-scheduleN-459',
            'MaxRange-3000/ran309 with Spray-nodeN-6-timeT-1000.0-arriveN-287-scheduleN-459',
            'MaxRange-3000/ran309 with TEG-nodeN-6-timeT-1000.0-arriveN-410-scheduleN-459',
            'MaxRange-3000/ran310 with CGR-nodeN-6-timeT-1000.0-arriveN-486-scheduleN-510',
            'MaxRange-3000/ran310 with DirectForward-nodeN-6-timeT-1000.0-arriveN-301-scheduleN-510',
            'MaxRange-3000/ran310 with Heuristic-nodeN-6-timeT-1000.0-arriveN-301-scheduleN-510',
            'MaxRange-3000/ran310 with Spray-nodeN-6-timeT-1000.0-arriveN-301-scheduleN-510',
            'MaxRange-3000/ran310 with TEG-nodeN-6-timeT-1000.0-arriveN-460-scheduleN-510',
            'MaxRange-3000/ran311 with CGR-nodeN-6-timeT-1000.0-arriveN-536-scheduleN-561',
            'MaxRange-3000/ran311 with DirectForward-nodeN-6-timeT-1000.0-arriveN-320-scheduleN-561',
            'MaxRange-3000/ran311 with Heuristic-nodeN-6-timeT-1000.0-arriveN-320-scheduleN-561',
            'MaxRange-3000/ran311 with Spray-nodeN-6-timeT-1000.0-arriveN-319-scheduleN-561',
            'MaxRange-3000/ran311 with TEG-nodeN-6-timeT-1000.0-arriveN-510-scheduleN-561',
            'MaxRange-3000/ran312 with CGR-nodeN-6-timeT-1000.0-arriveN-585-scheduleN-612',
            'MaxRange-3000/ran312 with DirectForward-nodeN-6-timeT-1000.0-arriveN-352-scheduleN-612',
            'MaxRange-3000/ran312 with Heuristic-nodeN-6-timeT-1000.0-arriveN-352-scheduleN-612',
            'MaxRange-3000/ran312 with Spray-nodeN-6-timeT-1000.0-arriveN-352-scheduleN-612',
            'MaxRange-3000/ran312 with TEG-nodeN-6-timeT-1000.0-arriveN-561-scheduleN-612',
            'MaxRange-3000/ran313 with CGR-nodeN-6-timeT-1000.0-arriveN-634-scheduleN-663',
            'MaxRange-3000/ran313 with DirectForward-nodeN-6-timeT-1000.0-arriveN-389-scheduleN-663',
            'MaxRange-3000/ran313 with Heuristic-nodeN-6-timeT-1000.0-arriveN-389-scheduleN-663',
            'MaxRange-3000/ran313 with Spray-nodeN-6-timeT-1000.0-arriveN-389-scheduleN-663',
            'MaxRange-3000/ran313 with TEG-nodeN-6-timeT-1000.0-arriveN-611-scheduleN-663',
            'MaxRange-3000/ran314 with CGR-nodeN-6-timeT-1000.0-arriveN-684-scheduleN-714',
            'MaxRange-3000/ran314 with DirectForward-nodeN-6-timeT-1000.0-arriveN-405-scheduleN-714',
            'MaxRange-3000/ran314 with Heuristic-nodeN-6-timeT-1000.0-arriveN-405-scheduleN-714',
            'MaxRange-3000/ran314 with Spray-nodeN-6-timeT-1000.0-arriveN-406-scheduleN-714',
            'MaxRange-3000/ran314 with TEG-nodeN-6-timeT-1000.0-arriveN-652-scheduleN-714',
            ]
    x_05_file_name_list =  [
            'MaxRange-4000/tx201 with CGR-nodeN-7-timeT-1000.0-arriveN-70-scheduleN-70',
            'MaxRange-4000/tx201 with DirectForward-nodeN-7-timeT-1000.0-arriveN-35-scheduleN-70',
            'MaxRange-4000/tx201 with Heuristic-nodeN-7-timeT-1000.0-arriveN-50-scheduleN-70',
            'MaxRange-4000/tx201 with Spray-nodeN-7-timeT-1000.0-arriveN-64-scheduleN-70',
            'MaxRange-4000/tx201 with TEG-nodeN-7-timeT-1000.0-arriveN-70-scheduleN-70',
            'MaxRange-4000/tx202 with CGR-nodeN-7-timeT-1000.0-arriveN-140-scheduleN-140',
            'MaxRange-4000/tx202 with DirectForward-nodeN-7-timeT-1000.0-arriveN-73-scheduleN-140',
            'MaxRange-4000/tx202 with Heuristic-nodeN-7-timeT-1000.0-arriveN-102-scheduleN-140',
            'MaxRange-4000/tx202 with Spray-nodeN-7-timeT-1000.0-arriveN-130-scheduleN-140',
            'MaxRange-4000/tx202 with TEG-nodeN-7-timeT-1000.0-arriveN-140-scheduleN-140',
            'MaxRange-4000/tx203 with CGR-nodeN-7-timeT-1000.0-arriveN-210-scheduleN-210',
            'MaxRange-4000/tx203 with DirectForward-nodeN-7-timeT-1000.0-arriveN-95-scheduleN-210',
            'MaxRange-4000/tx203 with Heuristic-nodeN-7-timeT-1000.0-arriveN-155-scheduleN-210',
            'MaxRange-4000/tx203 with Spray-nodeN-7-timeT-1000.0-arriveN-198-scheduleN-210',
            'MaxRange-4000/tx203 with TEG-nodeN-7-timeT-1000.0-arriveN-210-scheduleN-210',
            'MaxRange-4000/tx204 with CGR-nodeN-7-timeT-1000.0-arriveN-280-scheduleN-280',
            'MaxRange-4000/tx204 with DirectForward-nodeN-7-timeT-1000.0-arriveN-162-scheduleN-280',
            'MaxRange-4000/tx204 with Heuristic-nodeN-7-timeT-1000.0-arriveN-206-scheduleN-280',
            'MaxRange-4000/tx204 with Spray-nodeN-7-timeT-1000.0-arriveN-264-scheduleN-280',
            'MaxRange-4000/tx204 with TEG-nodeN-7-timeT-1000.0-arriveN-280-scheduleN-280',
            'MaxRange-4000/tx205 with CGR-nodeN-7-timeT-1000.0-arriveN-350-scheduleN-350',
            'MaxRange-4000/tx205 with DirectForward-nodeN-7-timeT-1000.0-arriveN-195-scheduleN-350',
            'MaxRange-4000/tx205 with Heuristic-nodeN-7-timeT-1000.0-arriveN-268-scheduleN-350',
            'MaxRange-4000/tx205 with Spray-nodeN-7-timeT-1000.0-arriveN-334-scheduleN-350',
            'MaxRange-4000/tx205 with TEG-nodeN-7-timeT-1000.0-arriveN-350-scheduleN-350',
            'MaxRange-4000/tx206 with CGR-nodeN-7-timeT-1000.0-arriveN-420-scheduleN-420',
            'MaxRange-4000/tx206 with DirectForward-nodeN-7-timeT-1000.0-arriveN-221-scheduleN-420',
            'MaxRange-4000/tx206 with Heuristic-nodeN-7-timeT-1000.0-arriveN-328-scheduleN-420',
            'MaxRange-4000/tx206 with Spray-nodeN-7-timeT-1000.0-arriveN-402-scheduleN-420',
            'MaxRange-4000/tx206 with TEG-nodeN-7-timeT-1000.0-arriveN-420-scheduleN-420',
            'MaxRange-4000/tx207 with CGR-nodeN-7-timeT-1000.0-arriveN-490-scheduleN-490',
            'MaxRange-4000/tx207 with DirectForward-nodeN-7-timeT-1000.0-arriveN-278-scheduleN-490',
            'MaxRange-4000/tx207 with Heuristic-nodeN-7-timeT-1000.0-arriveN-393-scheduleN-490',
            'MaxRange-4000/tx207 with Spray-nodeN-7-timeT-1000.0-arriveN-470-scheduleN-490',
            'MaxRange-4000/tx207 with TEG-nodeN-7-timeT-1000.0-arriveN-490-scheduleN-490',
            'MaxRange-4000/tx208 with CGR-nodeN-7-timeT-1000.0-arriveN-560-scheduleN-560',
            'MaxRange-4000/tx208 with DirectForward-nodeN-7-timeT-1000.0-arriveN-327-scheduleN-560',
            'MaxRange-4000/tx208 with Heuristic-nodeN-7-timeT-1000.0-arriveN-453-scheduleN-560',
            'MaxRange-4000/tx208 with Spray-nodeN-7-timeT-1000.0-arriveN-543-scheduleN-560',
            'MaxRange-4000/tx208 with TEG-nodeN-7-timeT-1000.0-arriveN-560-scheduleN-560',
            'MaxRange-4000/tx209 with CGR-nodeN-7-timeT-1000.0-arriveN-630-scheduleN-630',
            'MaxRange-4000/tx209 with DirectForward-nodeN-7-timeT-1000.0-arriveN-337-scheduleN-630',
            'MaxRange-4000/tx209 with Heuristic-nodeN-7-timeT-1000.0-arriveN-496-scheduleN-630',
            'MaxRange-4000/tx209 with Spray-nodeN-7-timeT-1000.0-arriveN-612-scheduleN-630',
            'MaxRange-4000/tx209 with TEG-nodeN-7-timeT-1000.0-arriveN-630-scheduleN-630',
            'MaxRange-4000/tx210 with CGR-nodeN-7-timeT-1000.0-arriveN-700-scheduleN-700',
            'MaxRange-4000/tx210 with DirectForward-nodeN-7-timeT-1000.0-arriveN-406-scheduleN-700',
            'MaxRange-4000/tx210 with Heuristic-nodeN-7-timeT-1000.0-arriveN-565-scheduleN-700',
            'MaxRange-4000/tx210 with Spray-nodeN-7-timeT-1000.0-arriveN-680-scheduleN-700',
            'MaxRange-4000/tx210 with TEG-nodeN-7-timeT-1000.0-arriveN-700-scheduleN-700',
            'MaxRange-4000/tx211 with CGR-nodeN-7-timeT-1000.0-arriveN-770-scheduleN-770',
            'MaxRange-4000/tx211 with DirectForward-nodeN-7-timeT-1000.0-arriveN-429-scheduleN-770',
            'MaxRange-4000/tx211 with Heuristic-nodeN-7-timeT-1000.0-arriveN-611-scheduleN-770',
            'MaxRange-4000/tx211 with Spray-nodeN-7-timeT-1000.0-arriveN-749-scheduleN-770',
            'MaxRange-4000/tx211 with TEG-nodeN-7-timeT-1000.0-arriveN-770-scheduleN-770',
            'MaxRange-4000/tx212 with CGR-nodeN-7-timeT-1000.0-arriveN-840-scheduleN-840',
            'MaxRange-4000/tx212 with DirectForward-nodeN-7-timeT-1000.0-arriveN-484-scheduleN-840',
            'MaxRange-4000/tx212 with Heuristic-nodeN-7-timeT-1000.0-arriveN-680-scheduleN-840',
            'MaxRange-4000/tx212 with Spray-nodeN-7-timeT-1000.0-arriveN-817-scheduleN-840',
            'MaxRange-4000/tx212 with TEG-nodeN-7-timeT-1000.0-arriveN-840-scheduleN-840',
            'MaxRange-4000/tx213 with CGR-nodeN-7-timeT-1000.0-arriveN-910-scheduleN-910',
            'MaxRange-4000/tx213 with DirectForward-nodeN-7-timeT-1000.0-arriveN-511-scheduleN-910',
            'MaxRange-4000/tx213 with Heuristic-nodeN-7-timeT-1000.0-arriveN-723-scheduleN-910',
            'MaxRange-4000/tx213 with Spray-nodeN-7-timeT-1000.0-arriveN-874-scheduleN-910',
            'MaxRange-4000/tx213 with TEG-nodeN-7-timeT-1000.0-arriveN-910-scheduleN-910',
            'MaxRange-4000/tx214 with CGR-nodeN-7-timeT-1000.0-arriveN-980-scheduleN-980',
            'MaxRange-4000/tx214 with DirectForward-nodeN-7-timeT-1000.0-arriveN-558-scheduleN-980',
            'MaxRange-4000/tx214 with Heuristic-nodeN-7-timeT-1000.0-arriveN-772-scheduleN-980',
            'MaxRange-4000/tx214 with Spray-nodeN-7-timeT-1000.0-arriveN-948-scheduleN-980',
            'MaxRange-4000/tx214 with TEG-nodeN-7-timeT-1000.0-arriveN-980-scheduleN-980',
            ]
    x_06_file_name_list = [
            'MaxRange-4000/ran301 with CGR-nodeN-6-timeT-1000.0-arriveN-51-scheduleN-51',
            'MaxRange-4000/ran301 with DirectForward-nodeN-6-timeT-1000.0-arriveN-28-scheduleN-51',
            'MaxRange-4000/ran301 with Heuristic-nodeN-6-timeT-1000.0-arriveN-28-scheduleN-51',
            'MaxRange-4000/ran301 with Spray-nodeN-6-timeT-1000.0-arriveN-51-scheduleN-51',
            'MaxRange-4000/ran301 with TEG-nodeN-6-timeT-1000.0-arriveN-51-scheduleN-51',
            'MaxRange-4000/ran302 with CGR-nodeN-6-timeT-1000.0-arriveN-102-scheduleN-102',
            'MaxRange-4000/ran302 with DirectForward-nodeN-6-timeT-1000.0-arriveN-53-scheduleN-102',
            'MaxRange-4000/ran302 with Heuristic-nodeN-6-timeT-1000.0-arriveN-53-scheduleN-102',
            'MaxRange-4000/ran302 with Spray-nodeN-6-timeT-1000.0-arriveN-102-scheduleN-102',
            'MaxRange-4000/ran302 with TEG-nodeN-6-timeT-1000.0-arriveN-102-scheduleN-102',
            'MaxRange-4000/ran303 with CGR-nodeN-6-timeT-1000.0-arriveN-153-scheduleN-153',
            'MaxRange-4000/ran303 with DirectForward-nodeN-6-timeT-1000.0-arriveN-87-scheduleN-153',
            'MaxRange-4000/ran303 with Heuristic-nodeN-6-timeT-1000.0-arriveN-87-scheduleN-153',
            'MaxRange-4000/ran303 with Spray-nodeN-6-timeT-1000.0-arriveN-153-scheduleN-153',
            'MaxRange-4000/ran303 with TEG-nodeN-6-timeT-1000.0-arriveN-153-scheduleN-153',
            'MaxRange-4000/ran304 with CGR-nodeN-6-timeT-1000.0-arriveN-204-scheduleN-204',
            'MaxRange-4000/ran304 with DirectForward-nodeN-6-timeT-1000.0-arriveN-114-scheduleN-204',
            'MaxRange-4000/ran304 with Heuristic-nodeN-6-timeT-1000.0-arriveN-115-scheduleN-204',
            'MaxRange-4000/ran304 with Spray-nodeN-6-timeT-1000.0-arriveN-200-scheduleN-204',
            'MaxRange-4000/ran304 with TEG-nodeN-6-timeT-1000.0-arriveN-204-scheduleN-204',
            'MaxRange-4000/ran305 with CGR-nodeN-6-timeT-1000.0-arriveN-255-scheduleN-255',
            'MaxRange-4000/ran305 with DirectForward-nodeN-6-timeT-1000.0-arriveN-165-scheduleN-255',
            'MaxRange-4000/ran305 with Heuristic-nodeN-6-timeT-1000.0-arriveN-165-scheduleN-255',
            'MaxRange-4000/ran305 with Spray-nodeN-6-timeT-1000.0-arriveN-250-scheduleN-255',
            'MaxRange-4000/ran305 with TEG-nodeN-6-timeT-1000.0-arriveN-255-scheduleN-255',
            'MaxRange-4000/ran306 with CGR-nodeN-6-timeT-1000.0-arriveN-306-scheduleN-306',
            'MaxRange-4000/ran306 with DirectForward-nodeN-6-timeT-1000.0-arriveN-198-scheduleN-306',
            'MaxRange-4000/ran306 with Heuristic-nodeN-6-timeT-1000.0-arriveN-197-scheduleN-306',
            'MaxRange-4000/ran306 with Spray-nodeN-6-timeT-1000.0-arriveN-290-scheduleN-306',
            'MaxRange-4000/ran306 with TEG-nodeN-6-timeT-1000.0-arriveN-306-scheduleN-306',
            'MaxRange-4000/ran307 with CGR-nodeN-6-timeT-1000.0-arriveN-357-scheduleN-357',
            'MaxRange-4000/ran307 with DirectForward-nodeN-6-timeT-1000.0-arriveN-238-scheduleN-357',
            'MaxRange-4000/ran307 with Heuristic-nodeN-6-timeT-1000.0-arriveN-236-scheduleN-357',
            'MaxRange-4000/ran307 with Spray-nodeN-6-timeT-1000.0-arriveN-334-scheduleN-357',
            'MaxRange-4000/ran307 with TEG-nodeN-6-timeT-1000.0-arriveN-357-scheduleN-357',
            'MaxRange-4000/ran308 with CGR-nodeN-6-timeT-1000.0-arriveN-408-scheduleN-408',
            'MaxRange-4000/ran308 with DirectForward-nodeN-6-timeT-1000.0-arriveN-294-scheduleN-408',
            'MaxRange-4000/ran308 with Heuristic-nodeN-6-timeT-1000.0-arriveN-297-scheduleN-408',
            'MaxRange-4000/ran308 with Spray-nodeN-6-timeT-1000.0-arriveN-379-scheduleN-408',
            'MaxRange-4000/ran308 with TEG-nodeN-6-timeT-1000.0-arriveN-407-scheduleN-408',
            'MaxRange-4000/ran309 with CGR-nodeN-6-timeT-1000.0-arriveN-459-scheduleN-459',
            'MaxRange-4000/ran309 with DirectForward-nodeN-6-timeT-1000.0-arriveN-335-scheduleN-459',
            'MaxRange-4000/ran309 with Heuristic-nodeN-6-timeT-1000.0-arriveN-334-scheduleN-459',
            'MaxRange-4000/ran309 with Spray-nodeN-6-timeT-1000.0-arriveN-432-scheduleN-459',
            'MaxRange-4000/ran309 with TEG-nodeN-6-timeT-1000.0-arriveN-459-scheduleN-459',
            'MaxRange-4000/ran310 with CGR-nodeN-6-timeT-1000.0-arriveN-510-scheduleN-510',
            'MaxRange-4000/ran310 with DirectForward-nodeN-6-timeT-1000.0-arriveN-358-scheduleN-510',
            'MaxRange-4000/ran310 with Heuristic-nodeN-6-timeT-1000.0-arriveN-357-scheduleN-510',
            'MaxRange-4000/ran310 with Spray-nodeN-6-timeT-1000.0-arriveN-472-scheduleN-510',
            'MaxRange-4000/ran310 with TEG-nodeN-6-timeT-1000.0-arriveN-510-scheduleN-510',
            'MaxRange-4000/ran311 with CGR-nodeN-6-timeT-1000.0-arriveN-561-scheduleN-561',
            'MaxRange-4000/ran311 with DirectForward-nodeN-6-timeT-1000.0-arriveN-388-scheduleN-561',
            'MaxRange-4000/ran311 with Heuristic-nodeN-6-timeT-1000.0-arriveN-388-scheduleN-561',
            'MaxRange-4000/ran311 with Spray-nodeN-6-timeT-1000.0-arriveN-511-scheduleN-561',
            'MaxRange-4000/ran311 with TEG-nodeN-6-timeT-1000.0-arriveN-561-scheduleN-561',
            'MaxRange-4000/ran312 with CGR-nodeN-6-timeT-1000.0-arriveN-612-scheduleN-612',
            'MaxRange-4000/ran312 with DirectForward-nodeN-6-timeT-1000.0-arriveN-429-scheduleN-612',
            'MaxRange-4000/ran312 with Heuristic-nodeN-6-timeT-1000.0-arriveN-429-scheduleN-612',
            'MaxRange-4000/ran312 with Spray-nodeN-6-timeT-1000.0-arriveN-561-scheduleN-612',
            'MaxRange-4000/ran312 with TEG-nodeN-6-timeT-1000.0-arriveN-612-scheduleN-612',
            'MaxRange-4000/ran313 with CGR-nodeN-6-timeT-1000.0-arriveN-663-scheduleN-663',
            'MaxRange-4000/ran313 with DirectForward-nodeN-6-timeT-1000.0-arriveN-467-scheduleN-663',
            'MaxRange-4000/ran313 with Heuristic-nodeN-6-timeT-1000.0-arriveN-467-scheduleN-663',
            'MaxRange-4000/ran313 with Spray-nodeN-6-timeT-1000.0-arriveN-612-scheduleN-663',
            'MaxRange-4000/ran313 with TEG-nodeN-6-timeT-1000.0-arriveN-663-scheduleN-663',
            'MaxRange-4000/ran314 with CGR-nodeN-6-timeT-1000.0-arriveN-714-scheduleN-714',
            'MaxRange-4000/ran314 with DirectForward-nodeN-6-timeT-1000.0-arriveN-484-scheduleN-714',
            'MaxRange-4000/ran314 with Heuristic-nodeN-6-timeT-1000.0-arriveN-485-scheduleN-714',
            'MaxRange-4000/ran314 with Spray-nodeN-6-timeT-1000.0-arriveN-660-scheduleN-714',
            'MaxRange-4000/ran314 with TEG-nodeN-6-timeT-1000.0-arriveN-714-scheduleN-714',
            ]
    
    ######
    # cycle
    #x_do_file_name_list = filter_strlist_by_name(x_05_file_name_list, "CGR") + filter_strlist_by_name(x_05_file_name_list, "TEG") + filter_strlist_by_name(x_05_file_name_list, "Heuristic") + filter_strlist_by_name(x_05_file_name_list, "DirectForward")

    # Random
    x_do_file_name_list = filter_strlist_by_name(x_04_file_name_list, "Spray") + filter_strlist_by_name(x_04_file_name_list, "DirectForward")

    # CGRQM
    #x_do_file_name_list = []
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
    print('====================== draw one senario by name =======================')
    #draw_one_senario_by_name('cycle with CGR-nodeN-11-timeT-802.0-arriveN-14-scheduleN-14', out_jsonob_list)

###
######################
###################################
x_current_path = os.getcwd()
print(x_current_path + "/box/jupyter/stuff folder/")
one_work_main(x_current_path + "/box/jupyter/stuff folder/")
