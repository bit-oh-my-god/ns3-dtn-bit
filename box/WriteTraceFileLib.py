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
import sympy
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
    global g_precise
    dis = dist_of(v, [0.0,0.0,0.0])
    v0 = v[0] / dis
    v1 = v[1] / dis
    v2 = v[2] / dis
    assert((v0 * v0) + (v1 * v1) + (v2 * v2) - 1 < g_precise)
    assert((v0 * v0) + (v1 * v1) + (v2 * v2) - 1 > (-1 * g_precise))
    assert((v0 * v0) + (v1 * v1) + (v2 * v2) - 1 < 0.000001)
    assert((v0 * v0) + (v1 * v1) + (v2 * v2) - 1 > -0.000001)
    return [v0, v1, v2]
# end def refine_mode_one(v) :
#=====================
# @brief 
def generate_direction_by_line(linelist):
    try :
        list(linelist)
        list(linelist[0])
        len(linelist[0]) == 3
    except :
        raise RuntimeError
    n = len(linelist)
    directionlist = []
    for i in range(0, n, 1):
        linepoint = linelist[i]
        direction = [
            linelist[(i + 1) % n][0] - linelist[(i ) % n][0],
            linelist[(i + 1) % n][1] - linelist[(i ) % n][1],
            linelist[(i + 1) % n][2] - linelist[(i ) % n][2],
        ]
        direction = refine_mode_one(direction)
        directionlist.append(direction)
    assert(len(directionlist) == len(linelist))
    return directionlist
    nothing = 1
# end def generate_direction_by_line():
#======================
#
#======================
class CURVE_POINT(object) :
    def __init__(self, position, moving_vec, x_vec, y_vec):
        self.m_position =np.array(position)
        self.m_moving_vec = np.array(refine_mode_one(moving_vec))
        self.m_x_vec =np.array(refine_mode_one(x_vec))
        self.m_y_vec =np.array(refine_mode_one(y_vec))
        global g_precise
        assert(np.dot(np.array(self.m_x_vec), np.array(self.m_y_vec)) < g_precise)
        assert(np.dot(np.array(self.m_y_vec), np.array(self.m_moving_vec)) < g_precise)
        assert(np.dot(np.array(self.m_x_vec), np.array(self.m_moving_vec)) < g_precise)
    def get_p(self):
        return self.m_position 
    def get_m(self):
        return self.m_moving_vec 
    def get_x(self):
        return self.m_x_vec 
    def get_y(self):
        return self.m_y_vec 
#s_a=sympy.Symbol('a')
#s_b=sympy.Symbol('b')
#s_c=sympy.Symbol('c')
#solved = sympy.solve(
#    [
#        (s_a * s_a) + (s_b * s_b) + (s_c * s_c) - 1.0,
#        (s_b / sympy.sqrt((s_a * s_a) + (s_c * s_c))) - math.tan(angle_of_long_axis_and_xz_surface),
#        s_a * vertical_line[0] + s_b * vertical_line[1] + s_c * vertical_line[2],
#    ],
#    [s_a,s_b,s_c]),
class NUMPY_BASED_CURVE_LINE_GENERATOR(object) :
    def __init__(self, ):
        nothing = 1
        self.newcurvepoints = []
    # @Coordinate transformation (oldx, oldy, oldm) * coordinate_transformation = (cacux, cacuy, cacum)
    def generate_points_by(self, listofcurvepoint, theta_step, longax, shortax, coordinate_transformation=None, ct_flag =False, init_theta=0):
        length = len(listofcurvepoint)
        if ct_flag == False :
            cotran = np.mat([
                [1,0,0],
                [0,1,0],
                [0,0,1],
            ])
        else :
            cotran = coordinate_transformation
        for i in range(0, length, 1) :
            oldp =listofcurvepoint[i]
            cacu_coordinate = np.transpose(np.mat([oldp.get_x(), oldp.get_y(), oldp.get_m()])) * cotran
            cacu_coordinate = np.transpose(cacu_coordinate)
            #if coordinate_transformation == None:
                #print("cacu_coordinate=\n{0}".format(cacu_coordinate))
                #print("old_coord=\n{0}".format(np.array([oldp.get_x(), oldp.get_y(), oldp.get_m()])))
                #print("coord_tran=\n{0}".format(cotran))
                #assert(cacu_coordinate[0].all() == cacux.all())
            #cacux = oldp.get_x()
            #cacuy = oldp.get_y()
            #cacum = oldp.get_m()
            cacux = np.array(cacu_coordinate[0])[0]
            cacuy = np.array(cacu_coordinate[1])[0]
            cacum = np.array(cacu_coordinate[2])[0]
            cacup = oldp.get_p()
            theta = theta_step * i + init_theta
            curve_point =  self.private_calcu_new(cacux, cacuy, cacum, cacup, theta, longax, shortax)
            self.newcurvepoints.append(curve_point)
    def private_calcu_new(self, cacux, cacuy, cacum, cacup, theta, longax, shortax) :
            rfang = ((shortax*shortax) * (longax *longax)) / ((longax *longax * math.sin(theta) * math.sin(theta)) + (shortax*shortax *math.cos(theta) * math.cos(theta)))
            r = math.sqrt(rfang)
            syv = r * math.cos(theta)
            sxv = r * math.sin(theta)
            newx = sxv * cacux + syv * cacuy
            #print("svx={0}, syv={1}, cacux= {2}, cacuy={3},\
            #\n newx = {4},".format(sxv, syv, cacux, cacuy, newx))
            newp = cacup + newx
            newy = cacum
            newm = chacheng(newy, newx)
            #print("newx = {0}; newp = {1}, cacup = {2}, newy = {3}, newm = {4}"\
            #.format(newx, newp, cacup, newy, newm))
            curve_point = CURVE_POINT(newp,newm,newx,newy)
            return curve_point
    def get_new_curve_points(self) :
        newre = self.newcurvepoints
        self.newcurvepoints = []
        return newre
def NUMPY_get_new_trace(newcurvepoints) :
    length = len(newcurvepoints)
    returnedlist = []
    for i in range(0, length, 1) :
        returnedlist.append(newcurvepoints[i].get_p())
    return returnedlist
