#!/home/tara/anaconda3/bin/python
#------------------------------
# run this to generate /box/current_trace/current_trace_generate_one
#------------------------------
import re
import sys
import random
import math
import inspect
import os
from math import sqrt
import numpy as np
from scipy import integrate
from matplotlib import pyplot as plt
import matplotlib as mpl
from matplotlib import cm
import pandas as pd
from mpl_toolkits.mplot3d import Axes3D
import jsonpickle
from matplotlib.colors import cnames
from matplotlib import animation

#========================================
# this is a file used to generate current_trace_file
#=========================================
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
#========================
# @brief refine a vector to length 1
def refine_mode_one(v) :
    dis = sqrt(dist_of(v, [0.0,0.0,0.0]))
    return [v[0] / dis, v[1] / dis, v[2] / dis]
#=========================
# @brief this func would generate the corner of a 'n' edge polygon with core of 'core', long axis(length in 'X' axis), short axis, vertical_line, angle between long axis and xz surface
# @return a vector of vertex of polygon
def generate_points_of_polygon_with_core_size_slope(n, core, size_long, size_width, vertical_line, angle_of_long_axis_and_xz_surface, is_clock_wise, root_1_or_2, axis_x) :
    #assert n
    #assert core
    #assert size_long
    #assert size_width
    #assert vertical_line
    print("nothing")
    def solve_it_on_plain(n, size_long, size_width) :
        assert(n > 6)
        angle = []
        result = []
        pi2 = math.pi / 2.0
        for i in range(0, n, 1) :
            one = 2 * math.pi / float(n)
            angle.append(i * one)
        print(angle)
        for angle_i in angle :
            if (abs(angle_i - pi2) < g_precise or abs(angle_i - (pi2 * 3)) < g_precise) :
                if (angle_i - pi2 < g_precise) :
                    result.append([0, size_width, 0])
                else :
                    result.append([0, (-1.0) * size_width, 0])
            else :
                # solve the equation x^2 / a^2 + y^2 / b^2 = 1
                tan = math.tan(angle_i)
                factor1 = (1.0 / pow(size_long, 2)) + (pow(tan, 2) / pow(size_width, 2))
                x = 0
                if (angle_i < pi2 or angle_i > pi2 * 3) :
                    x = sqrt(1.0 / factor1)
                else :
                    x = -1.0 * sqrt(1.0 / factor1)
                y = x * tan
                result.append([x, y, 0])
        return result
    def get_changing_vector_x(vertical_line, angle_of_long_axis_and_xz_surface, root_1_or_2) :
        #assert
        if (angle_of_long_axis_and_xz_surface == math.pi / 4.0 or angle_of_long_axis_and_xz_surface == math.pi / 4.0 * 3) :
            print("Error:bad01")
            sys.exit()
        a = 0
        b = 0
        c = 0
        # solve these three equation
        # 1. a^2 + b^2 + c^2 = 1
        # 2. b / sqrt(a^2 + c^2) = tan(angle)
        # 3. a * vertical_line[0] + b * vertical_line[1] + c * vertical_line[2] = 0
        b = sqrt(1.0 - 1.0 / (1.0 + math.tan(angle_of_long_axis_and_xz_surface))) # the positive root
        tmp01 = 0
        if (math.tan(angle_of_long_axis_and_xz_surface) > 0) :
            tmp01 = 1
        else :
            tmp01 = -1
        b = b * tmp01 # the real root, note the sign in second equation
        factor1 = (b * b) / (math.tan(angle_of_long_axis_and_xz_surface) * math.tan(angle_of_long_axis_and_xz_surface)) # f1 = b^2 / tan(anxz)^2
        factor2 = b * vertical_line[1]  # f2 = b * vertical_line[1]
        factor3 = ((vertical_line[2] * vertical_line[2]) / (vertical_line[0] * vertical_line[0])) + 1 # f3 = v[2]^2 / v[0]^2 + 1
        factor4 = (2 * factor2 * vertical_line[2]) / (vertical_line[0] * vertical_line[0]) # f4 = 2 * f2 * v[2] / v[0]^2
        factor5 = ((factor2 * factor2) / (vertical_line[0] * vertical_line[0])) - factor1 # f5 = f2^2 / v[0]^2 - f1
        factor6 = ((factor4 * factor4) - (4.0 * factor3 * factor5)) / (4.0 * factor3 * factor3) # solve this : f3 * c^2 + f4 * c + f5 = 0
        c1 = sqrt(factor6) - (factor4 / (2.0 * factor3))
        c2 = ((-1.0) * sqrt(factor6)) - (factor4 / (2 * factor3))
        a1 = (b * vertical_line[1] + c1 * vertical_line[2]) / ((-1.0) * vertical_line[0])
        a2 = (b * vertical_line[1] + c2 * vertical_line[2]) / ((-1.0) * vertical_line[0])
        if (root_1_or_2 == 1) :
            #assert(a1 * a1 + b * b + c1 * c1 == 1) # assert would fail, it's a bug? TODO
            return [a1, b, c1]
        elif (root_1_or_2 == 2) :
            #assert(a1 * a1 + b * b + c1 * c1 == 1) # assert would fail, it's a bug? TODO
            return [a2, b, c2]
        else :
            print("specify which root you want, root_1_or_2 can only have two value :'1, 2'")
            sys.exit()
    def get_changing_vector_y(z, x) :
        print("nothing")
        return np.cross(z, x)
    def map_one_to_slope(point, core, changing_vec) :
        assert(len(point) == 3)
        assert(len(core) == 3)
        assert(len(changing_vec) == 3)
        cv = np.matrix(changing_vec)
        p = np.matrix(point)
        mr = p * cv
        mrr = mr.tolist()
        #print(mrr)
        assert(len(mrr[0]) == 3)
        return [mrr[0][0] + core[0], mrr[0][1] + core[1], mrr[0][2] + core[2]]
    # first get the solution on plain, then map it to slope surface
    plain = solve_it_on_plain(n, size_long, size_width)
    print("plain:")
    print(plain)
    assert(len(plain[0]) == 3)
    cvx = []
    if (axis_x == []) :
        cvx = get_changing_vector_x(vertical_line, angle_of_long_axis_and_xz_surface, root_1_or_2)
    else :
        cvx = axis_x
        assert((cvx[0] * vertical_line[0] + cvx[1] * vertical_line[1] + cvx[2] * vertical_line[2]) == 0)
    cvz = refine_mode_one(vertical_line)
    cvx = refine_mode_one(cvx)
    cvy = get_changing_vector_y(cvz, cvx)
    cvall = [cvx, cvy, cvz]
    ret = list(map(lambda x : map_one_to_slope(x, core, cvall), plain)) 
    print("changing_vec")
    print(cvall)
    print("slope:")
    print(ret)
    if (is_clock_wise) :
        if (len(ret) > 0) :
            return ret
        else :
            sys.exit()
    else :
        print("not implement, yet, so lazy.")
        sys.exit()
