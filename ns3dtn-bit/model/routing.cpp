#include "routing.h"

namespace ns3 {
    namespace ns3dtnbit {

        YouRouting::YouRouting(DtnApp& dp) : RoutingMethodInterface(dp) {}

        int YouRouting::DoRoute(int s, int d) {
            auto foundsx = find_if(routed_table_.begin(), routed_table_.end(), [this](tuple<node_id_t, node_id_t, dtn_seqno_t> rtele){ if (get<2>(rtele) == debug_that_seqno_) { return true; } else { return false; } });
            if (foundsx != routed_table_.end()) {
                return get<1>(*foundsx);
            } else {
                int r = DoRouteDetail(s, d);
                auto tmppp = make_tuple(d, r, debug_that_seqno_);
                routed_table_.push_back(tmppp);
                return r;
            }
        }

        int YouRouting::DoRouteDetail(int s, int d) {
            using namespace boost;
            const Adob& ref_adob = RoutingMethodInterface::get_adob();
            auto g = ref_adob.get_graph_for_now();

            using Graph_T = decltype(g);
            using Vertex_D = boost::graph_traits<Graph_T>::vertex_descriptor;
            using Edge_D = boost::graph_traits<Graph_T>::edge_descriptor;

            Vertex_D s_des, d_des;
            {
                // get vertex_descriptor
                std::stringstream ss1;
                ss1 << "node-" << s;
                string s_str = ss1.str();
                std::stringstream ss0;
                ss0 << "node-" << d;
                string d_str = ss0.str();
                boost::graph_traits<Graph_T>::vertex_iterator vi, vi_end;
                for(boost::tie(vi, vi_end) = boost::vertices(g); vi != vi_end; ++vi){
                    if (g[*vi].name_ == s_str) {
                        s_des = *vi;
                    } else if (g[*vi].name_  == d_str) {
                        d_des = *vi;
                    }
                }
            }

            std::vector<Vertex_D> predecessor(boost::num_vertices(g));
            std::vector<int> distances(boost::num_vertices(g));

            dijkstra_shortest_paths(g, s_des,
                    weight_map(get(&my_edge_property::distance_, g)).
                    distance_map(make_iterator_property_map(distances.begin(), get(vertex_index, g))).
                    predecessor_map(make_iterator_property_map(predecessor.begin(), get(vertex_index, g)))
                    );
            Vertex_D cur = d_des;
            int count = num_vertices(g);
            while (predecessor[cur] != s_des && count-- > 0) {
                cur = predecessor[cur];
            }
            if (count <= 0) {
                std::cout << "fuckhere, you can't find the next hop, print predecessor and abort()" << std::endl;
                for (auto v : predecessor) {
                    std::cout << " v=" << v << ";";
                }
                std::cout << endl;
            }
            std::cout << "fuckhere , s_dex =" << s_des << "; d_des=" << d_des <<  ";predecessor[cur]=" << predecessor[cur] << ";cur=" << cur << std::endl;
            int result = (int)cur;
            return result;
        }

        void YouRouting::GetInfo(node_id_t destination_id, node_id_t from_id, std::vector<node_id_t> vec_of_current_neighbor, node_id_t own_id, dtn_time_t expired_time, int bundle_size, int networkconfigurationflag, map<int, vector<int>> id2cur_exclude_vec_of_id, dtn_time_t local_time, dtn_seqno_t that_seqno) {
            debug_that_seqno_ = that_seqno;
        }

        TegRouting::TegRouting(DtnApp& dp) : RoutingMethodInterface(dp) {}

        void TegRouting::GetInfo(node_id_t destination_id, node_id_t from_id, std::vector<node_id_t> vec_of_current_neighbor, node_id_t own_id, dtn_time_t expired_time, int bundle_size, int networkconfigurationflag, map<int, vector<int>> id2cur_exclude_vec_of_id, dtn_time_t local_time, dtn_seqno_t that_seqno) {
            debug_that_seqno_ = that_seqno;
        }

