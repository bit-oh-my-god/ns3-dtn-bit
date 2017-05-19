#ifndef ROUTING_H
#define ROUTING_H value

#include "ns3dtn-bit.h"
// this file is a massive shit?  yes, but I really want it keep this dirty.
// refactory yourself, first step would be don't repeat

namespace ns3 {
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

        // read this :
        // https://tools.ietf.org/html/draft-burleigh-dtnrg-cgr-00
        // not this is not a CGR-EB (which can be found in here: Analysis of the contact graph routing algorithm: Bounding interplanetary paths)
        class CGRRouting : public RoutingMethodInterface {
            using node_id_t = int;
            public :
            CGRRouting(DtnApp& dp);
            // s is source index, d is dest index, return next hop
            int DoRoute(int s, int d) override;

            void GetInfo(node_id_t destination_id, node_id_t from_id, std::vector<node_id_t> vec_of_current_neighbor, node_id_t own_id, dtn_time_t expired_time, int bundle_size, int networkconfigurationflag, map<int, vector<int>> id2cur_exclude_vec_of_id, dtn_time_t local_time, dtn_seqno_t that_seqno) override;

            private :
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
            void ContactReviewProcedure(node_id_t cur_d, dtn_time_t cur_deadline);

            int ForwardDecision();

            // implement this! TODO
            // check https://tools.ietf.org/html/draft-burleigh-dtnrg-cgr-00 -------- 2.5.3
            int NCMDecision();

            private :
            node_id_t destination_id_; 
            node_id_t own_id_;
            dtn_time_t local_time_;
            bool cgr_find_one_proximate_;
            int debug_recurrsive_deep_;
            int debug_crp_enter_count_;
            map<int, int> debug_node_access_count_map_;
            stack<int> debug_recurrsive_path_stack_;
            int debug_abort_count_;
            bool debug_cgr_this_exhausted_search_not_found_;
            dtn_seqno_t debug_cgr_that_seqno_;
            vector<pair<node_id_t, dtn_seqno_t>> exhausted_search_target_list_;
            //           dest  /   forfeit time  / best delivery time / hop / seqno
            vector<tuple<node_id_t, dtn_time_t, dtn_time_t, node_id_t, dtn_seqno_t>> routed_table_;
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
        };

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