#=====================
# end class curve_line_generator(object) :
#====================
#=========================
# @brief this func would generate the corner of a 'n' edge (n vertex also) spiral-polygon with corelist of 'corelist', long axis(length in 'X' axis), short axis, vertical_line_list, angle between long axis and xz surface, 
# every core moves 'core2corner' step, vertex of spiral-polygon generate  point, core list would move 'corelistmove' index
# @return a vector of vertex of spiral-polygon, which is caused by core-moving, 
#    BUG!!!!!!!!!
def generate_points_of_spiral_polygon_with_core_size_slope(n, core_list, size_long, size_width, vertical_line_list, angle_of_long_axis_and_xz_surface, is_clock_wise, root_1_or_2, axis_x, core2corner, corelistmove, cvxcalculator=None) :
    #print("nothing")
    assert(n >= 3)
    try :
        list(core_list)
        list(core_list[0])
        len(core_list[0]) == 3
        list(vertical_line_list)
        list(vertical_line_list[0])
        len(vertical_line_list[0]) == 3
    except :
        raise RuntimeError
    # normaly, corelistmove should > len(core_list)
    assert(corelistmove > 10)
    # =============
    def shouldpick(i, core2corner) :
        if core2corner == 1:
            return True
        if i == 0 :
            return True
        elif i % core2corner == 0 :
            return True
        else :
            return False
    # end def shouldpick(i, core2corner) :
    # ===============
    spiral_polygen = []
    for j in range(0, corelistmove, 1) :
        i = j % len(core_list)
        core = core_list[i]
        vertical_line= vertical_line_list[i]
        if shouldpick(j, core2corner) :
            polygon = generate_points_of_polygon_with_core_size_slope(n, core, size_long, size_width, vertical_line, angle_of_long_axis_and_xz_surface, is_clock_wise, root_1_or_2, axis_x, cvxcalculator)
            pickpoint = polygon[ j % n]
            spiral_polygen.append(pickpoint)
        else :
            continue
    return spiral_polygen
# end    def generate_points_of_polygon_with_core_size_slope(n, core, size_long, size_width, vertical_line, angle_of_long_axis_and_xz_surface, is_clock_wise, root_1_or_2, axis_x) :
# ===========================
# @brief this func would generate the corner of a 'n' edge (n vertex also) polygon with core of 'core', long axis(length in 'X' axis), short axis, vertical_line, angle between long axis and xz surface
# @return a vector of vertex of polygon
def generate_points_of_polygon_with_core_size_slope(n, core, size_long, size_width, vertical_line, angle_of_long_axis_and_xz_surface, is_clock_wise, root_1_or_2, axis_x, cvxcalculator=None) :
    assert(n >= 3)
    try :
        list(core)
        len(core) == 3
        list(vertical_line)
        len(vertical_line) == 3
    except :
        raise RuntimeError
    #assert size_long
    #assert size_width
    #print("nothing")
    def solve_it_on_plain(n, size_long, size_width) :
        global g_precise
        assert(n > 6)
        angle = []
        result = []
        pi2 = math.pi / 2.0
        for i in range(0, n, 1) :
            one = 2 * math.pi / float(n)
            angle.append(i * one)
        #print(angle)
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
        # return [a, b, c]
        def solveit(vertical_line, angle_of_long_axis_and_xz_surface, root_1_or_2) :
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
            c1 = math.sqrt(factor6) - (factor4 / (2.0 * factor3))
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
                print("error:specify which root you want, root_1_or_2 can only have two value :'1, 2'")
                sys.exit()
        # end def solveit(vertical_line, angle_of_long_axis_and_xz_surface, root_1_or_2) :
        def solveitbysymp(vertical_line, angle_of_long_axis_and_xz_surface, root_1_or_2) :
            s_a=sympy.Symbol('a')
            s_b=sympy.Symbol('b')
            s_c=sympy.Symbol('c')
            solved = sympy.solve(
                [
                    (s_a * s_a) + (s_b * s_b) + (s_c * s_c) - 1.0,
                    (s_b / sympy.sqrt((s_a * s_a) + (s_c * s_c))) - math.tan(angle_of_long_axis_and_xz_surface),
                    s_a * vertical_line[0] + s_b * vertical_line[1] + s_c * vertical_line[2],
                ],
                [s_a,s_b,s_c]),
            #print(solved[0][1])
            #print(float(solved[0][1][1]))
            if (root_1_or_2 == 1) :
                r = 1
            elif (root_1_or_2 == 2) :
                r = 2
            else :
                print("error:specify which root you want, root_1_or_2 can only have two value :'1, 2'")
                sys.exit()
            try :
                return [float(solved[0][r][0]), float(solved[0][r][1]), float(solved[0][r][2])]
            except TypeError:
                raise TypeError
                #print("fuckou !!! ! !! bad complex root, drop nonsolid-part, {0}".format(solved[0][0][0]))
                #ar = complex(solved[0][r][0]).real
                #br = complex(solved[0][r][1]).real
                #cr = complex(solved[0][r][2]).real
                #return [float(ar), float(br), float(cr)]
        # end def solveitbysymp(vertical_line, angle_of_long_axis_and_xz_surface, root_1_or_2) :
        try :
            returned = solveit(vertical_line, angle_of_long_axis_and_xz_surface, root_1_or_2)
            #print("end one solveit")
        except ValueError:
            #print("bad in solveit, let's go for one solveitbysymp")
            #returned = solveitbysymp(vertical_line, angle_of_long_axis_and_xz_surface, root_1_or_2)
            #print("end one solveitbysymp")
            raise ValueError
        return returned
    # end def get_changing_vector_x(vertical_line, angle_of_long_axis_and_xz_surface, root_1_or_2) :
    #angle_of_long_axis_and_xz_surface
    def get_changing_vector_y(z, x) :
        #print("nothing")
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
    # end def map_one_to_slope(point, core, changing_vec) :
    
    # first get the solution on plain, then map it to slope surface
    plain = solve_it_on_plain(n, size_long, size_width)
    #print("plain:")
    #print(plain)
    assert(len(plain[0]) == 3)
    # 坐标基 变换
    cvx = []
    global g_precise
    if (angle_of_long_axis_and_xz_surface != None) :
        try : 
            cvx = get_changing_vector_x(vertical_line, angle_of_long_axis_and_xz_surface, root_1_or_2)
        except TypeError:
            raise TypeError
    elif (axis_x != []) :
        cvx = axis_x
        assert((cvx[0] * vertical_line[0] + cvx[1] * vertical_line[1] + cvx[2] * vertical_line[2]) < g_precise)
    elif (cvxcalculator != None) :
        cvx = cvxcalculator(vertical_line)
        assert_v = (cvx[0] * vertical_line[0] + cvx[1] * vertical_line[1] + cvx[2] * vertical_line[2])
        assert(assert_v < g_precise)
    else :
        sys.exit("error-fuckyoumom")
    cvz = refine_mode_one(vertical_line)
    cvx = refine_mode_one(cvx)
    cvy = get_changing_vector_y(cvz, cvx)
    cvall = [cvx, cvy, cvz]
    ret = list(map(lambda x : map_one_to_slope(x, core, cvall), plain)) 
    #print("changing_vec")
    #print(cvall)
    #print("slope:")
    #print(ret)
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
    return math.sqrt(pow(vec1[0] - vec2[0], 2) + pow(vec1[1] - vec2[1], 2) + pow(vec1[2] - vec2[2], 2))
