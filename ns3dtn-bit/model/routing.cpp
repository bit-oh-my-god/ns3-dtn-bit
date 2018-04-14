#include "routing.h"

namespace ns3 {

    //
    // ============================== Heuristic Routing
    // 
    namespace ns3dtnbit {
        
        NS_LOG_COMPONENT_DEFINE ("DtnRouting");
        #define LogPrefixMacro LogPrefix()<<"[DtnRouting]line-"<<__LINE__<<"]"

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

        void YouRouting::GetInfo(node_id_t destination_id, 
                node_id_t from_id, std::vector<node_id_t> vec_of_current_neighbor, 
                node_id_t own_id, dtn_time_t expired_time, 
                uint32_t bundle_size, map<node_id_t, vector<node_id_t>> id2cur_exclude_vec_of_id, 
                dtn_time_t local_time, dtn_seqno_t that_seqno, vector<node_id_t> exclude_node_of_thispkt) {

            debug_that_seqno_ = that_seqno;
        }

    } /* ns3dtnbit */ 


    //
    // ========================================= TEG
    //
    namespace ns3dtnbit {
        TegRouting::TegRouting(DtnApp& dp) : RoutingMethodInterface(dp) {}

        void TegRouting::GetInfo(node_id_t destination_id, 
                node_id_t from_id, std::vector<node_id_t> vec_of_current_neighbor, 
                node_id_t own_id, dtn_time_t expired_time, 
                uint32_t bundle_size, map<node_id_t, vector<node_id_t>> id2cur_exclude_vec_of_id, 
                dtn_time_t local_time, dtn_seqno_t that_seqno, vector<node_id_t> exclude_node_of_thispkt) {

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

    } /* ns3dtnbit */ 

    namespace ns3dtnbit {
        EmptyRouting::EmptyRouting(DtnApp& dp) : RoutingMethodInterface(dp) {}

        int EmptyRouting::DoRoute(int s, int d)  {
            std::cout << " empty method, using spray and wait won't arrive here!" << std::endl;
            std::abort();
        }
    } /* ns3dtnbit */ 

} /* ns3  */ 
