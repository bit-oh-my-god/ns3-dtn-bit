#include "routing.h"

namespace ns3 {
    namespace ns3dtnbit {

        YouRouting::YouRouting(DtnApp& dp) : RoutingMethodInterface(dp) {}

        int YouRouting::DoRoute(int s, int d) {
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

        TegRouting::TegRouting(DtnApp& dp) : RoutingMethodInterface(dp) {}

        int TegRouting::DoRoute(int s, int d) {
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
                while (!found_vk) {
                    auto found = ref_adob.teg_routing_table_.find(rt_index);
                    if (found != ref_adob.teg_routing_table_.end()) {
                        vk = ref_adob.teg_routing_table_[rt_index];
                    } else {
                        vk = std::get<1>(rt_index);
                        found_vk = true;
                    }
                    if (vk < 0 || vk > ref_adob.get_node_number()) { std::cout << "Error: can't be" << __LINE__ << std::endl; std::abort(); }
                    rt_index = make_tuple(s, vk, tmp_time);
                }
                if (vk == -1) { std::abort(); }
                return vk;
            }
        }

        CGRRouting::CGRRouting(DtnApp& dp) : RoutingMethodInterface(dp) { }

        int CGRRouting::DoRoute(int s, int d) {
            Init();
            assert(d == destination_id_);
            assert(s == own_id_);
            ContactReviewProcedure(destination_id_, expired_time_);
            int result = ForwardDecision();
            assert(result < 100);
#ifdef DEBUG
            {
                cout << "DEBUG_CGR:" << "result= " << result 
                    << ";CGR-" << s << "->" << d << endl;
            }
#endif /* ifdef DEBUG */
            return result;
        }

        void CGRRouting::GetInfo(node_id_t destination_id, node_id_t from_id, std::vector<node_id_t> vec_of_current_neighbor, node_id_t own_id, dtn_time_t expired_time, int bundle_size, int networkconfigurationflag, map<int, vector<int>> id2cur_exclude_vec_of_id) {
            swap(id_of_d2cur_excluded_vec_of_d_, id2cur_exclude_vec_of_id); // a more efficent assignment
            destination_id_ = destination_id;
            node_id_transmit_from_ = from_id;
            expired_time_ = expired_time;
            own_id_ = own_id;
            ecc_ = bundle_size;
            id_of_current_neighbor_ = vec_of_current_neighbor;
            networkconfigurationflag_ = networkconfigurationflag;
        }