def chacheng(vec1, vec2) :
    l=vec1[0]
    m=vec1[1]
    n=vec1[2]
    o=vec2[0]
    p=vec2[1]
    q=vec2[2]
    return [m * q - n * p, n * o - l * q, l * p - m * o]
#========================
# @brief this func would write a sequence of trace into file with vector of points and speed and time, start at vector_of_points[m] as nodeid n
# @input vector_of_points is 'list of points you plan'
# @input speed_vec is speed you plan
def write_trace_into_file(vector_of_points, speed_vec, alltime, m, n) :
    #assert
    #print("nothing")
    #assert(len(vector_of_points) > 3)
    cur = (m) % len(vector_of_points)
    cur_t = 0
    speed_index = 0
    global g_trace_file, g_precise
    g_trace_file.write(ori_pattern(vector_of_points[cur], n))
    time_of_points = []
    while cur_t < alltime :
        to = (cur + 1) % len(vector_of_points)
        speed = speed_vec[speed_index % len(speed_vec)]
        if speed - 0.0 < g_precise and dist_of(vector_of_points[cur], vector_of_points[to]) < g_precise:
            cur += 1
            if (cur == len(vector_of_points)):
                break
            continue
        elif speed - 0.0 < g_precise and dist_of(vector_of_points[cur], vector_of_points[to]) > g_precise:
            sys.exit("can't error-121871234")
        else :
            nothing = 1
        g_trace_file.write(move_pattern(vector_of_points[to], n, cur_t, speed))
        time_of_points.append(cur_t)
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
    return time_of_points
# end def write_trace_into_file(vector_of_points, speed_vec, alltime, m, n) :
# @brief invoke write_trace_into_file, but assert len(vector_of_points) = len(speed_vec) 
def write_trace_into_file_exactly(vector_of_points, speed_vec, alltime, n) :
    assert(len(speed_vec) == len(vector_of_points))
    #assert(len(speed_vec) == alltime)
    return write_trace_into_file(vector_of_points, speed_vec, alltime, 0, n)
# end def write_trace_into_file_exactly(vector_of_points, speed_vec, alltime, m, n) :
# @brief calculate speed_vec from vector_of_points and time_of_points
def calculate_speed_vec_from(vector_of_points, time_of_points):
    nothing = 1
    lent = len(time_of_points)
    lenv = len(vector_of_points)
    print("len(time_of_points)={0}".format(lent))
    print("len(vector_of_points)={0}".format(lenv))
    assert(lent <= lenv)
    # time points is what we write (restrict by write-simu_frame_point-time), vector of points is what we can write,
    speed_vec = [1] * lenv
    assert(len(speed_vec) == lenv)
    for i in range(0, lent, 1) :
        to = vector_of_points[(i + 1) % lenv]
        fr = vector_of_points[(i) % lenv]
        tot = time_of_points[(i + 1) % lent]
        frt = time_of_points[(i) % lent]
        if (tot > frt) :
            speed = dist_of(fr, to) / (tot - frt)
            speed_vec[i] = speed
        else :
            speed_vec[i] = 0.1 # the last movement speed 
            assert(i == lent - 1)
    #assert(len(speed_vec) == lent)
    return speed_vec
# end    def calculate_speed_vec_from():
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
    colors = plt.cm.ScalarMappable(cmap=plt.cm.get_cmap('jet')).to_rgba(x=np.linspace(0, 1, len(v_of_v_p)))
    colors = [ 
        'b' ,	#blue
        'g' ,	#green
        'r' ,	#red
        'c', 	#cyan
        'm', 	#magenta
        'y' ,	#yellow
        'k', 	#black
    ]
    #print(len(colors))
    countn = 0
    for vector_of_points in v_of_v_p :
        xx = []
        yy = []
        zz = []
        for v in vector_of_points :
            xx.append(v[0])
            yy.append(v[1])
            zz.append(v[2])
        ax.scatter(xx, yy, zz, c= colors[countn % len(colors)])
        #print(colors[countn % len(colors)])
        #ax.plot(xx, yy, zz, c= colors[countn])
        countn += 1
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
    #print("n_of_points is{0}, len = {1}".format(n_of_points, len(points_vec)))
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
#==================== # @brief a main function
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
#==================== def test02_functon() :
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
#====================== def test03_functon() :
# @brief another main function
def test04_functon() :
    print("python version:" + sys.version)
    global g_precise, g_trace_file 
    g_precise = 0.000001 
    g_trace_file = open(get_path_suffix_of('ns3-dtn-bit') + "/box/current_trace/current_trace_generate_one", "w")
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
    print_in_3d(node7_trace)
    write_trace_into_file(node1_trace, speed_vec_1, 1000, 0, 0)
    write_trace_into_file(node2_trace, speed_vec_2, 1000, 0, 1)
    write_trace_into_file(node3_trace, speed_vec_3, 1000, 0, 2)
    write_trace_into_file(node4_trace, speed_vec_4, 1000, 0, 3)
    write_trace_into_file(node5_trace, speed_vec_5, 1000, 0, 4)
    write_trace_into_file(node6_trace, speed_vec_6, 1000, 0, 5)
    write_trace_into_file(node7_trace, speed_vec_7, 1000, 0, 6)
    write_trace_into_file(node8_trace, speed_vec_8, 1000, 0, 7)
