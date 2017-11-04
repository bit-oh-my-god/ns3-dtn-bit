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
handy_modify = True # diable me if you don't know what I'm doing TODO
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
    def __init__(self, name, tosend_list_ob, time_trace_map_ob):
        self.name = name
        self.tosend_list_ob = tosend_list_ob
        self.time_trace_map_ob = time_trace_map_ob
    def get_name(self) :
        return self.name
    def get_map(self) :
        return self.time_trace_map_ob
    def get_list(self) :
        return self.tosend_list_ob
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
    if float(arrived) != 0.0 : 
        average_delivery_delay = float(total_delivery_time) / float(arrived)
    else :
        average_delivery_delay = 802
    return [senario_name, routing_name, delivery_rate, average_delivery_delay]
    # return [senario_name, routing_name, delivery_rate, average_delivery_delay]
def draw_jsonob_list(p_out_jsonob_list) :
    abstracted_v_list = []
    for jsonob in p_out_jsonob_list :
        abstracted_v = abstract_value_from(jsonob)
        abstracted_v_list.append(abstracted_v)
    #----
    map_of_one_line_of_delivery_rate = {}
    map_of_one_line_of_average_delivery_delay = {}
    list_of_line_route_name_k = []
    list_of_senario_name_k = []
    for abv in abstracted_v_list :
        if abv[1] not in list_of_line_route_name_k :
            list_of_line_route_name_k.append(abv[1])
        if abv[0] not in list_of_senario_name_k :
            list_of_senario_name_k.append(abv[0])
    print('abstracted_v_list')
    print(abstracted_v_list)
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
    print('\nmap_of_one_line_of_delivery_rate')
    print(map_of_one_line_of_delivery_rate)
    print('\nmap_of_one_line_of_average_delivery_delay')
    print(map_of_one_line_of_average_delivery_delay)
    print('\nlist_of_line_route_name_k')
    print(list_of_line_route_name_k)
    print('\nlist_of_senario_name_k')
    print(list_of_senario_name_k)
    draw_graph_of(map_of_one_line_of_delivery_rate, list_of_line_route_name_k, list_of_senario_name_k, "delivery rate")
    draw_graph_of(map_of_one_line_of_average_delivery_delay, list_of_line_route_name_k, list_of_senario_name_k, "delivery delay")

