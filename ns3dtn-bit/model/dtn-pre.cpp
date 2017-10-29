#include "dtn-pre.h"

namespace ns3 {
    namespace ns3dtnbit {

#ifdef DEBUG
        string GetCallStack(int i = 2) {
            int nptrs;
            void *buffer[200];
            char **cstrings;
            char* return_str = new char[200];

            nptrs = backtrace(buffer, 200);
            sprintf(return_str, "backtrace() returned %d addresses\n", nptrs);
            /* The call backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO)
             *               would produce similar output to the following: */

            cstrings = backtrace_symbols(buffer, nptrs);
            if (cstrings == NULL || nptrs < 3) {
                perror("backtrace_symbols");
                exit(EXIT_FAILURE);
            }
            sprintf(return_str, "%s\n", cstrings[i]);
            free(cstrings);
            return return_str;
        }

        string FilePrint(string str) {
            std::stringstream ss;
            char* cs = new char[200];
            std::sprintf(cs, "file : %s, line : %d,", __FILE__, __LINE__);
            ss << "====== FilePrint ===== " << cs << "--->>" << str << endl;
            return ss.str();
        }

        string GetLogStr(string str) {
            std::stringstream ss;
            string caller = GetCallStack();
            ss << "==== Caller ====" << caller << "\n---->>" << str << endl;
            string Filep = FilePrint(ss.str());
            return Filep;
        }

        bool operator<(CgrXmit const & lhs, CgrXmit const & rhs) {return lhs.contact_start_time_ < rhs.contact_start_time_;}
#endif
    } /* ns3dtnbit */ 

} /* ns3  */ 

namespace ns3 {
    namespace ns3dtnbit {
        void Adob::AdobDo_01(std::map<int, vector<vector<int>>> t_2_adjacent_array, int node_number) {
            // boost code
            using namespace boost;
            node_number_ = node_number;
            int t = -1;
            for (auto t2 : t_2_adjacent_array) {
                t++;
                auto time_index = get<0>(t2);
                auto t3 = get<1>(t2);
                Graph my_g;
                vector<VeDe> vec_vertex_des;
                unordered_map<int, VeDe> tmp_m;
                // add node & node NameProperties
                for (int i = 0; i < node_number; i++) {
                    std::stringstream ss;
                    ss << "node-" << i;
                    auto tmp_vd = add_vertex(VertexProperties(ss.str()), my_g);
                    vec_vertex_des.push_back(tmp_vd);
                    tmp_m[i] = tmp_vd;
                }
                //auto vec_edge_des = std::vector<vector<EdDe>>(node_number_, std::vector<EdDe>(node_number_, EdDe()));
                for (int i = 0; i < node_number; ++i) {
                    for (int j = i; j < node_number; ++j) {
                        if (i == j) {continue;}
                        add_edge(vec_vertex_des[i], vec_vertex_des[j], EdgeProperties(t3[i][j], 1), my_g);
                        add_edge(vec_vertex_des[j], vec_vertex_des[i], EdgeProperties(t3[i][j], 1), my_g);
                    }
                }
                assert(boost::num_edges(my_g)>0);
                assert(boost::num_vertices(my_g)>0);
                // load it
                t_vec_.push_back(time_index);
                g_vec_.push_back(my_g);
                g_vede_m_.push_back(tmp_m);
            }
        }

