#ifndef NS3DTN_BIT_EXAMPLE_INTERFACE_H
#define NS3DTN_BIT_EXAMPLE_INTERFACE_H value

#include "ns3/ns3dtn-bit.h"
#include "ns3/routingInterface.h"

namespace ns3 {
    namespace ns3dtnbit {

        

        class DtnExampleInterface {
            public :
                DtnExampleInterface();
                DtnExampleInterface(DtnExampleInterface&& rh);
                virtual void ConfigureEx(int argc, char** argv);
                virtual void RunEx();
                virtual void SetSchduleN(size_t n) = 0;
                virtual void ReportEx(std::ostream& os) = 0;
                void set_th(size_t th);
                DtnExampleInterface& operator=(DtnExampleInterface&& rh) {
                    if (this!=&rh) {
                        // do some TODO
                    }
                    return *this;
                }

            protected :
                size_t th_ = 0;
                uint32_t random_seed_;
                uint32_t node_number_;
                dtn_time_t simulation_duration_;
                vector<Ptr<DtnApp>> apps_;
                bool print_wifi_log_;
                std::string trace_file_;
                std::string teg_file_;
                std::string log_file_;
                std::ofstream file_stream_;
                map<int, int> config_storage_max_;
                NodeContainer nodes_container_;
                NetDeviceContainer net_devices_container_;
                Ipv4InterfaceContainer ip_interface_container_;
                DtnApp::RoutingMethod ex_rm_;

                vector<Adob> CreateAdjacentList();
                virtual void CreateNodes();
                virtual void CreateDevices();
                virtual void InstallInternetStack();
                virtual std::unique_ptr<RoutingMethodInterface> CreateRouting(DtnApp& dtn);

                virtual void InstallApplications();
                virtual void ScheduleTask();
                // call LogCounter to counter for simulation time
                void LogCounter(int);
        };   

    } /* ns3dtnbit
    */ 
} /* ns3  */ 
#endif /* ifndef NS3DTN_BIT_EXAMPLE_INTERFACE_H */
