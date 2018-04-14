#ifndef CGRQMROUTING_H
#define CGRQMROUTING_H value

#include "routingInterface.h"
#include "cgrrouting.h"

namespace ns3 {

    // CGRQM
    namespace ns3dtnbit {
        // this is CGR-QM
        class CGRQMRouting : public CGRRouting {
            public :
            CGRQMRouting(DtnApp& dp);
            ~CGRQMRouting() override;
            // @brief invoke : 
            //              StorageinfoMaintainInterface("route answer is made, add queue usage, and time to decay", qm_empty01, qm_empty02, qm_empty03, get<6>(tmpfsx)); 
            virtual int ForwardDecision(vector<RouteResultCandidate> & rrc_vec);

            // CGRQM TODO
            void StorageinfoMaintainInterface(string action
                    ,map<node_id_t, pair<int, int>> parsed_storageinfo_from_neighbor
                    ,map<node_id_t, pair<int, int>>& move_storageinfo_to_this
                    ,map<node_id_t, size_t> storagemax
                    ,pair<vector<node_id_t>, dtn_time_t> path_of_route_and_decaytime
                        ,pair<node_id_t, int> update ={ -1, -1 }
                    ) override; 

            // @brief return index of rrc_vec
            int NCMDecision(vector<RouteResultCandidate> const & rrc_vec) override;
            void DebugUseScheduleToDoSome() override;
            void NotifyRouteSeqnoIsAcked(dtn_seqno_t seq)override;
            void LoadCurrentStorageOfOwn(node_id_t node, size_t storage) override;
            // nodeid -> [belive value , storageusage value]
            map<node_id_t, pair<int, int>> storageinfo_maintained_;
            // nodeid -> storage max
            map<node_id_t, size_t> storage_max_;
            // pq for release storage
            // recordtime -> [nodeinpath, ...]
            map<dtn_time_t, vector<int>> release_queue_;
            dtn_time_t last_release_check_time_;
            dtn_time_t last_belive_decay_;
            map<dtn_seqno_t, std::function<void()>> seqno2ackedcb_;
        };
    } /* ns3dtnbit */ 
} /* ns3  */ 
#endif /* ifndef CGRQMROUTING_H */