        void Adob::AdobDo_02(int node_number, int teg_layer_n, int max_range) {
            max_range_ = max_range;
            // add all node to it node-n-t-i
            using namespace boost;
            for (int n = 0; n < node_number; n++) {
                for (int t = 0; t < teg_layer_n; t++) {
                    stringstream ss;
                    ss << "node-" << n << "-" << t;
                    VeDe tmp_d = add_vertex(VertexProperties(ss.str()), teg_);
                    name2vd_map[ss.str()] = tmp_d;
                }
            }
            // add temporal link
            // g_vec_ is vector of static graph
            const int hypothetic_distance_of_temporal_link = max_range / 20;    // privent that message keep on one node all the time
            for (int t = 0; t < teg_layer_n - 1; t++) {
                for (int n = 0; n < node_number; n++) {
                    stringstream ss;
                    ss << "node-" << n << "-" << t;
                    auto name_0 = ss.str();
                    ss.str("");
                    ss << "node-" << n << "-" << t + 1;
                    auto name_1 = ss.str();
                    auto vd_0 = name2vd_map[name_0];
                    auto vd_1 = name2vd_map[name_1];
                    auto tmp_ed = add_edge(vd_0, vd_1, EdgeProperties(hypothetic_distance_of_temporal_link, 0), teg_);
                }
            }
            // for each layer add transmit link if distance_ of link 'a' of static upper layer graph is under max_range 
            // and the distance of link 'b' of static lower layer graph is also under max_range
            assert(g_vec_.size() >= teg_layer_n);
            // assume that g_vede_m_ == teg_layer_n 
            for (int t = 0; t < teg_layer_n - 1; t++) {
                Graph& tmp_g = g_vec_[t];
                Graph& tmp_g_other = g_vec_[t + 1];
                for (int i = 0; i < node_number; i++) {
                    for (int j = i; j < node_number; j++) {
                        if (i == j) {continue;}
                        auto i_d = g_vede_m_[t][i];
                        auto j_d = g_vede_m_[t][j];
                        auto e_p = edge(i_d, j_d, tmp_g);
                        auto i_d_other = g_vede_m_[t + 1][i];
                        auto j_d_other = g_vede_m_[t + 1][j];
                        auto e_p_other = edge(i_d_other, j_d_other, tmp_g_other);
                        if (e_p.second && e_p_other.second) {
                            auto ed = e_p.first;
                            auto ed_other = e_p_other.first;
                            if (tmp_g[ed].distance_ < max_range && tmp_g_other[ed_other].distance_ < max_range) {
                                string ix_name = "node-" + to_string(i) + "-" + to_string(t);
                                string jx_name = "node-" + to_string(j) + "-" + to_string(t);
                                string iy_name = "node-" + to_string(i) + "-" + to_string(t + 1);
                                string jy_name = "node-" + to_string(j) + "-" + to_string(t + 1);
                                auto tmp_id_of_g = name2vd_map[ix_name];
                                auto tmp_jd_of_g = name2vd_map[jx_name];
                                auto tmp_id_of_g_other = name2vd_map[iy_name];
                                auto tmp_jd_of_g_other = name2vd_map[jy_name];

                                auto tmp_edge_of_g = add_edge(tmp_id_of_g, tmp_jd_of_g_other, EdgeProperties(
                                            (tmp_g[ed].distance_ / 2) + (tmp_g_other[ed_other].distance_ / 2), 1), teg_);
                                auto tmp_edge_of_g_other = add_edge(tmp_jd_of_g, tmp_id_of_g_other, EdgeProperties(
                                            (tmp_g[ed].distance_ / 2) + (tmp_g_other[ed_other].distance_ / 2), 1), teg_);
                            }
                        } else {
                            std::cerr << "Error: can't acess edge" << " : line " << __LINE__ 
                                << " t=" << t << " i =" << i << " j =" << j 
                                << "size of g_vec_ =" << g_vec_.size()
                                << "size of g_vede_m_= " << g_vede_m_.size()
                                << std::endl;
                            std::abort();
                        }
                    }
                }
            }
        }

