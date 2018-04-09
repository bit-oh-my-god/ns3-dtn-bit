#include "routingInterface.h"
#include "ns3dtn-bit.h"
namespace ns3 {
    namespace ns3dtnbit {
    RoutingMethodInterface::RoutingMethodInterface(DtnApp& dp) : out_app_(dp) {}
        RoutingMethodInterface::~RoutingMethodInterface() {}
        void RoutingMethodInterface::GetInfo(int destination_id, int from_id, std::vector<int> vec_of_current_neighbor, int own_id, dtn_time_t expired_time, int bundle_size, int networkconfigurationflag, map<int, vector<int>> id2cur_exclude_vec_of_id, dtn_time_t local_time, dtn_seqno_t that_seqno) {
            // nothing
        }

        // CGRQM TODO
        void RoutingMethodInterface::StorageinfoMaintainInterface(string s
                ,map<int, pair<int, int>> parsed_storageinfo_from_neighbor
                ,map<int, pair<int, int>>& move_storageinfo_to_this
                ,map<int, int> storagemax
                ,vector<int> path_of_route
                , pair<int, int> update
                ) {
            // nothing
        }
        Adob RoutingMethodInterface::get_adob() { return out_app_.routing_assister_.vec_[0]; }

        std::string RoutingMethodInterface::LogPrefix() const {
                    std::stringstream ss;
                    ss << "[time-" << Simulator::Now().GetSeconds() 
                        << ";node-" << out_app_.GetNodeId() << ";";
                    return ss.str();
                }

    } /* ns3dtnbit */ 
}