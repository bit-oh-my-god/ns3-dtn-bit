#!/home/tara/anaconda3/bin/python

import re
import sys
import inspect
import os
from math import sqrt
import numpy as np
from scipy import integrate
import weakref
from matplotlib import pyplot as plt
import matplotlib as mpl
from matplotlib import cm
import pandas as pd
from mpl_toolkits.mplot3d import Axes3D
import jsonpickle
from matplotlib.colors import cnames
from matplotlib import animation
from matplotlib.lines import Line2D
import matplotlib.animation as animation
#======== definition of utilfuckyou
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
def nums(s):
    try:
        return int(s)
    except ValueError:
        return float(s)
class CGRXMIT(object) :
    def __init__(self, contact_start_time_, contact_end_time_, node_id_of_from_, node_id_of_to_, data_transmission_rate_, str_):
        self.m_contact_start_time_ = contact_start_time_
        self.m_contact_end_time_ =contact_end_time_
        self.m_node_id_of_from_=node_id_of_from_
        self.m_node_id_of_to_=node_id_of_to_
        self.m_data_transmission_rate_=data_transmission_rate_
        self.m_str_ =str_
        nothing = 1
    def get_str_(self):
        return self.m_str_
    def get_cst(self):
        return self.m_contact_start_time_
    def get_cet(self):
        return self.m_contact_end_time_
    def get_nf(self):
        return self.m_node_id_of_from_
    def get_nt(self):
        return self.m_node_id_of_to_
    def get_dr(self):
        return self.m_data_transmission_rate_
#======== definition of utilfuckyou

#======= definition of x_parsemainfunc
def x_parsemainfunc(
    logfilefullname = get_path_suffix_of('ns3-dtn-bit') + "/box/dtn_simulation_result/dtnrunninglog.txt",
    ) :
    simulation_result_file = open(logfilefullname, "r")
    lines = simulation_result_file.readlines();
    simulation_result_file.close()
    contactpool = []
    for line in lines :
        #cgrxmit==> contact_start_time_ =970;contact_end_time_=993;node_id_of_from_=6;node_id_of_to_=0;data_transmission_rate_=80000
        r1 = re.compile(r'cgrxmit==>\scontact_start_time_\s=(\d+\.*\d*);contact_end_time_=(\d+\.*\d*);node_id_of_from_=(\d+\.*\d*);node_id_of_to_=(\d+\.*\d*);data_transmission_rate_=(\d+\.*\d*)', re.VERBOSE)
        contactinfo = r1.search(line)
        if contactinfo:
            cst = float(nums(contactinfo.group(1)))
            cet = float(nums(contactinfo.group(2)))
            nf = int(nums(contactinfo.group(3)))
            nt = int(nums(contactinfo.group(4)))
            dr = float(nums(contactinfo.group(5)))
            onecontact = CGRXMIT(
            contact_start_time_ = cst, 
            contact_end_time_=cet,
            node_id_of_from_ = nf,
            node_id_of_to_=nt,
            data_transmission_rate_ = dr,
            str_=contactinfo.group(0),
            )
            contactpool.append(onecontact)
        else :
            nothing = 1
    return contactpool
#======= end definition of x_parsemainfunc
#======= definition of x_graphfunc
def x_graphfunc(contactpool):
    def x_drawoneline(ax,linestruct):
        nothing =1
        print("{0},{1},{2},{3}".format(ck.get_cst(), ck.get_cet(), ck.get_nf(), ck.get_nt))
        ax.plot([ck.get_cst(), ck.get_cet()], [ck.get_nf(), ck.get_nt()])
    # def x_drawoneline(ax,linestruct):
    fig = plt.figure()
    subflag = 111 
    ax = plt.subplot(subflag)
    ax.set_xlabel("time", fontsize=10)
    ax.set_ylabel("nodeid", fontsize=10)
    contactnodepool = []
    maxtimescale = 0
    mintimescale = 0
    for ck in contactpool :
        x_drawoneline(ax, ck)
        nf = ck.get_nf()
        nt = ck.get_nt()
        cet = ck.get_cet()
        if cet > maxtimescale :
            maxtimescale = cet
        if nf not in contactnodepool:
            contactnodepool.append(nf)
        if nt not in contactnodepool:
            contactnodepool.append(nt)
    def x_drawnodeline(ax,contactnodepool, timescale):
        for n in contactnodepool:
            ax.plot(timescale, [n,n], "k--")
    # end def x_drawnodeline(ax, tinescale):
    x_drawnodeline(ax,contactnodepool, [mintimescale, maxtimescale])
    plt.show()
#======= end definition of x_graphfunc
#======= definition of x_mainfunc
def x_mainfunc():
    contactpool = x_parsemainfunc()
    print(contactpool)
    x_graphfunc(contactpool)
#======= end definition of x_mainfunc
x_mainfunc()