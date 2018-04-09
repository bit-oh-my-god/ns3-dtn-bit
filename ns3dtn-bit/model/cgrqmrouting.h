#ifndef CGRQMROUTING_H
#define CGRQMROUTING_H value

#include "routingInterface.h"

namespace ns3 {

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
} /* ns3  */ 
#endif /* ifndef CGRQMROUTING_H */