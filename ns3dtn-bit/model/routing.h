#ifndef ROUTING_H
#define ROUTING_H value

#include "ns3dtn-bit.h"
// this file is a massive shit?  yes, but I really want it keep this dirty.
// refactory yourself, first step would be don't repeat

namespace ns3 {

    // Heuristic
    namespace ns3dtnbit {
        class YouRouting : public RoutingMethodInterface {
            using node_id_t = int;
            public :
            YouRouting(DtnApp& dp);
            // s is source index, d is dest index, return next hop
            int DoRoute(int s, int d) override;
            void GetInfo(node_id_t destination_id, node_id_t from_id, std::vector<node_id_t> vec_of_current_neighbor, node_id_t own_id, dtn_time_t expired_time, int bundle_size, int networkconfigurationflag, map<int, vector<int>> id2cur_exclude_vec_of_id, dtn_time_t local_time, dtn_seqno_t that_seqno) override;
            private :
            int DoRouteDetail(int s, int d);
            dtn_seqno_t debug_that_seqno_;
            //           dest  / hop / seqno
            vector<tuple<node_id_t, node_id_t, dtn_seqno_t>> routed_table_;
        };
    } /* ns3dtnbit */ 

    // CGR
    namespace ns3dtnbit {
        // read this :
        // https://tools.ietf.org/html/draft-burleigh-dtnrg-cgr-00
        // note this algo is not CGR-EB, it's standard CGR (which can be found in here: Analysis of the contact graph routing algorithm: Bounding interplanetary paths)
        class CGRRouting : public RoutingMethodInterface {
            using node_id_t = int;
            public :
            CGRRouting(DtnApp& dp);
            // s is source index, d is dest index, return next hop
            int DoRoute(int s, int d) override;

            void GetInfo(node_id_t destination_id, node_id_t from_id, std::vector<node_id_t> vec_of_current_neighbor, node_id_t own_id, dtn_time_t expired_time, int bundle_size, int networkconfigurationflag, map<int, vector<int>> id2cur_exclude_vec_of_id, dtn_time_t local_time, dtn_seqno_t that_seqno) override;

            private :
            
            class RouteResultCandidate {

            };
            void Init();
            // @return if can't route
            bool foundExhausted();
            // @return if cashed
            bool foundCashedRouteTable();
            // @brief record for foundExhausted() & foundCashedRouteTable() use 
            void RecordCRPResult(string str);

            // @brief first step of CGR-three-steps
            void Init();
            // @brief second step of CGR-three-steps
            //Please read this link, if you want to know about this. https://tools.ietf.org/html/draft-burleigh-dtnrg-cgr-00
            //---- cur_d 
            //---- cur_deadline
            //---- proximate_vec_
            //---- excluded_vec_
            //---- forfeit_time_
            //---- best_delivery_time_
            void ContactReviewProcedure(node_id_t cur_d, dtn_time_t cur_deadline, dtn_time_t best_deli);
            // @brief third step of CGR-three-steps
            // when if_cannot_find_route=false if_route_candidate_cashed=false then invoke NCMDecision()
            // if_cannot_find_route=true return -1
            // if_route_candidate_cashed=true return cashed result
            int ForwardDecision(bool if_cannot_find_route=false,bool if_route_candidate_cashed=false);

            // implement this! TODO
            // check https://tools.ietf.org/html/draft-burleigh-dtnrg-cgr-00 -------- 2.5.3
            // @brief return index of proximate_vec_
            int NCMDecision();

            // ----------------- start of debug
            bool cgr_debug_flag_0 = false;
            bool cgr_debug_flag_1 = false;
            int debug_recurrsive_deep_;
            int debug_crp_enter_count_;
            map<int, int> debug_node_access_count_map_;
            stack<int> debug_recurrsive_path_stack_;
            bool debug_cgr_this_exhausted_search_not_found_;
            dtn_seqno_t debug_cgr_that_seqno_;
            // @brief inspect recurrsive deep and stack 
            void debugFunc01(node_id_t cur_d,string str);
            // @brief inspect Cgr_xmit
            void debugFunc02(node_id_t cur_d, vector<CgrXmit>& cgr_xmit_vec_ref) const;
            // @brief print some
            void debugFunc03(string str) const;
            // @brief print some
            void debugFunc04(CgrXmit& m, dtn_time_t cur_deadline,dtn_time_t local_forfeit_time, double next_deadline, string str) const;
            void DebugPrintXmit(vector<CgrXmit>& cgr_xmit_vec_ref, int cur_d) const;
            // -------------- end of debug
            private :
            //          dest        seqno
            vector<pair<node_id_t, dtn_seqno_t>> exhausted_search_target_list_;
            //           dest  /   forfeit time  / best delivery time / hop / seqno / routing time
            vector<tuple<node_id_t, dtn_time_t, dtn_time_t, node_id_t, dtn_seqno_t, dtn_time_t>> routed_table_;
            vector<RouteResultCandidate> routed_result_candidates_;
            node_id_t destination_id_; 
            
            node_id_t own_id_;
            dtn_time_t local_time_;
            int cgr_find_proximate_count_;
            
