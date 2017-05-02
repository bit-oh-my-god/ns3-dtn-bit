/*
 * 1. generate your trace file and teg.txt
 * 2. define your example, mainly 3 parts of it
 *      a. constructor of YourExample
 *      b. CreateRouting()
 *      c. YouRouting()
 *          note that you do can use value of s,d as index of predecessor until listS is set to be the vertex container of Graph
 * 3. run your example
 */
#include "ns3/ns3dtn-bit-helper.h"
#include "ns3/ns3dtn-bit-example-interface.h"

using namespace ns3;
namespace ns3 {
    namespace ns3dtnbit {
        
        class YourExample : public DtnExampleInterface {
            public :
                YourExample() : DtnExampleInterface() {
                    const int schdule_for_n_trace = 6;
                    // simulation time should be less than trace_file_ time !Important
                    node_number_ = 10;           // change me!!
                    simulation_duration_ = 802;     // change me!!
                    print_wifi_log_ = false;
                    //ex_rm_ = DtnApp::RoutingMethod::Other;
                    ex_rm_ = DtnApp::RoutingMethod::TimeExpanded;
                    //ex_rm_ = DtnApp::RoutingMethod::CGR;
                    //ex_rm_ = DtnApp::RoutingMethod::SprayAndWait;
                    
                    if (schdule_for_n_trace == 3) {
                        node_number_ = 5;
                        simulation_duration_ = 802;
                    } else if (schdule_for_n_trace == 7) {
                        node_number_ = 11;
                        simulation_duration_ = 802;
                    } else if (schdule_for_n_trace == 6) {
                        node_number_ = 20;
                        simulation_duration_ = 802;
                    }
                }
                void ReportEx(std::ostream& os) override {
                    os << "Here In DtnExampleInterface::ReportEx" 
                        << "BundleTrace:node_number_=" << node_number_ 
                        << ";simulation_duration_=" << simulation_duration_ << endl;
                }

                void ScheduleTask() override {
                    int sch_size = 345;
                    auto handy_func = [sch_size, this](double sch_time, int dstnode, int i) {
                        std::cout << "bundle send schedule: time=" << sch_time << ";node-" << i << "send " << sch_size << " size-pkt to node-" << dstnode << std::endl;
                        this->apps_[i]->ScheduleTx(Seconds(sch_time), dstnode, sch_size);
                    };
                    const int schdule_for_n_trace = 6;
                    if (schdule_for_n_trace == 3) {
                        // this three would be transmit to node-1, and carry to node-2
                        handy_func(3, 9, 4);
                        handy_func(5.0, 2, 0);
                        handy_func(22.0, 2, 0);
                        // this two would wait node-1 to transmit
                        handy_func(54.0, 0, 2);
                        handy_func(66.0, 1, 2);
                    } else if (schdule_for_n_trace == 7) {
                        handy_func(22.1, 10, 1);
                        handy_func(70.1, 10, 2);
                        handy_func(32.1, 10, 1);
                        handy_func(42.1, 10, 5);
                    } else if (schdule_for_n_trace == 6) {
                        DtnExampleInterface::ScheduleTask();
                    } else {
                        cout << "WARN: can't find schdule, define it or use default random one by comment this method" << endl;
                    }
                }
        };

    } /* ns3dtnbit */ 

} /* ns3  */ 

int main(int argc, char *argv[]) {
    //!important LOG control
    //LogComponentEnable ("DtnRunningLog",LOG_LEVEL_DEBUG);
    //LogComponentEnable ("DtnRunningLog",LOG_LEVEL_INFO);
    //LogComponentEnable ("DtnRunningLog",LOG_LEVEL_LOGIC);
    LogComponentEnable ("DtnRunningLog", LOG_LEVEL_ALL);
    LogComponentEnable("Ns2MobilityHelper", LOG_LEVEL_INFO);

    assert(std::is_move_constructible<ns3dtnbit::YourExample>::value);
    assert(std::is_move_assignable<ns3dtnbit::YourExample>::value);
    std::unique_ptr<ns3dtnbit::DtnExampleInterface> exp(new ns3dtnbit::YourExample());
    auto runner = ns3dtnbit::DtnExampleRunner();
    runner.RunnerLoad(exp).RunIt(argc, argv);
    return 0;
}