#========================
# string pattern
def ori_pattern(vec, n) :
    #$node_(0) set X_ 2000.34380287508
    #$node_(0) set Y_ 2096.060359323184
    #$node_(0) set Z_ 2147.98298672775
    try :
        assert(vec[0] > 0)
        assert(vec[1] > 0)
        assert(vec[2] > 0)
    except :
        print("Error: pos must be positive, or later process (translate into teg.txt )would fail.")
        sys.exit()
    return "$node_(" + str(n) + ") set X_ " + str(vec[0]) + "\n"\
            "$node_(" + str(n) + ") set Y_ " + str(vec[1]) + "\n"\
            "$node_(" + str(n) + ") set Z_ " + str(vec[2]) + "\n"
def move_pattern(vec, n, t, s) :
    #$ns_ at 20.0 "$node_(6) setdest 6671.810294658997 6693.12828510459 5891.62337050674 45.63736346097109"
    return "$ns_ at " + str(t) + " \"" + "$node_("+ str(n) + ") setdest " +\
            str(vec[0]) + " " +\
            str(vec[1]) + " " +\
            str(vec[2]) + " " +\
            str(s) + "\"\n"
#=======================
# math
def dist_of(vec1, vec2) :
    return sqrt(pow(vec1[0] - vec2[0], 2) + pow(vec1[1] - vec2[1], 2) + pow(vec1[2] - vec2[2], 2))

