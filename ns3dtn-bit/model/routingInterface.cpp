#include "routingInterface.h"
#include "ns3dtn-bit.h"
namespace ns3 {
    namespace ns3dtnbit {
        RoutingMethodInterface::RoutingMethodInterface(DtnApp& dp) : out_app_(dp) {
            app_node_id_= out_app_.GetNodeId();
            assert(app_node_id_ >= 0);
        }

        RoutingMethodInterface::~RoutingMethodInterface() {}

        void RoutingMethodInterface::GetInfo(node_id_t destination_id, 
                node_id_t from_id, std::vector<node_id_t> vec_of_current_neighbor, 
                node_id_t own_id, dtn_time_t expired_time, 
                uint32_t bundle_size, map<node_id_t, vector<node_id_t>> id2cur_exclude_vec_of_id, 
                dtn_time_t local_time, dtn_seqno_t that_seqno) {
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
        Adob const & RoutingMethodInterface::get_adob() { return out_app_.routing_assister_.vec_[0]; }

        std::string RoutingMethodInterface::LogPrefix() const {
                    std::stringstream ss;
                    ss << "[time-" << Simulator::Now().GetSeconds() 
                        << ";node-" << app_node_id_ << ";";
                    return ss.str();
                }

    } /* ns3dtnbit */ 
}