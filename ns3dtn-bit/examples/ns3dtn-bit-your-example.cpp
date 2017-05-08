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
                    // simulation time should be less than trace_file_ time !Important
                    node_number_ = 10;           // change me!!
                    simulation_duration_ = 802;     // change me!!
                    print_wifi_log_ = false;
                    //ex_rm_ = DtnApp::RoutingMethod::Other;
                    //ex_rm_ = DtnApp::RoutingMethod::TimeExpanded;
                    ex_rm_ = DtnApp::RoutingMethod::CGR;
                    //ex_rm_ = DtnApp::RoutingMethod::SprayAndWait;

                    {
                        // following code is just handy used, not essential, you can do it yourself
                        const int schdule_for_n_trace = 9;
                        if (schdule_for_n_trace == 3) {
                            node_number_ = 5;
                            simulation_duration_ = 802;
                        } else if (schdule_for_n_trace == 9) {
                            node_number_ = 11;
                            simulation_duration_ = 802;
                        } else if (schdule_for_n_trace == 7) {
                            node_number_ = 11;
                            simulation_duration_ = 802;
                        } else if (schdule_for_n_trace == 6) {
                            node_number_ = 20;
                            simulation_duration_ = 802;
                        } else if (schdule_for_n_trace == 4) {
                            node_number_ = 15;
                            simulation_duration_ = 802;
                        } else if (schdule_for_n_trace == 8) {
                            node_number_ = 15;
                            simulation_duration_ = 802;
                        } else {
                            cout << "warn: " << __FILE__ << __LINE__ << endl;
                        }
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
                    {
                        // following code is just handy used, not essential, you can do it yourself
                        const int schdule_for_n_trace = 9;
                        if (schdule_for_n_trace == 3) {
                            // line
                            // this three would be transmit to node-1, and carry to node-2
                            handy_func(3, 1, 3);
                            handy_func(5.0, 2, 0);
                            handy_func(22.0, 2, 0);
                            // this two would wait node-1 to transmit
                            handy_func(54.0, 0, 2);
                            handy_func(66.0, 1, 2);
                        } else if (schdule_for_n_trace == 7) {
                            // cycle
                            handy_func(22.1, 10, 1);
                            handy_func(70.1, 10, 2);
                            handy_func(32.1, 10, 1);
                            handy_func(42.1, 10, 5);
                            handy_func(72.1, 10, 4);
                            handy_func(75.1, 10, 4);
                            handy_func(73.1, 10, 4);
                            handy_func(71.1, 10, 4);
                            handy_func(70.1, 10, 4);
                            handy_func(78.1, 10, 4);
                            handy_func(255.1, 4, 0);
                            handy_func(256.1, 4, 1);
                            handy_func(257.1, 4, 2);
                            handy_func(258.1, 4, 3);
                        } else if (schdule_for_n_trace == 9) {
                            // cycle with busy traffic
                            handy_func(22.1, 10, 1);
                            handy_func(70.1, 10, 2);
                            handy_func(32.1, 10, 1);
                            handy_func(42.1, 10, 5);
                            handy_func(72.1, 10, 4);
                            handy_func(75.1, 10, 4);
                            handy_func(73.1, 10, 4);
                            handy_func(71.1, 10, 4);
                            handy_func(70.1, 10, 4);
                            handy_func(78.1, 10, 4);
                            handy_func(255.1, 4, 0);
                            handy_func(256.1, 4, 1);
                            handy_func(257.1, 4, 2);
                            handy_func(258.1, 4, 3);
                            //
                            handy_func(257.1, 4, 2);
                            handy_func(257.1, 4, 2);
                            handy_func(257.1, 4, 2);
                            handy_func(258.1, 4, 3);
                            handy_func(258.1, 4, 3);
                            handy_func(258.1, 4, 3);
                            handy_func(101.1, 3, 1);
                            handy_func(111.1, 4, 3);
                            handy_func(120.1, 1, 2);
                            handy_func(130.1, 0, 1);
                            handy_func(130.1, 0, 1);
                            handy_func(130.1, 0, 1);
                            handy_func(150.1, 2, 3);
                            handy_func(150.1, 2, 3);
                            handy_func(150.1, 2, 3);
                            handy_func(170.1, 1, 5);
                            handy_func(150.1, 2, 3);
                            handy_func(180.1, 5, 4);
                            handy_func(180.1, 5, 4);
                            handy_func(180.1, 5, 4);
                            handy_func(190.1, 4, 3);
                            handy_func(200.1, 0, 6);
                            handy_func(200.1, 0, 6);
                            handy_func(200.1, 0, 6);
                            handy_func(210.1, 0, 7);
                            handy_func(222.1, 1, 3);
                            handy_func(222.1, 1, 3);
                            handy_func(222.1, 1, 3);
                            handy_func(232.1, 0, 8);
                            handy_func(242.1, 2, 3);
                            handy_func(255.1, 4, 1);
                            handy_func(255.1, 4, 1);
                            handy_func(255.1, 4, 1);
                            handy_func(262.1, 3, 2);
                            handy_func(272.1, 0, 4);
                            handy_func(282.1, 3, 5);
                            handy_func(282.1, 3, 5);
                            handy_func(282.1, 3, 5);
                            handy_func(292.1, 1, 2);
                            handy_func(300.1, 2, 3);
                            handy_func(310.1, 6, 2);
                            handy_func(320.1, 1, 0);
                            handy_func(282.1, 3, 5);
                            handy_func(330.1, 5, 7);
                            handy_func(340.1, 4, 2);
                            handy_func(350.1, 5, 2);
                            handy_func(360.1, 5, 0);
                            handy_func(370.1, 2, 3);
                            handy_func(380.1, 2, 4);
                            handy_func(390.1, 5, 1);
                            handy_func(400.1, 5, 0);
                            handy_func(420.1, 2, 1);
                            handy_func(440.1, 0, 1);
                            handy_func(460.1, 1, 7);
                            handy_func(470.1, 2, 1);
                            handy_func(480.1, 3, 0);
                            handy_func(490.1, 5, 4);
                            handy_func(550.1, 5, 4);
                            handy_func(600.1, 5, 1);
                            handy_func(700.1, 5, 2);
                        } else if (schdule_for_n_trace == 6) {
                            // moving group
                            DtnExampleInterface::ScheduleTask();
                        } else if (schdule_for_n_trace == 4) {
                            // random
                            DtnExampleInterface::ScheduleTask();
                        } else if (schdule_for_n_trace == 8) {
                            // random-2
                            DtnExampleInterface::ScheduleTask();
                        } else {
                            cout << "WARN: can't find schdule, define it or use default random one " << endl;
                        }
                    }
                }
        };

    } /* ns3dtnbit */ 

} /* ns3  */ 

int main(int argc, char *argv[]) {
    //!important LOG control
    LogComponentEnable ("DtnRunningLog",LOG_LEVEL_DEBUG);
    //LogComponentEnable ("DtnRunningLog",LOG_LEVEL_INFO);
    //LogComponentEnable ("DtnRunningLog",LOG_LEVEL_LOGIC);
    //LogComponentEnable ("DtnRunningLog", LOG_LEVEL_ALL);
    LogComponentEnable("Ns2MobilityHelper", LOG_LEVEL_INFO);

    assert(std::is_move_constructible<ns3dtnbit::YourExample>::value);
    assert(std::is_move_assignable<ns3dtnbit::YourExample>::value);
    std::unique_ptr<ns3dtnbit::DtnExampleInterface> exp(new ns3dtnbit::YourExample());
    auto runner = ns3dtnbit::DtnExampleRunner();
    runner.RunnerLoad(exp).RunIt(argc, argv);
    return 0;
}
