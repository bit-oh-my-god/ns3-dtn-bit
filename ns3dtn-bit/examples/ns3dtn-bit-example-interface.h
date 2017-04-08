#ifndef NS3DTN_BIT_EXAMPLE_INTERFACE_H
#define NS3DTN_BIT_EXAMPLE_INTERFACE_H value

#include "ns3/ns3dtn-bit.h"

namespace ns3 {
    namespace ns3dtnbit {

        class EmptyRouting : public RoutingMethodInterface {
            public :
                EmptyRouting(DtnApp& dp) : RoutingMethodInterface(dp) {}
                int DoRoute(int s, int d) override {
                    std::cout << " empty method, abort!" << std::endl;
                    std::abort();
                }
        };

        class DtnExampleInterface {
            public :
                DtnExampleInterface();
                DtnExampleInterface(DtnExampleInterface&& rh);
                virtual void ConfigureEx(int argc, char** argv);
                virtual void RunEx();
                virtual void ReportEx(std::ostream& os) = 0;
                DtnExampleInterface& operator=(DtnExampleInterface&& rh) {
                    if (this!=&rh) {
                        // do some TODO
                    }
                    return *this;
                }

            protected :
                uint32_t random_seed_;
                vector<Ptr<DtnApp>> app_;
                uint32_t node_number_;
                dtn_time_t simulation_duration_;
                bool pcap_boolean_, print_route_boolean_, print_log_boolean_;
                std::string trace_file_;
                std::string teg_file_;
                std::string log_file_;
                std::ofstream file_stream_;
                NodeContainer nodes_container_;
                NetDeviceContainer net_devices_container_;
                Ipv4InterfaceContainer ip_interface_container_;
                DtnApp::RoutingMethod ex_rm_;

                vector<DtnApp::Adob> CreateAdjacentList();
                virtual void CreateNodes();
                virtual void CreateDevices();
                virtual void InstallInternetStack();
                virtual void ScheduleTask();
                virtual std::unique_ptr<RoutingMethodInterface> CreateRouting(DtnApp& dtn) {
                    auto p = new EmptyRouting(dtn);
                    return std::unique_ptr<RoutingMethodInterface>(p);
                }

                virtual void InstallApplications();
                // call LogCounter to counter for simulation time
                void LogCounter(int);
        };   

    } /* ns3dtnbit
    */ 
} /* ns3  */ 
#endif /* ifndef NS3DTN_BIT_EXAMPLE_INTERFACE_H */
