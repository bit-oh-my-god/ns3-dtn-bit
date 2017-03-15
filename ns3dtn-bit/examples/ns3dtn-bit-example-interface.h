#ifndef NS3DTN_BIT_EXAMPLE_INTERFACE_H
#define NS3DTN_BIT_EXAMPLE_INTERFACE_H value

#include "ns3/ns3dtn-bit.h"

namespace ns3 {
    namespace ns3dtnbit
    {
     class DtnExample {
            public :
                DtnExample();
                void Configure(int argc, char** argv);
                void Run();
                void Report(std::ostream& os);
                //void Report(std::ostream& os);

            private :
                uint32_t random_seed_;
                uint32_t node_number_;
                double simulation_duration_;
                bool pcap_boolean_, print_route_boolean_;
                std::string trace_file_;
                std::string log_file_;
                std::ofstream file_stream_;
                NodeContainer nodes_container_;
                NetDeviceContainer net_devices_container_;
                Ipv4InterfaceContainer ip_interface_container_;

                void CreateNodes();
                void CreateDevices();
                void InstallInternetStack();
                void InstallApplications();
                void PopulateArpCache();
                void LogCounter(int);
        };

        class DtnExampleInterface {
            public :
                DtnExampleInterface();
                DtnExampleInterface(DtnExampleInterface&& rh);
                virtual void ConfigureEx(int argc, char** argv);
                virtual void RunEx();
                virtual void ReportEx(std::ostream& os);
                DtnExampleInterface& operator=(DtnExampleInterface&& rh) {
                    if (this!=&rh) {

                    }
                    return *this;
                }
            protected :
                uint32_t random_seed_;
                uint32_t node_number_;
                dtn_time_t simulation_duration_;
                bool pcap_boolean_, print_route_boolean_, print_log_boolean_;
                std::string trace_file_;
                std::string log_file_;
                std::ofstream file_stream_;
                NodeContainer nodes_container_;
                NetDeviceContainer net_devices_container_;
                Ipv4InterfaceContainer ip_interface_container_;

                virtual void CreateNodes();
                virtual void CreateDevices() = 0;
                virtual void InstallInternetStack() = 0;
                virtual void InstallApplications() = 0;
                // call LogCounter to counter for simulation time
                void LogCounter(int);
        };   
    } /* ns3dtnbit
    */ 
    
} /* ns3  */ 
#endif /* ifndef NS3DTN_BIT_EXAMPLE_INTERFACE_H */