#               'y' of one line index by route name; list of route name; list of 'x' axis name; flag is which figure you want
def draw_graph_of(map_of_one_line, list_of_line_route_name, list_of_senario_name, flag) :
    assert(len(map_of_one_line) == len(list_of_line_route_name))
    assert(map_of_one_line != None)
    assert(len(map_of_one_line[list_of_line_route_name[0]]) == len(list_of_senario_name))
    sena_num = len(list_of_senario_name)
    list_of_senario_name_kk = []
    if handy_modify :
        list_of_senario_name_kk = g_senario_name_list
        x = range(0, sena_num, 1)
    if flag == "delivery rate" :
        fig = plt.figure()
        #ax = fig.add_subplot(111, axes_class=AA.Axes, title='delivery_rate')
        ax = host_subplot(111, axes_class=AA.Axes)
        plt.title('senario name', y=1.03)
        #ax.set_xlabel('senario name', fontsize=18) 
        ax.set_ylabel('delivery rate', fontsize=16)
    elif (flag == "delivery delay") :
        fig = plt.figure()
        #ax = fig.add_subplot(111, axes_class=AA.Axes, title='delivery_delay')
        ax = host_subplot(111, axes_class=AA.Axes)
        plt.title('senario name', y=1.03)
        #ax.set_xlabel('senario name', fontsize=18)
        ax.set_ylabel('average delivery delay', fontsize=16)
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
# this is the main to run
def one_work_main(file_folder_path) :
    #=========
    #== settings
    # ./stuff folder/
                #'tx204 with TEG-nodeN-7-timeT-1000.0-arriveN-116-scheduleN-116',
    x_01_file_name_list = [
            'tx201 with CGR-nodeN-7-timeT-1000.0-arriveN-70-scheduleN-70',
            'tx201 with Heuristic-nodeN-7-timeT-1000.0-arriveN-54-scheduleN-70',
            'tx201 with Spray-nodeN-7-timeT-1000.0-arriveN-31-scheduleN-70',
            'tx201 with TEG-nodeN-7-timeT-1000.0-arriveN-70-scheduleN-70',
            'tx202 with CGR-nodeN-7-timeT-1000.0-arriveN-140-scheduleN-140',
            'tx202 with Heuristic-nodeN-7-timeT-1000.0-arriveN-103-scheduleN-140',
            'tx202 with Spray-nodeN-7-timeT-1000.0-arriveN-78-scheduleN-140',
            'tx202 with TEG-nodeN-7-timeT-1000.0-arriveN-140-scheduleN-140',
            'tx203 with CGR-nodeN-7-timeT-1000.0-arriveN-210-scheduleN-210',
            'tx203 with Heuristic-nodeN-7-timeT-1000.0-arriveN-158-scheduleN-210',
            'tx203 with Spray-nodeN-7-timeT-1000.0-arriveN-112-scheduleN-210',
            'tx203 with TEG-nodeN-7-timeT-1000.0-arriveN-210-scheduleN-210',
            'tx204 with CGR-nodeN-7-timeT-1000.0-arriveN-280-scheduleN-280',
            'tx204 with Heuristic-nodeN-7-timeT-1000.0-arriveN-219-scheduleN-280',
            'tx204 with Spray-nodeN-7-timeT-1000.0-arriveN-165-scheduleN-280',
            'tx204 with TEG-nodeN-7-timeT-1000.0-arriveN-280-scheduleN-280',
            'tx205 with CGR-nodeN-7-timeT-1000.0-arriveN-350-scheduleN-350',
            'tx205 with Heuristic-nodeN-7-timeT-1000.0-arriveN-271-scheduleN-350',
            'tx205 with Spray-nodeN-7-timeT-1000.0-arriveN-189-scheduleN-350',
            'tx205 with TEG-nodeN-7-timeT-1000.0-arriveN-350-scheduleN-350',
            'tx206 with CGR-nodeN-7-timeT-1000.0-arriveN-420-scheduleN-420',
            'tx206 with Heuristic-nodeN-7-timeT-1000.0-arriveN-325-scheduleN-420',
            'tx206 with Spray-nodeN-7-timeT-1000.0-arriveN-234-scheduleN-420',
            'tx206 with TEG-nodeN-7-timeT-1000.0-arriveN-420-scheduleN-420',
            'tx207 with CGR-nodeN-7-timeT-1000.0-arriveN-490-scheduleN-490',
            'tx207 with Heuristic-nodeN-7-timeT-1000.0-arriveN-387-scheduleN-490',
            'tx207 with Spray-nodeN-7-timeT-1000.0-arriveN-291-scheduleN-490',
            'tx207 with TEG-nodeN-7-timeT-1000.0-arriveN-490-scheduleN-490',
            'tx208 with CGR-nodeN-7-timeT-1000.0-arriveN-560-scheduleN-560',
            'tx208 with Heuristic-nodeN-7-timeT-1000.0-arriveN-460-scheduleN-560',
            'tx208 with Spray-nodeN-7-timeT-1000.0-arriveN-312-scheduleN-560',
            'tx208 with TEG-nodeN-7-timeT-1000.0-arriveN-560-scheduleN-560',
            'tx209 with CGR-nodeN-7-timeT-1000.0-arriveN-630-scheduleN-630',
            'tx209 with Heuristic-nodeN-7-timeT-1000.0-arriveN-503-scheduleN-630',
            'tx209 with Spray-nodeN-7-timeT-1000.0-arriveN-345-scheduleN-630',
            'tx209 with TEG-nodeN-7-timeT-1000.0-arriveN-630-scheduleN-630',
            'tx210 with CGR-nodeN-7-timeT-1000.0-arriveN-700-scheduleN-700',
            'tx210 with Heuristic-nodeN-7-timeT-1000.0-arriveN-550-scheduleN-700',
            'tx210 with Spray-nodeN-7-timeT-1000.0-arriveN-410-scheduleN-700',
            'tx210 with TEG-nodeN-7-timeT-1000.0-arriveN-700-scheduleN-700',
            'tx211 with CGR-nodeN-7-timeT-1000.0-arriveN-770-scheduleN-770',
            'tx211 with Heuristic-nodeN-7-timeT-1000.0-arriveN-627-scheduleN-770',
            'tx211 with Spray-nodeN-7-timeT-1000.0-arriveN-461-scheduleN-770',
            'tx211 with TEG-nodeN-7-timeT-1000.0-arriveN-770-scheduleN-770',
            'tx212 with CGR-nodeN-7-timeT-1000.0-arriveN-840-scheduleN-840',
            'tx212 with Heuristic-nodeN-7-timeT-1000.0-arriveN-632-scheduleN-840',
            'tx212 with Spray-nodeN-7-timeT-1000.0-arriveN-481-scheduleN-840',
            'tx212 with TEG-nodeN-7-timeT-1000.0-arriveN-840-scheduleN-840',
            'tx213 with CGR-nodeN-7-timeT-1000.0-arriveN-910-scheduleN-910',
            'tx213 with Heuristic-nodeN-7-timeT-1000.0-arriveN-702-scheduleN-910',
            'tx213 with Spray-nodeN-7-timeT-1000.0-arriveN-541-scheduleN-910',
            'tx213 with TEG-nodeN-7-timeT-1000.0-arriveN-910-scheduleN-910',
            'tx214 with CGR-nodeN-7-timeT-1000.0-arriveN-980-scheduleN-980',
            'tx214 with Heuristic-nodeN-7-timeT-1000.0-arriveN-754-scheduleN-980',
            'tx214 with Spray-nodeN-7-timeT-1000.0-arriveN-560-scheduleN-980',
            'tx214 with TEG-nodeN-7-timeT-1000.0-arriveN-980-scheduleN-980',
                     ]

    x_02_file_name_list = [
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

    x_03_file_name_list = [
            'ran301 with CGR-nodeN-6-timeT-1000.0-arriveN-51-scheduleN-51',
            'ran301 with Heuristic-nodeN-6-timeT-1000.0-arriveN-24-scheduleN-51',
            'ran301 with Spray-nodeN-6-timeT-1000.0-arriveN-24-scheduleN-51',
            'ran301 with TEG-nodeN-6-timeT-1000.0-arriveN-51-scheduleN-51',
            'ran302 with CGR-nodeN-6-timeT-1000.0-arriveN-102-scheduleN-102',
            'ran302 with Heuristic-nodeN-6-timeT-1000.0-arriveN-75-scheduleN-102',
            'ran302 with Spray-nodeN-6-timeT-1000.0-arriveN-77-scheduleN-102',
            'ran302 with TEG-nodeN-6-timeT-1000.0-arriveN-102-scheduleN-102',
            'ran303 with CGR-nodeN-6-timeT-1000.0-arriveN-153-scheduleN-153',
            'ran303 with Heuristic-nodeN-6-timeT-1000.0-arriveN-124-scheduleN-153',
            'ran303 with Spray-nodeN-6-timeT-1000.0-arriveN-98-scheduleN-153',
            'ran303 with TEG-nodeN-6-timeT-1000.0-arriveN-153-scheduleN-153',
            'ran304 with CGR-nodeN-6-timeT-1000.0-arriveN-204-scheduleN-204',
            'ran304 with Heuristic-nodeN-6-timeT-1000.0-arriveN-161-scheduleN-204',
            'ran304 with Spray-nodeN-6-timeT-1000.0-arriveN-153-scheduleN-204',
            'ran304 with TEG-nodeN-6-timeT-1000.0-arriveN-204-scheduleN-204',
            'ran305 with CGR-nodeN-6-timeT-1000.0-arriveN-255-scheduleN-255',
            'ran305 with Heuristic-nodeN-6-timeT-1000.0-arriveN-175-scheduleN-255',
            'ran305 with Spray-nodeN-6-timeT-1000.0-arriveN-199-scheduleN-255',
            'ran305 with TEG-nodeN-6-timeT-1000.0-arriveN-255-scheduleN-255',
            'ran306 with CGR-nodeN-6-timeT-1000.0-arriveN-306-scheduleN-306',
            'ran306 with Heuristic-nodeN-6-timeT-1000.0-arriveN-253-scheduleN-306',
            'ran306 with Spray-nodeN-6-timeT-1000.0-arriveN-237-scheduleN-306',
            'ran306 with TEG-nodeN-6-timeT-1000.0-arriveN-306-scheduleN-306',
            'ran307 with CGR-nodeN-6-timeT-1000.0-arriveN-357-scheduleN-357',
            'ran307 with Heuristic-nodeN-6-timeT-1000.0-arriveN-295-scheduleN-357',
            'ran307 with Spray-nodeN-6-timeT-1000.0-arriveN-212-scheduleN-357',
            'ran307 with TEG-nodeN-6-timeT-1000.0-arriveN-357-scheduleN-357',
            'ran308 with CGR-nodeN-6-timeT-1000.0-arriveN-408-scheduleN-408',
            'ran308 with Heuristic-nodeN-6-timeT-1000.0-arriveN-289-scheduleN-408',
            'ran308 with Spray-nodeN-6-timeT-1000.0-arriveN-347-scheduleN-408',
            'ran308 with TEG-nodeN-6-timeT-1000.0-arriveN-408-scheduleN-408',
            'ran309 with CGR-nodeN-6-timeT-1000.0-arriveN-459-scheduleN-459',
            'ran309 with Heuristic-nodeN-6-timeT-1000.0-arriveN-400-scheduleN-459',
            'ran309 with Spray-nodeN-6-timeT-1000.0-arriveN-355-scheduleN-459',
            'ran309 with TEG-nodeN-6-timeT-1000.0-arriveN-458-scheduleN-459',
            'ran310 with CGR-nodeN-6-timeT-1000.0-arriveN-510-scheduleN-510',
            'ran310 with Heuristic-nodeN-6-timeT-1000.0-arriveN-424-scheduleN-510',
            'ran310 with Spray-nodeN-6-timeT-1000.0-arriveN-334-scheduleN-510',
            'ran310 with TEG-nodeN-6-timeT-1000.0-arriveN-510-scheduleN-510',
            'ran311 with CGR-nodeN-6-timeT-1000.0-arriveN-561-scheduleN-561',
            'ran311 with Heuristic-nodeN-6-timeT-1000.0-arriveN-350-scheduleN-561',
            'ran311 with Spray-nodeN-6-timeT-1000.0-arriveN-402-scheduleN-561',
            'ran311 with TEG-nodeN-6-timeT-1000.0-arriveN-561-scheduleN-561',
            'ran312 with CGR-nodeN-6-timeT-1000.0-arriveN-612-scheduleN-612',
            'ran312 with Heuristic-nodeN-6-timeT-1000.0-arriveN-443-scheduleN-612',
            'ran312 with Spray-nodeN-6-timeT-1000.0-arriveN-422-scheduleN-612',
            'ran312 with TEG-nodeN-6-timeT-1000.0-arriveN-612-scheduleN-612',
            'ran313 with CGR-nodeN-6-timeT-1000.0-arriveN-663-scheduleN-663',
            'ran313 with Heuristic-nodeN-6-timeT-1000.0-arriveN-489-scheduleN-663',
            'ran313 with Spray-nodeN-6-timeT-1000.0-arriveN-540-scheduleN-663',
            'ran313 with TEG-nodeN-6-timeT-1000.0-arriveN-663-scheduleN-663',
            'ran314 with CGR-nodeN-6-timeT-1000.0-arriveN-714-scheduleN-714',
            'ran314 with Heuristic-nodeN-6-timeT-1000.0-arriveN-605-scheduleN-714',
            'ran314 with Spray-nodeN-6-timeT-1000.0-arriveN-496-scheduleN-714',
            'ran314 with TEG-nodeN-6-timeT-1000.0-arriveN-713-scheduleN-714',
            ]

    
    file_name_list = []
    #=====
    #==  change this TODO
    for filename in x_03_file_name_list :
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