#========================
# @brief this func would write a sequence of trace into file with vector of points and speed and time, start at vector_of_points[m] as nodeid n
# @input vector_of_points is 'list of points you plan'
# @input speed_vec is speed you plan
def write_trace_into_file(vector_of_points, speed_vec, alltime, m, n) :
    #assert
    print("nothing")
    #assert(len(vector_of_points) > 3)
    cur = (m) % len(vector_of_points)
    cur_t = 0
    speed_index = 0
    g_trace_file.write(ori_pattern(vector_of_points[cur], n))
    while cur_t < alltime :
        to = (cur + 1) % len(vector_of_points)
        speed = speed_vec[speed_index % len(speed_vec)]
        g_trace_file.write(move_pattern(vector_of_points[to], n, cur_t, speed))
        assert(cur != to)
        add_v = float(dist_of(vector_of_points[cur], vector_of_points[to])) / float(speed)
        if (add_v > 0) :
            cur = to
            cur_t += add_v
        else :
            print("Error:add_v <= 0")
            print(vector_of_points)
            print(add_v)
            print(dist_of(vector_of_points[cur], vector_of_points[to]))
            sys.exit()
        speed_index += 1
    print("good, write into file")

#==============
# @brief print in figure 2d
def print_in_2d(vector_of_points) :
    xx = []
    yy = []
    for v in vector_of_points :
        xx.append(v[0])
        yy.append(v[1])
    plt.plot(xx, yy, 'ro')
    #plt.axis([0, 6, 0, 20])
    plt.show()

#==============
# @brief print in figure 3d
def print_in_3d(vector_of_points) :
    xx = []
    yy = []
    zz = []
    for v in vector_of_points :
        xx.append(v[0])
        yy.append(v[1])
        zz.append(v[2])
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    ax.scatter(xx, yy, zz)
    plt.show()

#==============
# @brief print in figure 3d group
def print_in_3d_group(v_of_v_p) :
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    for vector_of_points in v_of_v_p :
        xx = []
        yy = []
        zz = []
        for v in vector_of_points :
            xx.append(v[0])
            yy.append(v[1])
            zz.append(v[2])
        ax.scatter(xx, yy, zz)
    plt.show()
#==========
# @brief
def generate_random_speed_vec(speed_range, n) :
    assert(len(speed_range)==2 and speed_range[1] > speed_range[0])
    speed_vec = []
    for i in range(0, n, 1) :
        speed_vec.append(random.uniform(speed_range[0], speed_range[1]))
    return speed_vec
#=============
# @brief 
def generate_points_of_random_in_range_and_n(argumap) :
    x_range = argumap["range"]["x_range"]
    y_range = argumap["range"]["y_range"]
    z_range = argumap["range"]["z_range"]
    n_of_points = argumap["n_of_points"]
    assert(len(x_range)==2 and x_range[1] > x_range[0])
    assert(len(y_range)==2 and y_range[1] > y_range[0])
    assert(len(z_range)==2 and z_range[1] > z_range[0])
    points_vec = []
    for i in range(0, n_of_points, 1) :
        points_vec.append([
            random.uniform(x_range[0], x_range[1]), 
            random.uniform(y_range[0], y_range[1]), 
            random.uniform(z_range[0], z_range[1]), 
            ])
    print("n_of_points is{0}, len = {1}".format(n_of_points, len(points_vec)))
    assert(n_of_points == len(points_vec)) 
    return points_vec
