
#ifndef ROUTINGINTERFACE_H
#define ROUTINGINTERFACE_H
#include "dtn-pre.h"
namespace ns3{
namespace ns3dtnbit {
        class RoutingMethodInterface {
            public :
                RoutingMethodInterface(DtnApp& dp);
                virtual ~RoutingMethodInterface();
                // Aim :
                // src is the node number for traffic source node
                // dst is the node number for traffic sink node
                // return the next hop node number
                // Note : 
                // use adob in the out_app_
                // @return nodeid of next hop, return -1 no routing result this time, return -2 no routing result forever,
                virtual int DoRoute(int src, int dst) = 0;
                /*
                 * this time I modify this interface for CGR, next time I would do it again! Change RoutingMethodInterface to Generic!!!
                 * TODO
                 * */
                virtual void GetInfo(int destination_id, int from_id, std::vector<int> vec_of_current_neighbor, int own_id, dtn_time_t expired_time, int bundle_size, int networkconfigurationflag, map<int, vector<int>> id2cur_exclude_vec_of_id, dtn_time_t local_time, dtn_seqno_t that_seqno);
                // CGRQM TODO
                virtual void StorageinfoMaintainInterface(string s
                        ,map<int, pair<int, int>> parsed_storageinfo_from_neighbor
                        ,map<int, pair<int, int>>& move_storageinfo_to_this
                        ,map<int, int> storagemax
                        ,vector<int> path_of_route
                        ,pair<int, int> update ={ -1, -1 }
                        );
                // read only 
                std::string LogPrefix() const;
            protected :
                
                const DtnApp& out_app_;
                Adob get_adob();
        };
    } /* ns3dtnbit */ 
}

#endif /* ROUTINGINTERFACE_H */