#====================== def test04_functon() :
# @brief another main function
# bug !!!!!!!!! not solved
def test05_functon() :
    print("python version:" + sys.version)
    global g_precise, g_trace_file 
    g_precise = 0.00001 
    g_trace_file = open(get_path_suffix_of('ns3-dtn-bit') + "/box/current_trace/current_trace.tcl", "w")
    simu_frame_point = 100
    simu_time = 100
    frame_precise = 3 # every time scale 3 trace-point
    def smooth_speed_vec(tracelist, speed):
        speed_vec_var_02 = np.ones(len(tracelist))
        speed_vec_var_02 *= speed
        return speed_vec_var_02
    # end def smooth_speed_vec(tracelist, speed):
    #==
    #============
    planet_trace_of_sun = np.ones((int(simu_frame_point * frame_precise),3))
    vertical_of_sun = np.ones((int(simu_frame_point * frame_precise),3))
    planet_trace_of_sun = planet_trace_of_sun * 8000
    vertical_of_sun[:, 0] *= 0.05
    vertical_of_sun[:, 1] *= 0.02
    vertical_of_sun[:, 2] *= 0.9
    #print(planet_trace_of_sun)
    planet_trace_of_earth = generate_points_of_spiral_polygon_with_core_size_slope(
        n=124, 
        core_list=planet_trace_of_sun, 
        size_long=500.0, 
        size_width=500.0, 
        vertical_line_list=vertical_of_sun, 
        angle_of_long_axis_and_xz_surface=(math.pi / 6.0), 
        is_clock_wise=True, 
        root_1_or_2=1, 
        axis_x=[],
        core2corner=2,
        corelistmove=len(planet_trace_of_sun),
        )
    #============
    # @ brief return cvx, which cvx * vertical_line = 0
    def solvecvxbysympy(vertical_line, tmpvec=None) :
        s_a=sympy.Symbol('a')
        s_b=sympy.Symbol('b')
        s_c=sympy.Symbol('c')
        if tmpvec == None :
            tmpvec = [0.05, 0.02, 0.9]
        solved = sympy.solve(
            [
                (s_a * s_a) + (s_b * s_b) + (s_c * s_c) - 1.0,
                s_a * tmpvec[0] + s_b * tmpvec[1] + s_c * tmpvec[2],
                s_a * vertical_line[0] + s_b * vertical_line[1] + s_c * vertical_line[2],
            ],
            [s_a,s_b,s_c]),
        print(solved)
        return [
            float(solved[0][0][0]),
            float(solved[0][0][1]),
            float(solved[0][0][2]),
        ]
    # end def solvecvxbysympy(vertical_line) :
    vertical_of_earth = generate_direction_by_line(planet_trace_of_earth)
    #vertical_of_earth1 = np.ones((int(len(planet_trace_of_earth)),3))
    node_trace_of_n1 = generate_points_of_spiral_polygon_with_core_size_slope(
        n=511, 
        core_list=planet_trace_of_earth, 
        size_long=100.0, 
        size_width=100.0, 
        vertical_line_list=vertical_of_earth, 
        angle_of_long_axis_and_xz_surface=None, 
        is_clock_wise=True, 
        root_1_or_2=1, 
        axis_x=[],
        core2corner=1,
        corelistmove=len(planet_trace_of_earth),
        cvxcalculator=solvecvxbysympy,
        )
    for ii in range(0, len(node_trace_of_n1),1) :
        p1 = node_trace_of_n1[ii]
        p2 = planet_trace_of_earth[ii]
        print("dist = {0}".format(dist_of(p1, p2)))
    print("earth point size ={0}".format(len(planet_trace_of_earth)))
    print("n1 point size ={0}".format(len(node_trace_of_n1)))
    #print(node_trace_of_n1)
    write_trace_into_file_exactly(
        vector_of_points=planet_trace_of_earth, 
        speed_vec=smooth_speed_vec(planet_trace_of_earth, 30), 
        alltime=simu_time, 
        n=0)
    write_trace_into_file_exactly(
        vector_of_points=node_trace_of_n1, 
        speed_vec=smooth_speed_vec(node_trace_of_n1, 70), 
        alltime=simu_time, 
        n=1)
    assert(len(planet_trace_of_earth) == len(node_trace_of_n1))
    #def write_trace_into_file_exactly(vector_of_points, speed_vec, alltime, n) :
    #write_trace_into_file(node7_trace, [10], 1000, 5, 6)
    print_in_3d_group([planet_trace_of_sun,planet_trace_of_earth,node_trace_of_n1,])
    g_trace_file.close()
#= def test05_functon() :
# ======

# ====
def sympy_test() :
    # solve these three equation
    # 1. a^2 + b^2 + c^2 = 1
    # 2. b / sqrt(a^2 + c^2) = tan(angle)
    # 3. a * vertical_line[0] + b * vertical_line[1] + c * vertical_line[2] = 0
    # return [a, b, c]
    sx=sympy.Symbol('x')
    sy=sympy.Symbol('y')
    print(round(math.cos(0.1), 4))
    print(round(math.sin(0.1), 4))
    solved = sympy.solve(
        [
            ((sx * sx) / (20*20)) + ((sy * sy) / (30 *30))- 1.0,
            (sy / sympy.sqrt(sx * sx + sy * sy)) - round(math.cos(0.1), 4),
            (sx / sympy.sqrt(sx * sx + sy * sy)) - round(math.sin(0.1),4),
            #sx - sympy.sqrt(sy),
            #sy - 1.2,
        ],
        [sx, sy])
    print(solved)
    #[print(float(solved[0][1][1]))
    #cmp = complex(1, 1) ** (1/2)
    a1 = np.array([1,2,3])
    a2 = np.array([2,0.9,1])
    print(np.dot(a1, a1))
    print(np.linalg.det(np.outer(a2, a1)))
    #print(float(cmp.real))
    list_var = [1]
    print(list_var * 4)
    oldcoord = np.mat([
        [1,2,3],
        [4,5,6],
        [7,8,9],
    ])
    coord_tran = np.mat([
        [1, 0, 0],
        [0, 0, 1],
        [0, 1, 0],
    ])
    newcoord = np.transpose(oldcoord) * coord_tran
    newcoord = np.transpose(newcoord)
    print("oldcoor=\n{0},coord_tran=\n{1},newcoor=\n{2}".format(
        oldcoord, coord_tran, newcoord,
    ))
# end def sympy_test() :
#==============================
# @brief another main function
def test06_functon() :
    print("python version:" + sys.version)
    global g_precise, g_trace_file 
    g_precise = 0.00001 
    g_trace_file = open(get_path_suffix_of('ns3-dtn-bit') + "/box/current_trace/current_trace.tcl", "w")
    simu_frame_point = 100
    simu_time = 100
    frame_precise = 3 # every time scale 3 trace-point
    def smooth_speed_vec(tracelist, speed):
        speed_vec_var_02 = np.ones(len(tracelist))
        speed_vec_var_02 *= speed
        return speed_vec_var_02
    # end def smooth_speed_vec(tracelist, speed):
    #==
    #============
    planet_trace_of_sun = np.ones((int(simu_frame_point * frame_precise),3))
    vertical_of_sun = np.ones((int(simu_frame_point * frame_precise),3))
    #planet_trace_of_sun = planet_trace_of_sun * 8000
    planet_trace_of_sun[:, 0] = np.linspace(3000, 3000 + len(planet_trace_of_sun), len(planet_trace_of_sun))
    planet_trace_of_sun[:, 1] = np.linspace(3000, 3000 + len(planet_trace_of_sun), len(planet_trace_of_sun))
    planet_trace_of_sun[:, 2] = np.linspace(1000, 1000 + len(planet_trace_of_sun), len(planet_trace_of_sun))
    vertical_of_sun[:, 0] *= 0.05
    vertical_of_sun[:, 1] *= 0.02
    vertical_of_sun[:, 2] *= 0.9
    #print(planet_trace_of_sun)
    planet_trace_of_earth = generate_points_of_spiral_polygon_with_core_size_slope(
        n=124, 
        core_list=planet_trace_of_sun, 
        size_long=50.0, 
        size_width=50.0, 
        vertical_line_list=vertical_of_sun, 
        angle_of_long_axis_and_xz_surface=(math.pi / 6.0), 
        is_clock_wise=True, 
        root_1_or_2=1, 
        axis_x=[],
        core2corner=2,
        corelistmove=244,
        )
    print("earth point size ={0}".format(len(planet_trace_of_earth)))
    write_trace_into_file_exactly(
        vector_of_points=planet_trace_of_earth, 
        speed_vec=smooth_speed_vec(planet_trace_of_earth, 30), 
        alltime=simu_time, 
        n=0)
    write_trace_into_file_exactly(
        vector_of_points=planet_trace_of_sun, 
        speed_vec=smooth_speed_vec(planet_trace_of_sun, 30), 
        alltime=simu_time, 
        n=1)
    #def write_trace_into_file_exactly(vector_of_points, speed_vec, alltime, n) :
    #write_trace_into_file(node7_trace, [10], 1000, 5, 6)
    # ====================
    print_in_3d_group([planet_trace_of_sun,planet_trace_of_earth])
    g_trace_file.close()
