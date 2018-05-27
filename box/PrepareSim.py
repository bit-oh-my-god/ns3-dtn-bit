#!/home/tara/anaconda3/bin/python
#------------------------------
# run this to convert from /box/current_trace/current_trace.tcl to /box/current_trace/teg.txt
#------------------------------
import re
import sys
import os
import inspect
from math import sqrt
import numpy as np
from scipy import integrate
from matplotlib import pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.patches as mpatches
from matplotlib.colors import cnames
from matplotlib import animation
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
# debug macro
def debug(value):
        calling_frame_record = inspect.stack()[1]
        frame = inspect.getframeinfo(calling_frame_record[0])
        m = re.search( "dd\((.+)\)", frame.code_context[0])
        if m:
            print("'{0}' = '{1}'" .format(m.group(1), value))
#====
# define TimeState
class TimeState :
    def __init__(self, time, node, dest, velocity) :
        self.time_ = float(time)
        self.node_ = int(node)
        self.dest_ = dest
        vv = float(velocity)
        #while vv > float(maxspeed) :
        #    vv = round(vv / 2, 5)
        if vv > float(maxspeed) :
            print('Warn: trace file speed is too fast. You must to round it down. Or it would cause deadly precise loose')
            sys.exit()
        self.velocity_ = vv
    def __str__(self):
        return "<TimeState: time_ = %f, node = %d, dest = %s, velocity = %f>" % (self.time_, self.node_, self.dest_, self.velocity_)
# end-define
#====
# define Position
class Position :
    def __init__(self, x, y, z) :
        self.x_ = float(x)
        self.y_ = float(y)
        self.z_ = float(z)
    def __str__(self) :
        return "<Position: x_ = %f, y_ = %f, z = %f>" % (self.x_, self.y_, self.z_)
    def tolist(self) :
        return [float(self.x_), float(self.y_), float(self.z_)]
def CreatePosition() :
    tmp = Position(-1, -1, -1)
    return tmp
def CreateNodeState() :
    empty_node_state = {'origin_pos' : CreatePosition(), 'time_changes' : []}
    return empty_node_state
# end-define
# define PTVGenarator(position time variable genarator)
# ==============================
# =======================
# == Important! change the method pos_t algebra-ed with, if you want.
# ====================
class PTVGenarator :
    def __init__(self, orip, ts) :
        self.ts_ = ts
        self.curp_ = orip.tolist()
        self.curt_ = int(0)
        # next ts index
        self.tsi_ = 0
        self.vx_ = 0
        self.vy_ = 0
        self.vz_ = 0
        #self.getpos(0)
    def get_pos_detail(self, t) :
        if (self.tsi_ == len(self.ts_)) or (self.curt_ < int(self.ts_[self.tsi_].time_)) :
            # do nothing
            nomeaning = "if tsi_ == len(self.ts_), we thought it would go as before"
        else :
            # modify
            ts_var = self.ts_[self.tsi_]
            dest_list = ts_var.dest_.tolist()
            speed = round(ts_var.velocity_, 4)
            dx = round(dest_list[0] - self.curp_[0], 2)
            dy = round(dest_list[1] - self.curp_[1], 2)
            dz = round(dest_list[2] - self.curp_[2], 2)
            deslen = sqrt(dx * dx + dy * dy + dz * dz) + 0.01
            self.vx_ = round(speed * (float(dx) / deslen), 4)
            self.vy_ = round(speed * (float(dy) / deslen), 4)
            self.vz_ = round(speed * (float(dz) / deslen), 4)
            self.tsi_ += 1
        # move
        self.curt_ = t
        self.curp_ = [round(self.vx_ + self.curp_[0], 4),
                      round(self.vy_ + self.curp_[1], 4),
                      round(self.vz_ + self.curp_[2], 4)]
        return self.curp_
    # t would be 0,1.....599
    def get_pos(self, t) :
        if t < T_max :
            return self.get_pos_detail(t)
        else :
            print("t is bigger than T_max")
# end-define PTVGenarator
#=============
# end of all define
#================================
#======================================
#================================================
# init some trace info
x_anim_interval = 50            #change me !!!!!!!
ax_xlimits = [2000, 10000]       #change me !!!!!!!
ax_ylimits = [2000, 10000]       #change me !!!!!!!
ax_zlimits = [2000, 10000]       #change me !!!!!!!
maxspeed = int(500)
T_max = 1000                #change me !!!!!!!
teg_slice_n = 1000          #change me !!!!!!!
Frames_n = T_max
global global_maxwifirange 
global_maxwifirange = 2000  #change me !!!!!!!
targettracefilepath = get_path_suffix_of('ns3-dtn-bit') + "/box/current_trace/current_trace.tcl"
targettegfilepath = get_path_suffix_of('ns3-dtn-bit') + '/box/current_trace/teg.txt'
#====
# read the trace file
trace_file = open(targettracefilepath, "r")
lines = trace_file.readlines();
assert(len(lines) > 1)
trace_file.close()