        void CGRRouting::Init() {
            excluded_vec_ = vector<int>();
            proximate_vec_ = vector<int>();
            excluded_vec_.push_back(node_id_transmit_from_);
            auto cur_excluded = id_of_d2cur_excluded_vec_of_d_[destination_id_];
            for (auto nei : id_of_current_neighbor_) {
                auto found = find(cur_excluded.begin(), cur_excluded.end(), nei);
                if (found != cur_excluded.end()) {
                    excluded_vec_.push_back(nei);
                }
            }
            forfeit_time_ = NS3DTNBIT_HYPOTHETIC_INFINITE_DELAY;
            best_delivery_time_ = 0;
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
        void CGRRouting::ContactReviewProcedure(node_id_t cur_d, dtn_time_t cur_deadline) {
            auto previous_of_forfeit_time = forfeit_time_;
            auto previous_of_best_delivery_time = best_delivery_time_;
            // 1.
            excluded_vec_.push_back(cur_d);
            const Adob& ref_adob = RoutingMethodInterface::get_adob();
            auto cgr_xmit_vec = ref_adob.node_id2cgr_xmit_vec_map_[cur_d];
#ifdef DEBUG
            {
                cout << "DEBUG_CGR" << ": ====== for node-" << cur_d << " ======" << endl;
                for (auto m : cgr_xmit_vec) {
                    cout << "\n m ==> m.contact_start_time_ =" << m.contact_start_time_
                        << ";m.contact_end_time_=" << m.contact_end_time_
                        << ";m.node_id_of_from_=" << m.node_id_of_from_
                        << ";m.node_id_of_to_=" << m.node_id_of_to_
                        << ";m.data_transmission_rate_=" << m.data_transmission_rate_ << endl;
                }
                cout << "end of DEBUG_CGR" << endl;
            }
#endif
            // 2.
            for (auto m : cgr_xmit_vec) {
                bool start_time_after_deadline = m.contact_start_time_ > cur_deadline;
                if (start_time_after_deadline) {
                    // 2.A
                    continue;
                } else {
                    // 2.B
                    node_id_t s = m.node_id_of_from_;
                    bool s_is_local_node_with_own_id = s == own_id_;
                    if (s_is_local_node_with_own_id) {
                        // 2.B.1
                        // compute ECC for this bundle
                        int ecc = ecc_;
                        assert(m.contact_end_time_ - m.contact_start_time_ < 5000);
                        // not accurate, just a hypothetic value,  TODO
                        int ecc_of_other_bundles = 0;
                        int residual_capacity = ((m.contact_end_time_ - m.contact_start_time_) * m.data_transmission_rate_) - ecc_of_other_bundles;
                        assert(residual_capacity > 1000);
                        auto found_1 = find(proximate_vec_.begin(), proximate_vec_.end(), cur_d);
                        bool d_is_in_proximate = found_1 != proximate_vec_.end();
                        if (residual_capacity < ecc) {
                            continue;
                        } else if (d_is_in_proximate) {
                            continue;
                        } else {
                            if (m.contact_end_time_ < forfeit_time_) {
                                forfeit_time_ = m.contact_end_time_;
                            } 
                            if (m.contact_start_time_ > best_delivery_time_) {
                                best_delivery_time_ = m.contact_start_time_;
                            }
                            proximate_vec_.push_back(cur_d);
                            // Note the computed forfeit time and best-case delivery time in the event that the bundle is queued for transmission to D. TODO
                        }
                    } else {
                        // 2.B.2
                        auto found_2 = find(excluded_vec_.begin(), excluded_vec_.end(), s);
                        bool s_is_in_excluded = found_2 != excluded_vec_.end();
                        if (s_is_in_excluded) {
                            continue;
                        } else {
                            if (m.contact_end_time_ < forfeit_time_) {
                                forfeit_time_ = m.contact_end_time_;
                            }
                            if (m.contact_start_time_ > best_delivery_time_) {
                                best_delivery_time_ = m.contact_start_time_;
                            }
                            double forwarding_latency = (2 * ecc_) / m.data_transmission_rate_;     // it's a very small account
                            double next_deadline = min((m.contact_end_time_ - forwarding_latency), cur_deadline);
                            ContactReviewProcedure(s, next_deadline);
                        }
                    }
                }
            }
            // 3.
            excluded_vec_.erase(find(excluded_vec_.begin(), excluded_vec_.end(), cur_d));
            forfeit_time_ = previous_of_forfeit_time;
            best_delivery_time_ = previous_of_best_delivery_time;
        }

        int CGRRouting::ForwardDecision() {
            if (proximate_vec_.empty()) {
                std::cout << "Warn: cgr can't find one !" << __LINE__ << endl;
                return -1;
            } else {
                if (proximate_vec_.size() >= 2) {
                    std::cout << "Warn : we need to implement a network configuration matter decision!" << endl;
                    return NCMDecision();
                } else if (proximate_vec_.size() == 1) {
                    return proximate_vec_[0];
                } else {
                    std::cout << "Error:" << __LINE__ << endl;
                    std::abort();
                }
            }
        }

        // implement this! TODO
        // check https://tools.ietf.org/html/draft-burleigh-dtnrg-cgr-00 -------- 2.5.3
        int CGRRouting::NCMDecision() {
            return proximate_vec_[0];
        }

    } /* ns3dtnbit */ 

} /* ns3  */ 