#= def test06_functon() :
#==============================
# @brief another main function
def test07_functon() :
    print("python version:" + sys.version)
    global g_precise, g_trace_file 
    g_precise = 0.00001 
    g_trace_file = open(get_path_suffix_of('ns3-dtn-bit') + "/box/current_trace/current_trace.tcl", "w")
    simu_frame_point = 1000
    simu_time = 2000
    g_original_point = np.array([10000, 10000, 10000])
    def smooth_speed_vec(tracelist, speed):
        speed_vec_var_02 = np.ones(len(tracelist))
        speed_vec_var_02 *= speed
        return speed_vec_var_02
    # end def smooth_speed_vec(tracelist, speed):
    def assert_coord_tran(coord_tran01) :
        print(np.array(coord_tran01[0])[0])
        print(np.array(coord_tran01[1])[0])
        print(np.array(coord_tran01[2])[0])
        assert(np.dot(np.array(coord_tran01[0][0]), np.array(coord_tran01[1])[0]) < g_precise)
        assert(np.dot(np.array(coord_tran01[0])[0], np.array(coord_tran01[2])[0]) < g_precise)
        assert(np.dot(np.array(coord_tran01[2])[0], np.array(coord_tran01[1])[0]) < g_precise)
    # end def assert_coord_tran(coord_tran01) :
    #============
    generator = NUMPY_BASED_CURVE_LINE_GENERATOR()

    curvepointlist_of_outergod = [CURVE_POINT(position=[3000,3000,3000],moving_vec=[0,0,1],x_vec=[1,0,0],y_vec=[0,1,0])] * simu_frame_point
    planet_trace_of_outergod = NUMPY_get_new_trace(curvepointlist_of_outergod)
    curvepointlist_of_realsun = [CURVE_POINT(position=list(g_original_point),moving_vec=[0,0,1],x_vec=[1,0,0],y_vec=[0,1,0])] * simu_frame_point
    planet_trace_of_sun = NUMPY_get_new_trace(curvepointlist_of_realsun)

    curvepointlist_of_core_earth = [CURVE_POINT(position=list(g_original_point),moving_vec=[0,0,1],x_vec=[1,0,0],y_vec=[0,1,0])] * simu_frame_point
    mv =refine_mode_one([0,0.1,1])
    xv =refine_mode_one([1,0.1,-0.01])
    yv =chacheng(mv, xv)
    curvepointlist_of_core_mars = [CURVE_POINT(position=list(g_original_point),moving_vec=mv, x_vec=xv, y_vec=yv)] * simu_frame_point

    

#=== earth
    generator.generate_points_by(curvepointlist_of_core_earth, (math.pi / 120.0), 4300, 4250)
    curvepointlist_of_earth =generator.get_new_curve_points()
    planet_trace_of_earth = NUMPY_get_new_trace(curvepointlist_of_earth)
    print("ptearth[1] = {0}; len(ptearth) = {1}".format(planet_trace_of_earth[1], len(planet_trace_of_earth)))
#=== earth

#=== moon
    coord_tran00 = np.mat([
        [1, 0, 0],
        [0, 0, 1],
        [0, 1, 0],
    ])
    assert_coord_tran(coord_tran00)
    generator.generate_points_by(curvepointlist_of_earth, (math.pi / 11.0), 290, 300, coordinate_transformation=coord_tran00
    , ct_flag=True, init_theta = 0.8)
    curvepointlist_of_moon = generator.get_new_curve_points()
    planet_trace_of_moon = NUMPY_get_new_trace(curvepointlist_of_moon)
    print("ptmoon[1] = {0}; len(ptmoon) = {1}".format(planet_trace_of_moon[1], len(planet_trace_of_moon)))
#=== moon

#=== mars
    generator.generate_points_by(curvepointlist_of_core_mars, (math.pi / 64.0), 1980, 1800, init_theta = 3.8)
    curvepointlist_of_mars = generator.get_new_curve_points()
    planet_trace_of_mars = NUMPY_get_new_trace(curvepointlist_of_mars)
    print("ptmars[1] = {0}; len(ptmars) = {1}".format(planet_trace_of_mars[1], len(planet_trace_of_mars)))
#=== mars

#=== satt of mars
    coord_tran01 = np.mat([
        refine_mode_one([1, 0.1, -0.6]),
        refine_mode_one([0.2, 4, 1]),
        refine_mode_one(chacheng([1, 0.1, -0.6], [0.2, 4, 1])), 
        ])
    assert_coord_tran(coord_tran01)
    generator.generate_points_by(curvepointlist_of_mars, (math.pi / 14.0), 370, 360, coordinate_transformation= coord_tran01, 
    ct_flag=True, init_theta = 3.8) # TODO change this
    curvepointlist_of_mars_satt = generator.get_new_curve_points()
    trace_of_mars_satt = NUMPY_get_new_trace(curvepointlist_of_mars_satt)
    print("ptmars[1] = {0}; len(ptmars) = {1}".format(trace_of_mars_satt[1], len(trace_of_mars_satt)))
#=== satt of mars

#=== outer_relay_sattelitte
    coord_tran05 = np.mat([
        refine_mode_one([0.1, -0.1, 2]),
        refine_mode_one([0.5, 2.9, 0.12]),
        refine_mode_one(chacheng([0.5, 2.9, 0.12], [0.1, -0.1, 2])), 
        ])
    assert_coord_tran(coord_tran05)
    ppof = np.array(g_original_point) + (900 * np.array(coord_tran05[1])[0])
    curvepointlist_of_core_relaysattelite = [CURVE_POINT(position=list(ppof),moving_vec=np.array(coord_tran05[0])[0], x_vec=np.array(coord_tran05[2])[0], y_vec=np.array(coord_tran05[1])[0])] * simu_frame_point

    generator.generate_points_by(curvepointlist_of_core_relaysattelite, (math.pi / 80.0), 5580, 2500, 
    init_theta = 0) #TODO change this
    curvepointlist_of_relaysattelite = generator.get_new_curve_points()
    trace_of_relaysattelite = NUMPY_get_new_trace(curvepointlist_of_relaysattelite)
    print("ptrelay[1] = {0}; len(ptrelay) = {1}".format(trace_of_relaysattelite[1], len(trace_of_relaysattelite)))