nodes_info = []
# parse every line
for line in lines :
    #print(line)
    match_sharp_then_skip = re.findall(r'\#',line)
    if match_sharp_then_skip :
        continue
    #node_
    match_object = re.match(r'\$node_\((\d+\.*\d*)\) set ([XYZ])_ (\d+\.*\d*)', line)
    if match_object :
        node_number = int(match_object.group(1))
        if match_object.group(2) == 'X' :
            nodes_info.append(CreateNodeState())
            nodes_info[node_number]['origin_pos'].x_ = match_object.group(3)
        elif match_object.group(2) == 'Y' :
            nodes_info[node_number]['origin_pos'].y_ = match_object.group(3)
        elif match_object.group(2) == 'Z' :
            nodes_info[node_number]['origin_pos'].z_ = match_object.group(3)
    else :
        #ns_
        match_ob = re.match\
        (r'\$ns_ at (\d+\.*\d*) \"\$node_\((\d+\.*\d*)\) setdest (\d+\.*\d*) (\d+\.*\d*) (\d+\.*\d*) (\d+\.*\d*)\"',\
         line)
        if match_ob == None:
            print("can't match any! null!")
            print(line)
            sys.exit()
        node_number = int(match_ob.group(2))
        # tmp is reference semantic, so this work
        tmp = nodes_info[node_number]['time_changes']
        
        tmp_ts = TimeState(match_ob.group(1),
                           match_ob.group(2),
                           Position(match_ob.group(3), match_ob.group(4), match_ob.group(5)),
                           match_ob.group(6))
        tmp.append(tmp_ts)
# end-parse
#"""
##====
## define PrintNodeInfo
#def PrintNodeInfo(node_info) :
#    for keys,values in node_info.items() :
#        print(keys)
#        if isinstance(values, list) :
#            for v in values :
#                print(v)
#        elif isinstance(values, Position) :
#            print(values)
## end-define
#
## print nodes_info
#for node_info in nodes_info :
#    PrintNodeInfo(node_info)
## end-print
#"""

#============================================ dividing line ===============================================
#====
# calculate draw data : chech this link : 
# https://jakevdp.github.io/blog/2013/02/16/animating-the-lorentz-system-in-3d/
N_trajectories = len(nodes_info)
print("N_trajectories = %d" % int(N_trajectories))
# this is a array for original position, should be (N_trajectories, 3) shape
pos_0_l = []
for i in range(0, N_trajectories)  :
    pos_0_l.append(nodes_info[i]['origin_pos'].tolist())
pos_0 = np.array(pos_0_l)
# this should be a vector, describe the simulate time, like this np.linspace(0, 4, 1000)
time_vec = np.linspace(0, 1, T_max)



# this is a expression for position on time_vec, should be the (N_trajectories, time_vec.max, 3) shape
x_pt = []
for i in range(0, N_trajectories, 1) :
    x_pt.append([])
    ptvg_i = PTVGenarator(nodes_info[i]['origin_pos'], nodes_info[i]['time_changes'])
    # note, because time in nodes_info is double and double is not a friendly data type for visualization,
    # so we would do this with the int data type which would cause some precise loose
    for t in range(0, T_max, 1) :
        x_pt[i].append([])
        if t != 0 :
            # calculate time-changing pos
            x_pt[i][t] = ptvg_i.get_pos(t)
        else :
            # origin pos
            tx = pos_0[i][0]
            ty = pos_0[i][1]
            tz = pos_0[i][2]
            x_pt[i][t] = [round(tx, 2), round(ty, 2), round(tz, 2)]
            if i == 3 :
                print(x_pt[3][0])
pos_t = np.array(x_pt)
print(np.shape(pos_t))
print("good ending!!")
# end-calculate
#====
# output the position of nodes. 'x_pt'.
# 

with open(targettegfilepath, 'w') as teg_f:
    for i in range(0, N_trajectories, 1) :
        for t in range(0, T_max, int(T_max / teg_slice_n)) :
            teg_f.write('node {0} time {1} pos {2} {3} {4}\n'.
                        format(i, t, int(x_pt[i][t][0]), int(x_pt[i][t][1]), int(x_pt[i][t][2])))
#====
# make graph
# Set up figure & 3D axis for animation
fig = plt.figure()
#Add an axes at position rect [left, bottom, width, height]
#where all quantities are in fractions of figure width and height. 
ax = fig.add_axes([0, 0, 1, 1], projection='3d')
ax.axis('on')
# choose a different color for each trajectory
#colors = plt.cm.jet(np.linspace(0, 1, N_trajectories))
colors = plt.cm.ScalarMappable(cmap=plt.cm.get_cmap('jet')).to_rgba(x=np.linspace(0, 1, N_trajectories))

# set up lines and points
lines = sum([ax.plot([], [], [], '-', c=c)
             for c in colors], [])
pts = sum([ax.plot([], [], [], 'o', c=c)
           for c in colors], [])