        int TegRouting::DoRouteDetail(int s, int d) {
            using namespace boost;
            Adob adob = RoutingMethodInterface::get_adob();
            Adob& ref_adob = adob;
            using Graph_T = Adob::Graph;
            using VeDe_T = Adob::VeDe;
            using EdDe_T = Adob::EdDe;
            using DelayIndex = Adob::DelayIndex;
            using RoutingTableIndex = Adob::Teg_i_j_t;
            const int c = Adob::hypo_c;

            int tmp_time = Simulator::Now().GetSeconds();
            int time_max = ref_adob.t_vec_.size();
            RoutingTableIndex rt_index = make_tuple(s, d, tmp_time);
            DelayIndex d_index = make_tuple(s, d, tmp_time, c);

            if (ref_adob.delay_map_[d_index] > time_max - tmp_time) {
                std::cout << "WARN:this routing is not possible" << __LINE__ << ":" 
                    << "\nremain time= " << time_max - tmp_time 
                    << "\ndelay time= " << ref_adob.delay_map_[d_index] 
                    << std::endl; return -1;
            } else {
                int vk = -1;
                bool found_vk = false;
                int finding_count = 0;
                while (!found_vk && finding_count < 100) {
                    finding_count += 1;
                    auto found = ref_adob.teg_routing_table_.find(rt_index);
                    if (found != ref_adob.teg_routing_table_.end()) {
                        vk = ref_adob.teg_routing_table_[rt_index];
                    } else {
                        vk = std::get<1>(rt_index);
                        found_vk = true;
                    }
                    if (vk < 0 || vk > ref_adob.get_node_number()) { std::cout << "Error: can't be, vk=" << vk << "file,line:" << __FILE__ << __LINE__ << std::endl; std::abort(); }
                    rt_index = make_tuple(s, vk, tmp_time);
                }
                if (finding_count > 99) {
                    cout << "error: finding routing table too much time, abort()" << endl;
                    std::abort();
                }
                if (vk == -1) { std::abort(); }
                return vk;
            }
        }

        int TegRouting::DoRoute(int s, int d) {
            auto foundsx = find_if(routed_table_.begin(), routed_table_.end(), [this](tuple<node_id_t, node_id_t, dtn_seqno_t> rtele){ if (get<2>(rtele) == debug_that_seqno_) { return true; } else { return false; } });
            if (foundsx != routed_table_.end()) {
                return get<1>(*foundsx);
            } else {
                int r = DoRouteDetail(s, d);
                auto tmppp = make_tuple(d, r, debug_that_seqno_);
                routed_table_.push_back(tmppp);
                return r;
            }
        }

        CGRRouting::CGRRouting(DtnApp& dp) : RoutingMethodInterface(dp) {
#ifdef CGR_DEBUG_0
            cgr_debug_flag_0 = true;
#endif
#ifdef CGR_DEBUG_1
            cgr_debug_flag_1 = true;
#endif
        }