#=== outer_relay_sattelitte

#=== outer_relay_sattelitte1
    coord_tran02 = np.mat([
        refine_mode_one([1, 0.1, -0.6]),
        refine_mode_one([0.2, 4, 1]),
        refine_mode_one(chacheng([1, 0.1, -0.6], [0.2, 4, 1])), 
        ])
    assert_coord_tran(coord_tran02)
    ppof = np.array(g_original_point) + (900 * np.array(coord_tran02[1])[0])
    curvepointlist_of_core_relaysattelite1 = [CURVE_POINT(position=list(ppof),moving_vec=np.array(coord_tran02[0])[0], x_vec=np.array(coord_tran02[2])[0], y_vec=np.array(coord_tran02[1])[0])] * simu_frame_point

    generator.generate_points_by(curvepointlist_of_core_relaysattelite1, (math.pi / 60.0), 4580, 3200, 
    init_theta = 0.8)
    curvepointlist_of_relaysattelite1 = generator.get_new_curve_points()
    trace_of_relaysattelite1 = NUMPY_get_new_trace(curvepointlist_of_relaysattelite1)
    print("ptrelay1[1] = {0}; len(ptrelay1) = {1}".format(trace_of_relaysattelite1[1], len(trace_of_relaysattelite1)))
#=== outer_relay_sattelitte1

#=== outer_relay_sattelitte2
    coord_tran03 = np.mat([
        refine_mode_one([0.5, 0.1, -0.4]),
        refine_mode_one([0.2, 3, 1]),
        refine_mode_one(chacheng([0.5, 0.1, -0.4], [0.2, 3, 1])), 
        ])
    assert_coord_tran(coord_tran03)
    ppof = np.array(g_original_point) + (900 * np.array(coord_tran03[1])[0])
    curvepointlist_of_core_relaysattelite2 = [CURVE_POINT(position=list(ppof),moving_vec=np.array(coord_tran03[0])[0], x_vec=np.array(coord_tran03[2])[0], y_vec=np.array(coord_tran03[1])[0])] * simu_frame_point

    generator.generate_points_by(curvepointlist_of_core_relaysattelite2, (math.pi / 70.0), 4580, 1900,
    init_theta = 0.8)
    curvepointlist_of_relaysattelite2 = generator.get_new_curve_points()
    trace_of_relaysattelite2 = NUMPY_get_new_trace(curvepointlist_of_relaysattelite2)
    print("ptrelay2[1] = {0}; len(ptrelay2) = {1}".format(trace_of_relaysattelite2[1], len(trace_of_relaysattelite2)))
#=== outer_relay_sattelitte2

#=== outer_relay_sattelitte3
    coord_tran04 = np.mat([
        refine_mode_one([10, 1, -0.6]),
        refine_mode_one([0.2, 4, 10]),
        refine_mode_one(chacheng([10, 1, -0.6], [0.2, 4, 10])), 
        ])
    assert_coord_tran(coord_tran04)
    ppof = np.array(g_original_point) + (900 * np.array(coord_tran04[1])[0])
    curvepointlist_of_core_relaysattelite3 = [CURVE_POINT(position=list(ppof),moving_vec=np.array(coord_tran04[0])[0], x_vec=np.array(coord_tran04[2])[0], y_vec=np.array(coord_tran04[1])[0])] * simu_frame_point

    generator.generate_points_by(curvepointlist_of_core_relaysattelite3, (math.pi / 40.0), 7580, 4200, 
    init_theta = 0.8)
    curvepointlist_of_relaysattelite3 = generator.get_new_curve_points()
    trace_of_relaysattelite3 = NUMPY_get_new_trace(curvepointlist_of_relaysattelite3)
    print("ptrelay3[1] = {0}; len(ptrelay3) = {1}".format(trace_of_relaysattelite3[1], len(trace_of_relaysattelite3)))
#=== outer_relay_sattelitte3

#=== outer_relay_sattelitte4
    coord_tran05 = np.mat([
        refine_mode_one([7, 2, -2.6]),
        refine_mode_one([0.3, -0.4, 0.5]),
        refine_mode_one(chacheng([0.3, -0.4, 0.5], [7, 2, -2.6])), 
        ])
    assert_coord_tran(coord_tran05)
    ppof = np.array(g_original_point) + (1400 * np.array(coord_tran05[0])[0])
    curvepointlist_of_core_relaysattelite4 = [CURVE_POINT(position=list(ppof),moving_vec=np.array(coord_tran05[0])[0], x_vec=np.array(coord_tran05[2])[0], y_vec=np.array(coord_tran05[1])[0])] * simu_frame_point

    generator.generate_points_by(curvepointlist_of_core_relaysattelite4, (math.pi / 40.0), 2580, 3200, 
    init_theta = 3.8)
    curvepointlist_of_relaysattelite4 = generator.get_new_curve_points()
    trace_of_relaysattelite4 = NUMPY_get_new_trace(curvepointlist_of_relaysattelite4)
    print("ptrelay3[1] = {0}; len(ptrelay3) = {1}".format(trace_of_relaysattelite4[1], len(trace_of_relaysattelite4)))
#=== outer_relay_sattelitte4

#=== outer_relay_sattelitte5
    coord_tran06 = np.mat([
        refine_mode_one([1.5, 1.1, -0.5]),
        refine_mode_one([-0.7, 3, 4.5]),
        refine_mode_one(chacheng([-0.7, 3, 4.5], [1.5, 1.1, -0.5])), 
        ])
    assert_coord_tran(coord_tran06)
    ppof = np.array(g_original_point) + (1900 * np.array(coord_tran06[1])[0])
    curvepointlist_of_core_relaysattelite5 = [CURVE_POINT(position=list(ppof),moving_vec=np.array(coord_tran06[0])[0], x_vec=np.array(coord_tran06[2])[0], y_vec=np.array(coord_tran06[1])[0])] * simu_frame_point

    generator.generate_points_by(curvepointlist_of_core_relaysattelite5, (math.pi / 70.0), 3580, 1900,
    init_theta = 2.8)
    curvepointlist_of_relaysattelite5 = generator.get_new_curve_points()
    trace_of_relaysattelite5 = NUMPY_get_new_trace(curvepointlist_of_relaysattelite5)
    print("ptrelay2[1] = {0}; len(ptrelay2) = {1}".format(trace_of_relaysattelite5[1], len(trace_of_relaysattelite5)))
#=== outer_relay_sattelitte5