            int ecc_;
            int networkconfigurationflag_;
            node_id_t node_id_transmit_from_;
            std::vector<node_id_t> id_of_current_neighbor_;
            dtn_time_t expired_time_;
            vector<node_id_t> proximate_vec_;
            vector<node_id_t> excluded_vec_;
            map<int, vector<node_id_t>> id_of_d2cur_excluded_vec_of_d_;
            dtn_time_t forfeit_time_;
            dtn_time_t best_delivery_time_;
            vector<dtn_time_t> final_forfeit_time_;
            vector<dtn_time_t> final_best_delivery_time_;
        };
    } 

    // CGRQM
    namespace ns3dtnbit {
        // this is CGR-QM
        class CGRQMRouting : public RoutingMethodInterface {
            using node_id_t = int;
            public :
            CGRQMRouting(DtnApp& dp);
            // s is source index, d is dest index, return next hop
            int DoRoute(int s, int d) override;

            void GetInfo(node_id_t destination_id, node_id_t from_id, std::vector<node_id_t> vec_of_current_neighbor, node_id_t own_id, dtn_time_t expired_time, int bundle_size, int networkconfigurationflag, map<int, vector<int>> id2cur_exclude_vec_of_id, dtn_time_t local_time, dtn_seqno_t that_seqno) override;

            // CGRQM TODO
            void StorageinfoMaintainInterface(string action
                    ,map<int, pair<int, int>> parsed_storageinfo_from_neighbor
                    ,map<int, pair<int, int>>& move_storageinfo_to_this
                    ,map<int, int> storagemax
                    ,vector<int> path_of_route
                        ,pair<int, int> update ={ -1, -1 }
                    ) override; 

            private :
            void DebugPrintXmit(vector<CgrXmit>& cgr_xmit_vec_ref, int cur_d);

            void Init();

            /*
             * Please read this link, if you want to know about this. https://tools.ietf.org/html/draft-burleigh-dtnrg-cgr-00
             *  ---- cur_d 
             *  ---- cur_deadline
             *  ---- proximate_vec_
             *  ---- excluded_vec_
             *  ---- forfeit_time_
             *  ---- best_delivery_time_
             * */
            void ContactReviewProcedure(node_id_t cur_d, dtn_time_t cur_deadline, dtn_time_t best_deli, vector<int> cur_d_path);

            int ForwardDecision();

            // implement this! TODO
            // check https://tools.ietf.org/html/draft-burleigh-dtnrg-cgr-00 -------- 2.5.3
            int NCMDecision();


            // -----------------

            private :
            node_id_t destination_id_; 
            bool cgr_debug_flag_0 = false;
            bool cgr_debug_flag_1 = false;
            node_id_t own_id_;
            dtn_time_t local_time_;
            int cgr_find_proximate_count_;
            int debug_recurrsive_deep_;
            int debug_crp_enter_count_;
            map<int, int> debug_node_access_count_map_;
            stack<int> debug_recurrsive_path_stack_;
            bool debug_cgr_this_exhausted_search_not_found_;
            dtn_seqno_t debug_cgr_that_seqno_;
            vector<pair<node_id_t, dtn_seqno_t>> exhausted_search_target_list_;
            //           dest  /   forfeit time  / best delivery time / hop(route answer) / seqno / routing time   / route path of that hop
            vector<tuple<node_id_t, dtn_time_t, dtn_time_t, node_id_t, dtn_seqno_t, dtn_time_t, vector<int>>> routed_table_;
            int ecc_;
            int networkconfigurationflag_;
            node_id_t node_id_transmit_from_;
            std::vector<node_id_t> id_of_current_neighbor_;
            dtn_time_t expired_time_;
            vector<node_id_t> proximate_vec_;
            vector<vector<int>> proximate_path_vec_;
            // CGRQM TODO
            // nodeid -> belive value , storage value
            map<int, pair<int, int>> storageinfo_maintained_;
            // nodeid -> storage max
            map<int, int> storage_max_;
            // pq for release storage
            map<dtn_time_t, vector<int>> release_queue_;
            vector<node_id_t> excluded_vec_;
            map<int, vector<node_id_t>> id_of_d2cur_excluded_vec_of_d_;
            dtn_time_t forfeit_time_;
            dtn_time_t best_delivery_time_;
            vector<dtn_time_t> final_forfeit_time_;
            vector<dtn_time_t> final_best_delivery_time_;
        };

    } /* ns3dtnbit */ 

    // TEG
    namespace ns3dtnbit {
        class TegRouting : public RoutingMethodInterface {
            using node_id_t = int;
            public :
            TegRouting(DtnApp& dp);
            // s is source index, d is dest index, return next hop
            int DoRoute(int s, int d) override;     
            void GetInfo(node_id_t destination_id, node_id_t from_id, std::vector<node_id_t> vec_of_current_neighbor, node_id_t own_id, dtn_time_t expired_time, int bundle_size, int networkconfigurationflag, map<int, vector<int>> id2cur_exclude_vec_of_id, dtn_time_t local_time, dtn_seqno_t that_seqno) override;
            private :
            int DoRouteDetail(int s, int d);
            dtn_seqno_t debug_that_seqno_;
            //           dest  / hop / seqno
            vector<tuple<node_id_t, node_id_t, dtn_seqno_t>> routed_table_;
        };

        class EmptyRouting : public RoutingMethodInterface {
            public :
                EmptyRouting(DtnApp& dp);
                int DoRoute(int s, int d) override;
        };
    } /* ns3dtnbit */ 

} /* ns3  */ 
#endif /* ifndef ROUTING_H */