        int CGRRouting::DoRoute(int s, int d) {
            cout << "DEBUG_CGR:" << " ============== A new DoRoute ==========\n" << endl;
            Init();
            assert(d == destination_id_);
            assert(s == own_id_);
            auto founds = find(exhausted_search_target_list_.begin(), exhausted_search_target_list_.end(), make_pair(d, debug_cgr_that_seqno_));
            auto foundsx = find_if(routed_table_.begin(), routed_table_.end(), 
                    [this](tuple<node_id_t, dtn_time_t, dtn_time_t, node_id_t, dtn_seqno_t, dtn_time_t> rtele){
                    // get<0>(rtele) == destination_id_ 
                    //&& (get<4>(rtele) == debug_cgr_that_seqno_ ||  (local_time_ - get<5>(rtele)) < (NS3DTNBIT_BUFFER_CHECK_INTERVAL * 15)) 
                    //&& (get<1>(rtele) - (NS3DTNBIT_BUFFER_CHECK_INTERVAL * 3)) > local_time_) 
                    if ( get<0>(rtele) == destination_id_
                            && (get<1>(rtele) - NS3DTNBIT_BUFFER_CHECK_INTERVAL * 2) > local_time_
                       ) { return true; } else { return false; }
                    });
            bool reuse_flag = NS3DTNBIT_CGR_OPTIMAL_OPTION;
            if (founds != exhausted_search_target_list_.end() && reuse_flag) {
                cout << "DEBUG_CGR:" << "WARN:may make bad routing-1. this dest is searched before, and no available one is found, " 
                    << "so this time it would exhausted search possiblity too. we would return a -1" 
                    << " .This won't affect routing result, but can reduce simulation time." 
                    << ";local_time_ = " << local_time_ 
                    << ";debug_cgr_that_seqno_ = " << debug_cgr_that_seqno_
                    << endl;
                return -1;
            } else if (foundsx != routed_table_.end() && reuse_flag) {
                tuple<node_id_t, dtn_time_t, dtn_time_t, node_id_t, dtn_seqno_t, dtn_time_t> tmpfsx = *foundsx;
                cout << "DEBUG_CGR:" << "WARN:may make bad routing-2. reuse routing result, reduce routing time" 
                    << ";local_time_ = " << local_time_ 
                    << ";debug_cgr_that_seqno_ = " << debug_cgr_that_seqno_
                    << ";reused result = " << get<3>(tmpfsx) 
                    << endl;
                return get<3>(tmpfsx);
            }
            ContactReviewProcedure(destination_id_, forfeit_time_, best_delivery_time_);
            cout << "DEBUG_CGR:" << "before ForwardDecision()" 
                << "BundleTrace:entertimes=" << debug_crp_enter_count_ << "times" << endl;
            int result_index = ForwardDecision();
            int result = result_index >= 0 ? proximate_vec_[result_index] : -1;
            if (debug_cgr_this_exhausted_search_not_found_) {
                exhausted_search_target_list_.push_back(make_pair(d, debug_cgr_that_seqno_));
            }
            if (result != -1) {
                tuple<node_id_t, dtn_time_t, dtn_time_t, node_id_t, dtn_seqno_t, dtn_time_t> tmprvec = 
                    make_tuple(
                            destination_id_, 
                            final_forfeit_time_[result_index], 
                            final_best_delivery_time_[result_index], 
                            result, 
                            debug_cgr_that_seqno_, 
                            local_time_);
                assert(local_time_ < final_forfeit_time_[result_index]);
                if (cgr_debug_flag_0) {
                    cout << "CGR_DEBUG:"
                        << "push one route result"
                        << ";destination_id_=" << destination_id_
                        << ";forfeit_time_=" << final_forfeit_time_[result_index]
                        << ";final_best_delivery_time_=" << final_best_delivery_time_[result_index]
                        << ";result=" << result
                        << ";debug_cgr_that_seqno_=" << debug_cgr_that_seqno_
                        << ";local_time_=" << local_time_ << endl;
                }
                routed_table_.push_back(tmprvec);
            }
            assert(result < 100);
            cout << "DEBUG_CGR:"  << " ================== end this DoRoute() =========\n"
                << "result= " << result 
                << ";CGR-" << s << "->" << d << endl;
            return result;
        }

        void CGRRouting::GetInfo(node_id_t destination_id, node_id_t from_id, std::vector<node_id_t> vec_of_current_neighbor, node_id_t own_id, dtn_time_t expired_time, int bundle_size, int networkconfigurationflag, map<int, vector<int>> id2cur_exclude_vec_of_id, dtn_time_t local_time, dtn_seqno_t that_seqno) {
            swap(id_of_d2cur_excluded_vec_of_d_, id2cur_exclude_vec_of_id); // a more efficent assignment
            local_time_ = local_time;
            destination_id_ = destination_id;
            node_id_transmit_from_ = from_id;
            expired_time_ = expired_time;
            own_id_ = own_id;
            debug_cgr_that_seqno_ = that_seqno;
            ecc_ = bundle_size;
            id_of_current_neighbor_ = vec_of_current_neighbor;
            networkconfigurationflag_ = networkconfigurationflag;
        }