# ===================== write into file
    time_points_of_earth = write_trace_into_file_exactly(
        vector_of_points=planet_trace_of_earth, 
        speed_vec=smooth_speed_vec(planet_trace_of_earth, 39) , 
        alltime=simu_time, 
        n=0)

    speed_vec_of_moon = calculate_speed_vec_from(planet_trace_of_moon, time_points_of_earth)

    write_trace_into_file_exactly(
        vector_of_points=planet_trace_of_moon, 
        speed_vec=speed_vec_of_moon, 
        alltime=simu_time, 
        n=1)

    time_points_of_mars = write_trace_into_file_exactly(
        vector_of_points=planet_trace_of_mars, 
        speed_vec=smooth_speed_vec(planet_trace_of_mars, 33) , 
        alltime=simu_time, 
        n=2)

    speed_vec_of_mars_satt = calculate_speed_vec_from(trace_of_mars_satt, time_points_of_mars)

    write_trace_into_file_exactly(
        vector_of_points=trace_of_relaysattelite, 
        speed_vec=smooth_speed_vec(trace_of_relaysattelite, 70) , 
        alltime=simu_time, 
        n=3)

    write_trace_into_file_exactly(
        vector_of_points=trace_of_mars_satt, 
        speed_vec=speed_vec_of_mars_satt, 
        alltime=simu_time, 
        n=4)

    write_trace_into_file_exactly(
        vector_of_points=trace_of_relaysattelite1, 
        speed_vec=smooth_speed_vec(trace_of_relaysattelite1, 80) , 
        alltime=simu_time, 
        n=5)

    write_trace_into_file_exactly(
        vector_of_points=trace_of_relaysattelite2, 
        speed_vec=smooth_speed_vec(trace_of_relaysattelite2, 60) , 
        alltime=simu_time, 
        n=6)

    write_trace_into_file_exactly(
        vector_of_points=trace_of_relaysattelite3, 
        speed_vec=smooth_speed_vec(trace_of_relaysattelite3, 50) , 
        alltime=simu_time, 
        n=7)

    write_trace_into_file_exactly(
        vector_of_points=trace_of_relaysattelite4, 
        speed_vec=smooth_speed_vec(trace_of_relaysattelite4, 70) , 
        alltime=simu_time, 
        n=8)

    write_trace_into_file_exactly(
        vector_of_points=trace_of_relaysattelite5, 
        speed_vec=smooth_speed_vec(trace_of_relaysattelite5, 80) , 
        alltime=simu_time, 
        n=9)
    #write_trace_into_file(node7_trace, [10], 1000, 5, 6)
    # ====================
    print_in_3d_group([
        planet_trace_of_earth, 
        planet_trace_of_sun,
        planet_trace_of_moon,
        planet_trace_of_mars,
        planet_trace_of_outergod,
        trace_of_relaysattelite,
        trace_of_relaysattelite1,
        trace_of_relaysattelite2,
        trace_of_relaysattelite3,
        trace_of_relaysattelite4,
        trace_of_relaysattelite5,
        trace_of_mars_satt,
        ])
    g_trace_file.close()
#= def test07_functon() :

#= 
def test08_function() :
    print("python version:" + sys.version)
    global g_precise, g_trace_file 
    g_precise = 0.00001 
    g_trace_file = open(get_path_suffix_of('ns3-dtn-bit') + "/box/current_trace/current_trace.tcl", "w")
    simu_frame_point = 1000
    simu_time = 2000
    g_original_point = np.array([10000, 10000, 10000])
    ox = g_original_point[0]
    oy = g_original_point[1]
    oz = g_original_point[2]
    def movelittle(point, m=10):
        return [point[0] + m, point[1] + m, point[2] + m]
    # end def movelittle(point):
    #===============
    # maxrange = 1500;  N pkts during 0~900 s
    trace_0_1 = [ox+300, oy+300, oz]
    trace_1_1 = [ox+1300, oy+1300, oz]
    trace_4_1 = [ox+6700, oy+6700, oz]
    trace_2_1 = [ox+7700, oy+7700, oz]
    trace3_1 = [ox+1300, oy+2700, oz]
    trace3_2 = [ox+1300, oy+3900, oz]
    trace3_3 = [ox+1300, oy+4900, oz]
    trace5_1 = [ox+2700, oy+1300, oz]
    trace5_2 = [ox+2900, oy+1300, oz]
    trace5_3 = [ox+3300, oy+1300, oz]
    trace6_1 = [ox+4700, oy+1300, oz]
    trace6_2 = [ox+7300, oy+1300, oz]
    trace6_3 = [ox+7700, oy+5700, oz]
    trace7_1 = [ox+3200, oy+2400, oz]
    trace7_2 = [ox+3200, oy+3100, oz]
    trace7_3 = [ox+3300, oy+3300, oz]
    trace8_1 = [ox+4200, oy+4200, oz]
    trace8_2 = [ox+5000, oy+5000, oz]
    trace8_3 = [ox+5700, oy+5700, oz]
    trace9_1 = [ox+1300, oy+6300, oz]
    trace9_2 = [ox+3700, oy+7200, oz]
    trace9_3 = [ox+5700, oy+7700, oz]
# node 00
    trace_of_node_00 = [trace_0_1, movelittle(trace_0_1),]
    time_points_of_node00 = [0, 2000]
    speed_vec_of_node00 = calculate_speed_vec_from(trace_of_node_00, time_points_of_node00)
# node 00

# node 01
    trace_of_node_01 = [trace_1_1, movelittle(trace_1_1),]
    time_points_of_node01 = [0, 2000]
    speed_vec_of_node01 = calculate_speed_vec_from(trace_of_node_01, time_points_of_node01)
# node 01

# node 02
    trace_of_node_02 = [trace_2_1, movelittle(trace_2_1),]
    time_points_of_node02 = [0, 2000]
    speed_vec_of_node02 = calculate_speed_vec_from(trace_of_node_02, time_points_of_node02)
# node 02

# node 03
    trace_of_node_03 = [trace3_2, movelittle(trace3_2),
    trace3_1, movelittle(trace3_1), trace3_2, 
    trace3_3, movelittle(trace3_3), trace3_2, 
    trace3_1, movelittle(trace3_1), trace3_2, 
    trace3_3, movelittle(trace3_3), trace3_2, 
    trace3_1, movelittle(trace3_1), trace3_2, 
    trace3_3, movelittle(trace3_3), trace3_2, 
    trace3_1, movelittle(trace3_1), trace3_2, 
    trace3_3, movelittle(trace3_3), trace3_2, 
    trace3_1, movelittle(trace3_1), trace3_2, 
    trace3_3, movelittle(trace3_3), trace3_2, 
    ]
    time_points_of_node03 = [0, 30,
        62, 62+51, 160,
        200, 200+33, 299,
        412, 412+33, 489,
        581, 581+21, 650,
        870, 870+22, 920,
        1120, 1120+31, 1239,
        1399, 1399+20, 1440,
        1490, 1490+18, 1530,
        1589, 1589+20, 1700,
        1790, 1790+18, 2000]
    speed_vec_of_node03 = calculate_speed_vec_from(trace_of_node_03, time_points_of_node03)
