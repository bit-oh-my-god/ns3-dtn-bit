#ifndef DTN_PRE_H
#define DTN_PRE_H value


#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h" 
#include "ns3/v4ping-helper.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/double.h"
#include "ns3/random-variable-stream.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ns2-mobility-helper.h"
#include "ns3/qos-utils.h"
#include "ns3/log.h"
#include "ns3/test.h"
#include "dtn_package.h"
#include "common_header.h"

// this file declare some class that was instantialized before DTN sim runs

namespace ns3 {

    namespace ns3dtnbit {

        /*
         * by default, get the function name called the logfunc which called this
         */
        std::string GetCallSta(int);
        std::string FilePrint(std::string);
        std::string GetLogStr(std::string);

        // see RFC - CGR
        struct CgrXmit {
            // D' list of xmits means that all xmit in list have node_id_of_to_ to be D
            dtn_time_t contact_start_time_;
            dtn_time_t contact_end_time_;
            node_id_t node_id_of_from_;       // transmission node
            node_id_t node_id_of_to_;         // receiving node
            double data_transmission_rate_; // set it to 80 000
            string ToString() const {
                stringstream ss;
                ss << "\n cgrxmit==> contact_start_time_ =" << contact_start_time_
                    << ";contact_end_time_=" << contact_end_time_
                    << ";node_id_of_from_=" << node_id_of_from_
                    << ";node_id_of_to_=" << node_id_of_to_
                    << ";data_transmission_rate_=" << data_transmission_rate_ << endl;
                return ss.str();
            }
        };

        bool operator<(CgrXmit const & lhs, CgrXmit const & rhs);

        // used for boost graph
        struct my_edge_property {
            my_edge_property () { }
            my_edge_property(int v, int c) {
                distance_ = v;
                message_color_ = c;
            }
            // physic distance
            int distance_;
            // see teg paper
            int message_color_;
        };

        // used for boost graph
        struct my_vertex_property {
            my_vertex_property() {}
            my_vertex_property(string s) {
                name_ = s;
            }
            string name_;
        };

    } /* ns3dtnbit */ 
} /* ns3 */ 

namespace boost {
    template <class WeightMap,class CapacityMap>
        class edge_writer {
            public:
                edge_writer(WeightMap w, CapacityMap c) : wm(w),cm(c) {}
                template <class Edge>
                    void operator()(std::ostream &out, const Edge& e) const {
                        out << "[distance_=\"" << wm[e] << "\", message_color_=\"" << cm[e] << "\"]";
                    }
            private:
                WeightMap wm;
                CapacityMap cm;

        };

    template <class WeightMap, class CapacityMap>
        edge_writer<WeightMap,CapacityMap> make_edge_writer(WeightMap w,CapacityMap c) {
            return edge_writer<WeightMap,CapacityMap>(w,c);
        } 
} /* boost */ 

namespace ns3 {
    namespace ns3dtnbit {

        class DtnApp;

        // adjacent object
        struct Adob {
            //using EdgeProperties = boost::property<edge_mycost_t, int>;
            using EdgeProperties = my_edge_property;
            //using NameProperties = boost::property<boost::vertex_name_t, std::string>;
            using VertexProperties = my_vertex_property;
            // use vecS is essential to use vertex_descriptor as an index of vector
            using Graph = boost::adjacency_list < boost::vecS, boost::vecS, boost::bidirectionalS, VertexProperties, EdgeProperties, boost::no_property>;
            //using Graph = boost::adjacency_list < boost::vecS, boost::vecS, boost::directedS, NameProperties, EdgeProperties, boost::no_property>;
            using VeDe = boost::graph_traits < Graph >::vertex_descriptor;
            using EdDe = boost::graph_traits < Graph >::edge_descriptor;
            Adob();
            ~Adob();
            // generate g_vec_, a vector of static graphs for heuristics routing using, also used for other routing
            void AdobDo_01(std::map<int, vector<vector<int>>> t_2_adjacent_array, int node_number);

            // Aim : generate ob for time-expanded graph
            // Note : teg_layer_n is number of layer in teg, which would let expanded teg to have teg_layer_n * N amout of nodes.
            // where N is the number of nodes in a static graph
            void AdobDo_02(int node_number, int teg_layer_n, int max_range);

            // get teg_routing_table_ done for time-expanded graph
            // the complexity of this function is O(N * N * N * T), please do not use too large argument
            void AdobDo_03();

            // get xmit for every nodes, for cgr
            void AdobDo_04();

            Graph get_graph_for_now() const;
            int get_teg_size(); 
            int max_range_;
            int get_g_vec_size();
            int get_node_number();
            // timepoint = t_vec_[i'th slice]
            vector<dtn_time_t> t_vec_;
            // static graph = g_vec_[i'th slice]
            vector<Graph> g_vec_;
            // vertex map of i'th slice
            vector<unordered_map<int, VeDe>> g_vede_m_;
            // vertex map of teg
            unordered_map<string, VeDe> name2vd_map;
            Graph teg_;
            //                    i     j      t
            using Teg_i_j_t = tuple<int, int, int>;
            //                     src   dst   time   color
            using DelayIndex = tuple<int, int, int, int>;
            // following is something I want to use for unordered_map, but yet bug still
            // using custom hash for tuple element in unordered_map 
            struct key_hash : public std::unary_function<Teg_i_j_t, std::size_t> {
                std::size_t operator()(const Teg_i_j_t& k) const {
                    return std::get<0>(k) ^ std::get<1>(k) ^ std::get<2>(k);
                }
            };
            struct equal_to : public std::binary_function<Teg_i_j_t, Teg_i_j_t, bool> {
                bool operator()(const Teg_i_j_t& lhs, const Teg_i_j_t& rhs) const {
                    return std::get<0>(lhs) == std::get<0>(rhs) && std::get<1>(lhs) == std::get<1>(rhs) && std::get<2>(lhs) == std::get<2>(rhs);
                }
            };
            struct key_hash0 : public std::unary_function<DelayIndex, std::size_t> {
                std::size_t operator()(const DelayIndex& k) const {
                    return std::get<0>(k) ^ std::get<1>(k) ^ std::get<2>(k) ^ std::get<3>(k);
                }
            };
            struct equal_to0 : public std::binary_function<DelayIndex, DelayIndex, bool> {
                bool operator()(const DelayIndex& lhs, const DelayIndex& rhs) const {
                    return std::get<0>(lhs) == std::get<0>(rhs) && std::get<1>(lhs) == std::get<1>(rhs) && std::get<2>(lhs) == std::get<2>(rhs) && std::get<3>(lhs) == std::get<3>(rhs);
                }
            };
            //                                            tegijt     k
            //using CustomedMap = std::unordered_map<const Teg_i_j_t, int, key_hash, equal_to>;
            //                                      ijtc        delay
            //using DelayMap = unordered_map<const DelayIndex, int, key_hash0, equal_to0>;
            // using unordered_map would be more efficient, but I got a compile error, fix this compile error TODO
            using CustomedMap = map<Teg_i_j_t, int>;
            using DelayMap = map<DelayIndex, int>;
            // color should be equal to the interval of graph, see teg paper
            const static int hypo_c = 1;
            CustomedMap teg_routing_table_;
            DelayMap delay_map_;
            int node_number_;
            // for CGR
            map<int, vector<CgrXmit>> node_id2cgr_xmit_vec_map_;
        };
        
    } /* ns3dtnbit */ 

} /* ns3  */ 


#endif /* ifndef DTN_PRE_H */