        void CGRRouting::Init() {
            debug_recurrsive_deep_ = 0;
            debug_crp_enter_count_ = 0;
            debug_cgr_this_exhausted_search_not_found_ = false;
            debug_node_access_count_map_ = map<int, int>();
            debug_recurrsive_path_stack_ = stack<int>();
            cgr_find_one_proximate_ = false;
            forfeit_time_ = expired_time_;
            best_delivery_time_ = local_time_;
            excluded_vec_ = vector<int>();
            proximate_vec_ = vector<int>();
            final_forfeit_time_.clear();
            final_best_delivery_time_.clear();
            auto cur_excluded = id_of_d2cur_excluded_vec_of_d_[destination_id_];
            for (auto nei : id_of_current_neighbor_) {
                auto found = find(cur_excluded.begin(), cur_excluded.end(), nei);
                if (found != cur_excluded.end()) {
                    excluded_vec_.push_back(nei);
                }
            }
            excluded_vec_.push_back(node_id_transmit_from_);
        }

        void CGRRouting::DebugPrintXmit(vector<CgrXmit>& cgr_xmit_vec_ref, int cur_d) {
            {
                cout << "DEBUG_CGR:" << " ====== for node-" << cur_d 
                    << " xmit is :" << endl;
                for (auto m : cgr_xmit_vec_ref) {
                    cout << "\n m ==> m.contact_start_time_ =" << m.contact_start_time_
                        << ";m.contact_end_time_=" << m.contact_end_time_
                        << ";m.node_id_of_from_=" << m.node_id_of_from_
                        << ";m.node_id_of_to_=" << m.node_id_of_to_
                        << ";m.data_transmission_rate_=" << m.data_transmission_rate_ << endl;
                }
                cout << "end of DEBUG_CGR" << endl;
            }

        }