# node 03

# node 04
    trace_of_node_04 = [trace_4_1, movelittle(trace_4_1),]
    time_points_of_node04 = [0, 2000]
    speed_vec_of_node04 = calculate_speed_vec_from(trace_of_node_04, time_points_of_node04)
# node 04

# node 05
    trace_of_node_05 = [trace5_2, movelittle(trace5_2),
    trace5_1, movelittle(trace5_1), trace5_2, 
    trace5_3, movelittle(trace5_3), trace5_2, 
    trace5_1, movelittle(trace5_1), trace5_2, 
    trace5_3, movelittle(trace5_3), trace5_2, 
    trace5_1, movelittle(trace5_1), trace5_2, 
    trace5_3, movelittle(trace5_3), trace5_2, 
    ]
    time_points_of_node05 = [0, 130,
    166, 166+28, 220,
    256, 256+31, 300,
    366, 366+38, 500,
    696, 696+28, 740,
    846, 846+28, 900,
    1056, 1056+28, 2000,
    ]
    speed_vec_of_node05 = calculate_speed_vec_from(trace_of_node_05, time_points_of_node05)
# node 05

# node 06
    trace_of_node_06 = [trace6_2, movelittle(trace6_2),
        trace6_1, movelittle(trace6_1), trace6_2, 
        trace6_3, movelittle(trace6_3), trace6_2, 
        trace6_1, movelittle(trace6_1), trace6_2, 
        trace6_3, movelittle(trace6_3), trace6_2, 
        trace6_1, movelittle(trace6_1), trace6_2, 
        trace6_3, movelittle(trace6_3), trace6_2, 
        ]
    time_points_of_node06 = [0, 200,
        256, 256+31, 400, 
        620, 620+41, 680, 
        696, 696+28, 750, 
        920, 920+41, 1000, 
        1056, 1056+28, 1200, 
        1320, 1320+41, 2000]
    speed_vec_of_node06 = calculate_speed_vec_from(trace_of_node_06, time_points_of_node06)
# node 06

# node 07
    trace_of_node_07 = [trace7_2, movelittle(trace7_2),
        trace7_1, movelittle(trace7_1), trace7_2, 
        trace7_3, movelittle(trace7_3), trace7_2, 
        trace7_1, movelittle(trace7_1), trace7_2, 
        trace7_3, movelittle(trace7_3), trace7_2, 
        ]
    time_points_of_node07 = [0, 400,
        479, 479 + 24, 550,
        629, 629 + 20, 800,
        920, 920 + 24, 1100,
        1262, 1262 + 20, 2000, ]
    speed_vec_of_node07 = calculate_speed_vec_from(trace_of_node_07, time_points_of_node07)
# node 07

# node 08
    trace_of_node_08 = [trace8_2,  movelittle(trace8_2),
    trace8_1, movelittle(trace8_1), trace8_2,
    trace8_3, movelittle(trace8_3), trace8_2,
    trace8_1, movelittle(trace8_1), trace8_2,
    trace8_3, movelittle(trace8_3), trace8_2,]
    time_points_of_node08 = [0, 530,
        622, 622+40, 780,
        1020, 1020+40, 1120,
        1262, 1262+40, 1400,
        1529, 1528+22, 2000]
    speed_vec_of_node08 = calculate_speed_vec_from(trace_of_node_08, time_points_of_node08)
# node 08

# node 09
    trace_of_node_09 = [trace9_2,  movelittle(trace9_2),
    trace9_1, movelittle(trace9_1), trace9_2, 
    trace9_3, movelittle(trace9_3), trace9_2, 
    trace9_1, movelittle(trace9_1), trace9_2, 
    trace9_3, movelittle(trace9_3), trace9_2, 
    trace9_1, movelittle(trace9_1), trace9_2, 
    trace9_3, movelittle(trace9_3), trace9_2, 
    trace9_1, movelittle(trace9_1), trace9_2, 
    trace9_3, movelittle(trace9_3), trace9_2, 
    trace9_1, movelittle(trace9_1), trace9_2, 
    trace9_3, movelittle(trace9_3), trace9_2, 
    ]
    time_points_of_node09 = [0, 150,
        200, 200+33, 300,
        400, 400+21, 488,
        581, 581+21, 610,
        690, 690+44, 888,
        1120, 1120+31, 1200,
        1314, 1314+17, 1420,
        1490, 1490+18, 1550,
        1604, 1604+30, 1740,
        1790, 1790+18, 1840,
        1904, 1904+30, 2000,]
    speed_vec_of_node09 = calculate_speed_vec_from(trace_of_node_09, time_points_of_node09)
# node 09

    write_trace_into_file_exactly(
        vector_of_points=trace_of_node_00, 
        speed_vec=speed_vec_of_node00, 
        alltime=simu_time, 
        n=0)
    write_trace_into_file_exactly(
        vector_of_points=trace_of_node_01, 
        speed_vec=speed_vec_of_node01, 
        alltime=simu_time, 
        n=1)
    write_trace_into_file_exactly(
        vector_of_points=trace_of_node_02, 
        speed_vec=speed_vec_of_node02, 
        alltime=simu_time, 
        n=2)
    write_trace_into_file_exactly(
        vector_of_points=trace_of_node_03, 
        speed_vec=speed_vec_of_node03, 
        alltime=simu_time, 
        n=3)
    write_trace_into_file_exactly(
        vector_of_points=trace_of_node_04, 
        speed_vec=speed_vec_of_node04, 
        alltime=simu_time, 
        n=4)
    write_trace_into_file_exactly(
        vector_of_points=trace_of_node_05, 
        speed_vec=speed_vec_of_node05, 
        alltime=simu_time, 
        n=5)
    write_trace_into_file_exactly(
        vector_of_points=trace_of_node_06, 
        speed_vec=speed_vec_of_node06, 
        alltime=simu_time, 
        n=6)
    write_trace_into_file_exactly(
        vector_of_points=trace_of_node_07, 
        speed_vec=speed_vec_of_node07, 
        alltime=simu_time, 
        n=7)
    write_trace_into_file_exactly(
        vector_of_points=trace_of_node_08, 
        speed_vec=speed_vec_of_node08, 
        alltime=simu_time, 
        n=8)
    write_trace_into_file_exactly(
        vector_of_points=trace_of_node_09, 
        speed_vec=speed_vec_of_node09, 
        alltime=simu_time, 
        n=9)
# end def test08_function() :
#======================
#
#
#
#
#======================
assert(dist_of([1000,1000,1000],[2000,1000,1000]) == 1000)
random.seed()
test08_function()
#sympy_test()