# show legend
patchs = []
legendcount = 0
for c in colors :
    patchs.append(mpatches.Patch(color=c, label='node-{0}'.format(legendcount)))
    legendcount += 1
oldlegend = plt.legend(handles=patchs)
# prepare the axes limits
ax.set_xlim(ax_xlimits)
ax.set_ylim(ax_ylimits)
ax.set_zlim(ax_zlimits)
# set point-of-view: specified by (altitude degrees, azimuth degrees)
#‘elev’ stores the elevation angle in the z plane. ‘azim’ stores the azimuth angle in the x,y plane.
ax.view_init(60, 20)
# initialization function: plot the background of each frame
def init():
    for line, pt in zip(lines, pts):
        line.set_data([], [])
        line.set_3d_properties([])

        pt.set_data([], [])
        pt.set_3d_properties([])
    return lines + pts
# ====== begin of CONTACTLINEHOLDER
class CONTACTLINEHOLDER(object):
    def __init__(self, ax):
        nothing =1
        self.m_ax = ax
        self.clmap = {repr([-1,-2]): None}
        #print(self.clmap)
    def updateoneline(self, thatline, twonodeid=[1,2]):
        restr = repr(twonodeid)
        #print("in updateoneline, repr={0}".format(restr))
        assert(len(twonodeid) == 2)
        assert(twonodeid[0] != twonodeid[1])
        twonodeid.sort()
        if restr in self.clmap:
            self.removeoneline(self.clmap[restr])
        self.clmap[restr] = thatline
        if thatline == None:
            try:
                del(self.clmap[restr])
            except KeyError:
                raise KeyError
    def removeoneline(self,thatline):
        #print("in removeoneline, thatline={0}".format(thatline))
        thatline[0].remove()
        del(thatline[0])
        nothing = 1
    def draw_one_line_as(self, linepoint1=None, linepoint2=None):
        nothing = 1
        xseq =  [linepoint1[0], linepoint2[0]]
        yseq =  [linepoint1[1], linepoint2[1]]
        zseq =  [linepoint1[2], linepoint2[2]]
        line = self.m_ax.plot(xseq, yseq, zseq,"k:",linewidth=1)
        return line
    def draw_contact_line_between(self, x_pt, i) :
        print("============ frame = {0} ------".format(i))
        def dist(p1, p2):
            dx = p1[0] - p2[0]
            dy = p1[1] - p2[1]
            dz = p1[2] - p2[2]
            dis = round(sqrt(float(dx * dx + dy * dy + dz * dz))+ 0.01, 4)
            return dis
        pointpool = {}
        for j in range(0, N_trajectories, 1) :
            position = x_pt[j][i]
            #print(position)
            pointpool[j] = position
        for k in pointpool :
            for j in pointpool:
                if (k == j) :
                    continue
                distance = dist(pointpool[k], pointpool[j])
                global global_maxwifirange
                if distance < global_maxwifirange: 
                    thatline = self.draw_one_line_as(pointpool[k],pointpool[j])
                    self.updateoneline(thatline, [k,j])
                    #print(" has contact bet {0}, {1}; dis = {2}".format(k, j, distance))
                else :
                    self.updateoneline(None, [k,j])
                    #print(" no contact bet {0}, {1}; dis = {2}".format(k, j, distance))
        #print(self.clmap)
# ======= end of CONTACTLINEHOLDER
global g_contactlineholder
g_contactlineholder = CONTACTLINEHOLDER(ax)
# animation function.  This will be called sequentially with the frame number
def animate(i):
    # we'll step two time-steps per frame.  This leads to nice results.
    #i = (2 * i) % pos_t.shape[1]
    global g_contactlineholder
    for line, pt, pos_ti in zip(lines, pts, pos_t):
        #.T means transpose of an array
        x, y, z = pos_ti[:i].T
        line.set_data(x, y)
        line.set_3d_properties(z)

        pt.set_data(x[-1:], y[-1:])
        pt.set_3d_properties(z[-1:])
    g_contactlineholder.draw_contact_line_between(x_pt, i)
    # change angle per frame
    ax.view_init(40, 0.2 * i)
    # don't change angle per frame
    #ax.view_init(40, 15)
    #ax.set_title('time- {0}; metric m/s; node-{1}-pox-{2}-{3}-{4}'.
    #         format(i, 1, pos_t[1][i][0], pos_t[1][i][1], pos_t[1][i][2]))
    fig.canvas.draw()
    # render legend
    oldlegend = plt.legend()
    return lines + pts 
# instantiate the animator.
anim = animation.FuncAnimation(fig, animate, init_func=init,
                               frames=Frames_n, interval=x_anim_interval, blit=True)
# Save as mp4. This requires mplayer or ffmpeg to be installed
## anim.save('lorentz_attractor.mp4', fps=15, extra_args=['-vcodec', 'libx264'])
plt.show()

#= TODO print colors and node id
# end-make
print("Good Ending!!")