        /*
         * Please read this link, if you want to know about this. https://tools.ietf.org/html/draft-burleigh-dtnrg-cgr-00
         *  ---- cur_d 
         *  ---- cur_deadline
         *  ---- proximate_vec_
         *  ---- excluded_vec_
         *  ---- forfeit_time_
         *  ---- best_delivery_time_
         * */
        void CGRRouting::ContactReviewProcedure(node_id_t cur_d, dtn_time_t cur_deadline, dtn_time_t best_deli) {
            debug_crp_enter_count_ += 1;
            assert(debug_crp_enter_count_ < 300);
            if (cgr_debug_flag_1) {
                debug_recurrsive_path_stack_.push(cur_d);
                debug_recurrsive_deep_ += 1;
                auto found = debug_node_access_count_map_.find(cur_d);
                if (found != debug_node_access_count_map_.end()) {
                    debug_node_access_count_map_[cur_d] += 1;
                } else {
                    debug_node_access_count_map_[cur_d] = 1;
                }
                if (debug_recurrsive_deep_ > 7) {
                    if (debug_crp_enter_count_ > 5000) {
                        cout << "Error: this CGR is missing in deep recurrsive, we would print all access map and abort, enter this function " << debug_crp_enter_count_ << " times, check readme.md to know what happens" << endl;
                        while (!debug_recurrsive_path_stack_.empty()) {
                            cout << "node-" << debug_recurrsive_path_stack_.top() << " to" << endl;
                            debug_recurrsive_path_stack_.pop();
                        }
                        for (auto mv : debug_node_access_count_map_) {
                            int idn = get<0>(mv);
                            int count = get<1>(mv);
                            cout << "node-" << idn << " access " << count << "times" << endl;
                        }
                        abort();
                    }
                }
            }
            if (cgr_find_one_proximate_) {
                if (cgr_debug_flag_1) { debug_recurrsive_deep_ -= 1; } 
                return;
            }
            // 1.
            excluded_vec_.push_back(cur_d);
            const Adob& ref_adob = RoutingMethodInterface::get_adob();
            auto& cgr_xmit_vec_ref = ref_adob.node_id2cgr_xmit_vec_map_[cur_d];
            if (cgr_debug_flag_0) {
                if (debug_cgr_that_seqno_ == NS3DTNBIT_CGR_DEBUG_SEQ_1 || debug_cgr_that_seqno_ == NS3DTNBIT_CGR_DEBUG_SEQ_2) {
                    cout << "CGR_DEBUG:temporary debug use, deleteme when you don't need me, xmitdebug-seqno-" << debug_cgr_that_seqno_ << __FILE__ << ":" <<  __LINE__ 
                        << ";own_id_ = " << own_id_ 
                        << ";local_time_ = " << local_time_ 
                        << endl;
                    DebugPrintXmit(cgr_xmit_vec_ref, cur_d);
                }
            }
            // 2.
            for (auto& m : cgr_xmit_vec_ref) {
                bool last_moment_check = 
                    local_time_ < cur_deadline - NS3DTNBIT_BUFFER_CHECK_INTERVAL * 2
                    && m.contact_start_time_ + NS3DTNBIT_BUFFER_CHECK_INTERVAL * 2< cur_deadline 
                    && local_time_ < m.contact_end_time_ - NS3DTNBIT_BUFFER_CHECK_INTERVAL * 2;
                dtn_time_t local_forfeit_time = cur_deadline;
                dtn_time_t local_best_delivery_time = best_deli;
                if (!last_moment_check) {
                    // 2.A
                    continue;
                } else {
                    // 2.B
                    node_id_t s = m.node_id_of_from_;
                    bool s_is_local_node_with_own_id = s == own_id_;
                    if (s_is_local_node_with_own_id) {
                        if (cgr_debug_flag_1) {
                            cout << "DEBUG_CGR:" << "in tail of recursive, and, if we find a proximate one,"
                                << "we would break the search, because the CGR algorithm in RFC would force to find all possible pathes,"
                                << " which is too big for some senario, mainly the group moving senario " << endl;
                        }
                        // 2.B.1
                        // compute ECC for this bundle
                        int ecc = ecc_;
                        assert(m.contact_end_time_ - m.contact_start_time_ < 5000);
                        // not accurate, just a hypothetic value,  TODO
                        int ecc_of_other_bundles = 1800;
                        int residual_capacity = ((m.contact_end_time_ - m.contact_start_time_) * m.data_transmission_rate_) - ecc_of_other_bundles; 
                        assert(residual_capacity > 10000);
                        auto found_1 = find(proximate_vec_.begin(), proximate_vec_.end(), cur_d);
                        bool d_is_in_proximate = found_1 != proximate_vec_.end();
                        if (residual_capacity < ecc) {
                            continue;
                        } else if (d_is_in_proximate) {
                            continue;
                        } else {
                            if (m.contact_end_time_ < local_forfeit_time) {
                                local_forfeit_time = m.contact_end_time_;
                            } 
                            if (m.contact_start_time_ > local_best_delivery_time) {
                                local_best_delivery_time = m.contact_start_time_;
                            }
                            // this line won't let searching end imediately, still would find the available one in this branch.
                            cgr_find_one_proximate_ = true;
                            // Note the computed forfeit time and best-case delivery time in the event that the bundle is queued for transmission to D.
                            assert(local_time_ < local_forfeit_time);
                            proximate_vec_.push_back(cur_d);
                            final_forfeit_time_.push_back(local_forfeit_time);
                            final_best_delivery_time_.push_back(local_best_delivery_time);
                            if (cgr_debug_flag_0) {
                                if (debug_cgr_that_seqno_ == NS3DTNBIT_CGR_DEBUG_SEQ_1 || debug_cgr_that_seqno_ == NS3DTNBIT_CGR_DEBUG_SEQ_2) {
                                    cout << "\nCGR_DEBUG:\nthis is the xmit we finally choose" 
                                        << ";forfeit_time_ = " << local_forfeit_time
                                        << ";cur_deadline = " << cur_deadline << endl;
                                    cout << "\n m ==> m.contact_start_time_ =" << m.contact_start_time_
                                        << ";m.contact_end_time_=" << m.contact_end_time_
                                        << ";m.node_id_of_from_=" << m.node_id_of_from_
                                        << ";m.node_id_of_to_=" << m.node_id_of_to_
                                        << ";m.data_transmission_rate_=" << m.data_transmission_rate_ << endl;
                                }
                            }
                        }
                    } else {
                        if (cgr_debug_flag_1) {
                            cout << "DEBUG_CGR:" << "in body of recursive" << endl;
                        }
                        // 2.B.2
                        auto found_2 = find(excluded_vec_.begin(), excluded_vec_.end(), s);
                        bool s_is_in_excluded = found_2 != excluded_vec_.end();
                        if (s_is_in_excluded) {
                            continue;
                        } else {
                            if (m.contact_end_time_ < local_forfeit_time) {
                                local_forfeit_time = m.contact_end_time_;
                            }
                            if (m.contact_start_time_ > local_best_delivery_time) {
                                local_best_delivery_time = m.contact_start_time_;
                            }
                            double forwarding_latency = (2 * ecc_) / m.data_transmission_rate_;     // it's a very small account
                            double next_deadline = min((m.contact_end_time_ - forwarding_latency), cur_deadline) - NS3DTNBIT_BUFFER_CHECK_INTERVAL;
                            if (next_deadline < local_time_) {
                                cout << "CGR_DEBUG:" << __LINE__
                                    << ";m.contact_end_time_=" << m.contact_end_time_
                                    << ";next_deadline=" << next_deadline
                                    << ";cur_deadline=" << cur_deadline
                                    << ";forwarding_latency=" << forwarding_latency
                                    << ";local_time_=" << local_time_ << endl;
                                continue;
                            }
                            if (cgr_debug_flag_0) {
                                if (debug_cgr_that_seqno_ == NS3DTNBIT_CGR_DEBUG_SEQ_1 || debug_cgr_that_seqno_ == NS3DTNBIT_CGR_DEBUG_SEQ_2) {
                                    cout << "\nCGR_DEBUG:\nthis is the xmit we enter" 
                                        << ";cur_deadline = " << cur_deadline
                                        << ";forfeit_time_ = " << local_forfeit_time
                                        << ";next_deadline = " << next_deadline << endl;
                                    cout << "\n m ==> m.contact_start_time_ =" << m.contact_start_time_
                                        << ";m.contact_end_time_=" << m.contact_end_time_
                                        << ";m.node_id_of_from_=" << m.node_id_of_from_
                                        << ";m.node_id_of_to_=" << m.node_id_of_to_
                                        << ";m.data_transmission_rate_=" << m.data_transmission_rate_ << endl;
                                }
                            }
                            assert(local_time_ < next_deadline);
                            ContactReviewProcedure(s, next_deadline, local_best_delivery_time);
                        }
                    }
                }
            }
            // 3.
            excluded_vec_.erase(find(excluded_vec_.begin(), excluded_vec_.end(), cur_d));
            if (cgr_debug_flag_1) {
                debug_recurrsive_deep_ -= 1;
                debug_recurrsive_path_stack_.pop();
            }
        }

        int CGRRouting::ForwardDecision() {
            if (proximate_vec_.empty()) {
                std::cout << "Warn: cgr can't find one !" << __LINE__ << endl;
                debug_cgr_this_exhausted_search_not_found_ = true;
                return -1;
            } else {
                if (proximate_vec_.size() >= 2) {
                    std::cout << "Warn : we need to implement a network configuration matter decision!" << endl;
                    return NCMDecision();
                } else if (proximate_vec_.size() == 1) {
                    return 0;
                } else {
                    std::cout << "Error:" << __FILE__ <<  __LINE__ << endl;
                    std::abort();
                }
            }
        }

        // implement this! TODO
        // check https://tools.ietf.org/html/draft-burleigh-dtnrg-cgr-00 -------- 2.5.3
        int CGRRouting::NCMDecision() {
            return 0;
        }

        EmptyRouting::EmptyRouting(DtnApp& dp) : RoutingMethodInterface(dp) {}

        int EmptyRouting::DoRoute(int s, int d)  {
            std::cout << " empty method, using spray and wait won't arrive here!" << std::endl;
            std::abort();
        }
    } /* ns3dtnbit */ 

} /* ns3  */ 