        void Adob::AdobDo_03() {
            assert(get_teg_size() > get_g_vec_size() * get_node_number());
            string teg_viz_filename = root_path + "/box/dtn_simulation_result/teg_viz.txt";
            // get round time 
            int rounded_time = Simulator::Now().GetSeconds();
            int t_max = g_vec_.size();
            //DelayMap delay_map_;

            std::cout << "NOTE:in AdobDo_03, before initialize delay_map_" << std::endl;
            // Initialize delay_map_
            const int c = hypo_c; // assume every data transmit would cost one unit of time
            for (int t = t_max - 2; t >= 0; t--) {
                for (int i = 0; i < get_node_number(); i++) {
                    for (int j = 0; j < get_node_number(); j++) {
                        string v_i_t_name = "node-" + to_string(i) + "-" + to_string(t);
                        string v_j_t_plus_c_name = "node-" + to_string(j) + "-" + to_string(t + c);
                        auto ep = edge(name2vd_map[v_i_t_name], name2vd_map[v_j_t_plus_c_name], teg_);
                        if (ep.second) {
                            int edge_delay_color = teg_[ep.first].message_color_;
                            DelayIndex tmp_dl = make_tuple(i, j, t, edge_delay_color);
                            delay_map_[tmp_dl] = edge_delay_color;
                        } else {
                            DelayIndex tmp_dl = make_tuple(i, j, t + 1, c);
                            DelayIndex tmp_dl_x = make_tuple(i, j, t, c);
                            auto found = delay_map_.find(tmp_dl);
                            if (found != delay_map_.end()) {
                                if (delay_map_[tmp_dl] < NS3DTNBIT_HYPOTHETIC_INFINITE_DELAY) {
                                    delay_map_[tmp_dl_x] = delay_map_[tmp_dl] + 1;
                                } else {
                                    delay_map_[tmp_dl_x] = NS3DTNBIT_HYPOTHETIC_INFINITE_DELAY;
                                }
                            } else {
                                delay_map_[tmp_dl_x] = NS3DTNBIT_HYPOTHETIC_INFINITE_DELAY;
                            }
                        }
                    }
                }
            }
            std::cout << "NOTE:in AdobDo_03, before shortest delay path" << std::endl;
            // shortest delay path
            for (int k = 0; k < get_node_number(); k++) {
                for (int i = 0; i < get_node_number(); i++) {
                    for (int j = 0; j < get_node_number(); j++) {
                        for (int t = 0; t < t_max - 2; t++) { // it's not meaning to set delay_map_ for t_max - 1, this is one difference from that paper
                            DelayIndex di_cur = make_tuple(i, j, t, c), di_to_k = make_tuple(i, k, t, c);
                            int delay_cur = delay_map_[di_cur];
                            int delay_to_k = delay_map_[di_to_k];
                            DelayIndex di_from_k = make_tuple(k, j, t + delay_to_k, c);
                            auto found = delay_map_.find(di_from_k);
                            if (found != delay_map_.end()) {
                                int delay_from_k = delay_map_[di_from_k];
                                int sum = delay_to_k + delay_from_k;
                                if (sum < delay_cur) {
                                    delay_map_[di_cur] = sum;
                                    auto tmp_tuple = make_tuple(i, j, t);
                                    teg_routing_table_[tmp_tuple] = k;
                                }
                            } else {
                                // maybe t + delay_to_k > T_max
                            }
                        }
                    }
                }
            }
            std::cout << "NOTE: write viz for teg" << std::endl;
            ofstream dot(teg_viz_filename);
            using EdgeProperties = ns3::ns3dtnbit::Adob::EdgeProperties;
            using VertexProperties = ns3::ns3dtnbit::Adob::VertexProperties;
            boost::write_graphviz(dot, teg_, 
                    boost::make_label_writer(boost::get(&VertexProperties::name_, teg_)),
                    boost::make_edge_writer(boost::get(&EdgeProperties::distance_, teg_), boost::get(&EdgeProperties::message_color_, teg_)));
            std::cout << "NOTE:in AdobDo_03, after shortest delay path" << std::endl;
        }