#===================
# @brief a main function
def test01_functon() :
    print("python version:" + sys.version)
    global g_precise, g_trace_file 
    g_precise = 0.000001
    g_trace_file = open(get_path_suffix_of('nd3-dtn-bit') + "/box/current_trace/current_trace_generate_one", "w")
    #==
    #
    assert(dist_of([1000,1000,1000],[2000,1000,1000]) == 1000)
    node1_trace = generate_points_of_polygon_with_core_size_slope(24, [8000,8000,8000], 1500.0, 2800.0, [0.1,0.2,0.9], (math.pi / 6.0), True, 1, [])
    node2_trace = generate_points_of_polygon_with_core_size_slope(24, [8000,8000,8500], 1500.0, 2800.0, [0.1,0.2,0.9], (math.pi / 6.0), True, 1, [])
    node3_trace = generate_points_of_polygon_with_core_size_slope(24, [8000,8000,7500], 1500.0, 2800.0, [0.1,0.2,0.9], (math.pi / 6.0), True, 1, [])
    node4_trace = generate_points_of_polygon_with_core_size_slope(24, [10000,10000,6500], 310.0, 180.0, [0.1,2,0.2], (math.pi / 12.0), True, 1, [0, -1, 10])
    node5_trace = generate_points_of_polygon_with_core_size_slope(24, [6000,6000,9500], 310.0, 180.0, [0.1,2,0.2], (math.pi / 12.0), True, 1, [0, -1, 10])
    node6_trace = [[6000, 6000, 11000], [6000,6000,10200]]
    node7_trace = [[10000, 10000, 5400], [10000,10000,6000]]
    print_in_3d_group([node1_trace,node2_trace,node3_trace,node4_trace, node5_trace, node6_trace, node7_trace])
    write_trace_into_file(node1_trace, [100], 1000, 0, 0)
    write_trace_into_file(node2_trace, [30], 1000, 9, 1)
    write_trace_into_file(node3_trace, [30], 1000, 15, 2)
    write_trace_into_file(node4_trace, [20], 1000, 5, 3)
    write_trace_into_file(node5_trace, [20], 1000, 5, 4)
    write_trace_into_file(node6_trace, [10], 1000, 5, 5)
    write_trace_into_file(node7_trace, [10], 1000, 5, 6)
    g_trace_file.close()

#====================
# @brief another main function
def test02_functon() :
    print("python version:" + sys.version)
    global g_precise, g_trace_file 
    g_precise = 0.000001 
    g_trace_file = open(get_path_suffix_of('nd3-dtn-bit') + "/box/current_trace/current_trace_generate_one", "w")
    #==
    #
    assert(dist_of([1000,1000,1000],[2000,1000,1000]) == 1000)
    random.seed()
    node1_trace = generate_points_of_random_in_range_and_n(
        {   
            "range":
                {   "x_range":[1000, 20000], 
                    "y_range":[1000, 20000], 
                    "z_range":[1000, 20000],
                },
            "n_of_points": 33
        }
    )
    node2_trace = generate_points_of_random_in_range_and_n(
        {   
            "range":
                {   "x_range":[1000, 20000], 
                    "y_range":[1000, 20000], 
                    "z_range":[1000, 20000],
                },
            "n_of_points": 33
        }
    )
    node3_trace = generate_points_of_random_in_range_and_n(
        {   
            "range":
                {   "x_range":[1000, 20000], 
                    "y_range":[1000, 20000], 
                    "z_range":[1000, 20000],
                },
            "n_of_points": 33
        }
    )
    node4_trace = generate_points_of_random_in_range_and_n(
        {   
            "range":
                {   "x_range":[1000, 20000], 
                    "y_range":[1000, 20000], 
                    "z_range":[1000, 20000],
                },
            "n_of_points": 33
        }
    )
    node5_trace = generate_points_of_random_in_range_and_n(
        {   
            "range":
                {   "x_range":[1000, 20000], 
                    "y_range":[1000, 20000], 
                    "z_range":[1000, 20000],
                },
            "n_of_points": 33
        }
    )
    node6_trace = generate_points_of_random_in_range_and_n(
        {   
            "range":
                {   "x_range":[1000, 20000], 
                    "y_range":[1000, 20000], 
                    "z_range":[1000, 20000],
                },
            "n_of_points": 33
        }
    )
    speed_vec_1 = generate_random_speed_vec([120, 222], 5)
    speed_vec_2 = generate_random_speed_vec([100, 200], 9)
    speed_vec_3 = generate_random_speed_vec([160, 250], 11)
    speed_vec_4 = generate_random_speed_vec([90, 240], 14)
    speed_vec_5 = generate_random_speed_vec([80, 260], 12)
    speed_vec_6 = generate_random_speed_vec([110, 200], 7)
    write_trace_into_file(node1_trace, speed_vec_1, 1000, 2, 0)
    write_trace_into_file(node2_trace, speed_vec_2, 1000, 2, 1)
    write_trace_into_file(node3_trace, speed_vec_3, 1000, 2, 2)
    write_trace_into_file(node4_trace, speed_vec_4, 1000, 2, 3)
    write_trace_into_file(node5_trace, speed_vec_5, 1000, 2, 4)
    write_trace_into_file(node6_trace, speed_vec_6, 1000, 2, 5)
