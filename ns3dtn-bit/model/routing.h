#ifndef ROUTING_H
#define ROUTING_H value

#include "routingInterface.h"
// this file is a massive shit?  yes, but I really want it keep this dirty.
// refactory yourself, first step would be don't repeat

namespace ns3 {

    // Heuristic
    namespace ns3dtnbit {
        class YouRouting : public RoutingMethodInterface {
            public :
            YouRouting(DtnApp& dp);
            // s is source index, d is dest index, return next hop
            int DoRoute(int s, int d) override;
void GetInfo(node_id_t destination_id, 
                node_id_t from_id, std::vector<node_id_t> vec_of_current_neighbor, 
                node_id_t own_id, dtn_time_t expired_time, 
                uint32_t bundle_size, map<node_id_t, vector<node_id_t>> id2cur_exclude_vec_of_id, 
                dtn_time_t local_time, dtn_seqno_t that_seqno);
            private :
            int DoRouteDetail(int s, int d);
            dtn_seqno_t debug_that_seqno_;
            //           dest  / hop / seqno
            vector<tuple<node_id_t, node_id_t, dtn_seqno_t>> routed_table_;
        };
    } /* ns3dtnbit */ 
 

    // TEG
    namespace ns3dtnbit {
        class TegRouting : public RoutingMethodInterface {
            public :
            TegRouting(DtnApp& dp);
            // s is source index, d is dest index, return next hop
            int DoRoute(int s, int d) override;     
void GetInfo(node_id_t destination_id, 
                node_id_t from_id, std::vector<node_id_t> vec_of_current_neighbor, 
                node_id_t own_id, dtn_time_t expired_time, 
                uint32_t bundle_size, map<node_id_t, vector<node_id_t>> id2cur_exclude_vec_of_id, 
                dtn_time_t local_time, dtn_seqno_t that_seqno);
            private :
            int DoRouteDetail(int s, int d);
            dtn_seqno_t debug_that_seqno_;
            //           dest  / hop / seqno
            vector<tuple<node_id_t, node_id_t, dtn_seqno_t>> routed_table_;
        };

    } /* ns3dtnbit */ 

    // EmptyRouitng
    namespace ns3dtnbit {
        class EmptyRouting : public RoutingMethodInterface {
            public :
                EmptyRouting(DtnApp& dp);
                int DoRoute(int s, int d) override;
        };
    } /* ns3dtnbit */ 
} /* ns3  */ 
#endif /* ifndef ROUTING_H */