        /*
         * */ 
        void Adob::AdobDo_04() {
            // init node_id2cgr_xmit_vec_map_ TODO
            cout << "DEBUG_CGR " << "in AdobDo_04" << endl;
            int node_number = get_node_number();
            int time_max = get_g_vec_size();
            for (int d = 0; d < node_number; ++d) {
                vector<CgrXmit> vec_of_xmits;
                for (int s = 0; s < node_number; ++s) {
                    if (d == s) {continue;}
                    bool no_more_xmits = false;
                    int time_cur = 0;
                    while (!no_more_xmits) {
                        dtn_time_t contact_start_time, contact_end_time;
                        int node_id_of_to = d, node_id_of_from = s;
                        double data_transmission_rate = 80000;
                        bool found_xmit = false;
                        {
                            // init -> 0, when find available one, -> 1, when available one become not available -> 2
                            int link_state = 0;
                            for (int t = time_cur; t < time_max && link_state != 2; ++t) {
                                if (t == time_max - 1) {no_more_xmits = true;}
                                auto g_t = g_vec_[t];
                                auto g_vd_m = g_vede_m_[t];
                                auto s_descriptor = g_vd_m[s];
                                auto d_descriptor = g_vd_m[d];
                                auto ep = edge(s_descriptor, d_descriptor, g_t);
                                if (ep.second) {
                                    int dist = g_t[ep.first].distance_;
                                    if (dist < max_range_ && link_state == 0) {
                                        contact_start_time = t;
                                        link_state = 1;
                                    } else if ((dist > max_range_ || t == (time_max - 1)) && link_state == 1) {
                                        contact_end_time = t;
                                        link_state = 2;
                                        found_xmit = true;
                                        time_cur = t;
                                        break;
                                    }
                                } else {
                                    cout << "Error:" << __LINE__ << " can't be, s =" << s << ";d =" << d << ";t=" << t << endl;
                                    std::abort();
                                }
                            }
                        }
                        if (found_xmit) {
                            CgrXmit cgr_xmit_obj = {
                                contact_start_time,
                                contact_end_time,
                                node_id_of_from,
                                node_id_of_to,
                                data_transmission_rate
                            };
                            vec_of_xmits.push_back(cgr_xmit_obj);
                        }
                    }
                }
                if (vec_of_xmits.size() == 0) { cout << "DEBUG_CGR_WARN : node have no xmit" << ";node-" << d << endl; }
                if (vec_of_xmits.size() > 0) { cout << "DEBUG_CGR_DEBUG : node have " << vec_of_xmits.size() << " xmits" << ";node-" << d << endl; }
                // sort them
                sort(vec_of_xmits.begin(), vec_of_xmits.end(), [](const CgrXmit& a, const CgrXmit& b) {
                        return a.contact_start_time_ < b.contact_start_time_;
                        });
                if (vec_of_xmits[0].contact_start_time_ >= vec_of_xmits[1].contact_start_time_) {
                    for (auto xmit : vec_of_xmits) {
                        cout << xmit.contact_start_time_ << endl;
                    }
                    assert(vec_of_xmits[0].contact_start_time_ <= vec_of_xmits[1].contact_start_time_);
                }
                node_id2cgr_xmit_vec_map_[d] = vec_of_xmits;
            }
        }

        Adob::Adob() {}
        Adob::~Adob() {}

        int Adob::get_node_number() { return node_number_; }
        int Adob::get_teg_size() { return num_edges(teg_); }
        int Adob::get_g_vec_size() { return g_vec_.size(); }

        Adob::Graph Adob::get_graph_for_now() const {
            for (int i = t_vec_.size() - 1; i >= 0 ; i--) {
                if (Simulator::Now().GetSeconds() >= t_vec_[i]) {
                    auto g_re = g_vec_[i];
                    return g_re;
                }
            }
            std::cout << "Error:" <<__LINE__ << "can't be, Seconds =" << Simulator::Now().GetSeconds() << "t_vec_" << t_vec_[0] << std::endl;
            std::abort();
        }
    } /* ns3dtnbit */ 
} /* ns3  */ 