#====================
# @brief another main function
def test03_functon() :
    print("python version:" + sys.version)
    global g_precise, g_trace_file 
    g_precise = 0.000001 
    g_trace_file = open(get_path_suffix_of('nd3-dtn-bit') + "/box/current_trace/current_trace_generate_one", "w")
    #==
    #
    assert(dist_of([1000,1000,1000],[2000,1000,1000]) == 1000)
    random.seed()
    node1_trace = [[1000, 1000, 3000], [1000, 5000, 6000], [1000, 5000, 5200]]
    node2_trace = [[1000, 1000, 1000], [5000, 1000, 4000], [5000, 1000, 5800]]
    node3_trace = [[1000, 1000, 2000], [1010, 1010, 2002]]
    node4_trace = [[5000, 5000, 10000], [5021, 5020, 10100]]
    node5_trace = [[1000, 5000, 5800], [1000, 5000, 6000], [5000, 5000, 10800]]
    node6_trace = [[5000, 1000, 4200], [5000, 1000, 4000], [5000, 5000, 9200]]
    speed_vec_1 = [25, 2, 1]
    speed_vec_2 = [25, 2, 1]
    speed_vec_3 = [5]
    speed_vec_4 = [5]
    speed_vec_5 = [1, 30, 1]
    speed_vec_6 = [1, 30, 1]
    write_trace_into_file(node1_trace, speed_vec_1, 1000, 0, 0)
    write_trace_into_file(node2_trace, speed_vec_2, 1000, 0, 1)
    write_trace_into_file(node3_trace, speed_vec_3, 1000, 0, 2)
    write_trace_into_file(node4_trace, speed_vec_4, 1000, 0, 3)
    write_trace_into_file(node5_trace, speed_vec_5, 1000, 0, 4)
    write_trace_into_file(node6_trace, speed_vec_6, 1000, 0, 5)
#======================
# @brief another main function
def test04_functon() :
    print("python version:" + sys.version)
    global g_precise, g_trace_file 
    g_precise = 0.000001 
    g_trace_file = open(get_path_suffix_of('nd3-dtn-bit') + "/box/current_trace/current_trace_generate_one", "w")
    #==
    #
    assert(dist_of([1000,1000,1000],[2000,1000,1000]) == 1000)
    random.seed()
    node3_trace = [[1000, 1000, 2000], [1010, 1010, 2002]]
    node4_trace = [[7000, 7000, 8000], [7021, 7020, 8100]]
    node1_trace = [[1000, 1500, 2500], [1000, 5000, 6000], [1000, 5000, 5800]]
    node5_trace = [[1000, 5000, 5800], [1000, 5000, 6000], [7000, 7000, 8800]]
    node2_trace = [[1500, 1500, 2000], [5000, 5000, 2000], [5000, 5100, 2000]]
    node6_trace = [[5000, 5100, 2000], [5000, 5000, 2000], [7000, 7000, 8200]]
    node7_trace = [[1500, 1000, 2500], [5000, 1000, 6000], [5000, 1000, 6200]]
    node8_trace = [[5000, 1000, 6200], [5000, 1000, 6000], [7000, 7000, 8300]]
    speed_vec_1 = [25, 2, 1]
    speed_vec_2 = [25, 2, 1]
    speed_vec_3 = [5]
    speed_vec_4 = [5]
    speed_vec_5 = [1, 30, 1]
    speed_vec_6 = [1, 30, 1]
    speed_vec_7 = [25, 2, 1]
    speed_vec_8 = [1, 30, 1]
    write_trace_into_file(node1_trace, speed_vec_1, 1000, 0, 0)
    write_trace_into_file(node2_trace, speed_vec_2, 1000, 0, 1)
    write_trace_into_file(node3_trace, speed_vec_3, 1000, 0, 2)
    write_trace_into_file(node4_trace, speed_vec_4, 1000, 0, 3)
    write_trace_into_file(node5_trace, speed_vec_5, 1000, 0, 4)
    write_trace_into_file(node6_trace, speed_vec_6, 1000, 0, 5)
    write_trace_into_file(node7_trace, speed_vec_7, 1000, 0, 6)
    write_trace_into_file(node8_trace, speed_vec_8, 1000, 0, 7)
#======================

test04_functon()
