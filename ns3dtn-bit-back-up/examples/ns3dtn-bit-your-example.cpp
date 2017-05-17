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
                    node_number_ = 12;           // change me!!
                    simulation_duration_ = 802;     // change me!!
                    print_wifi_log_ = false;
                    //ex_rm_ = DtnApp::RoutingMethod::Other;
                    //ex_rm_ = DtnApp::RoutingMethod::TimeExpanded;
                    //ex_rm_ = DtnApp::RoutingMethod::CGR;
                    ex_rm_ = DtnApp::RoutingMethod::SprayAndWait;

                    {
                        // following code is just handy used, not essential, you can do it yourself
                        // 
                        // FIXME 
                        const int schdule_for_n_trace = 25; // one schdule_for_n_trace correspond to one node moving and one task plan
                        if (schdule_for_n_trace == 3) {
                            node_number_ = 5;
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
                        } else if (schdule_for_n_trace == 10    // cycle-traffic1
                                || schdule_for_n_trace == 17    // cycle-traffic2
                                || schdule_for_n_trace == 9     // cycle-traffic3
                                || schdule_for_n_trace == 11    // cycle-traffic4
                                || schdule_for_n_trace == 12    // cycle-traffic5
                                || schdule_for_n_trace == 15    // cycle-traffic6
                                || schdule_for_n_trace == 16    // cycle-traffic7
                                || schdule_for_n_trace == 19    // cycle-traffic8
                                || schdule_for_n_trace == 20    // cycle-traffic9
                                || schdule_for_n_trace == 21    // cycle-traffic10
                                || schdule_for_n_trace == 22    // cycle-traffic11
                                || schdule_for_n_trace == 23    // cycle-traffic12
                                || schdule_for_n_trace == 24    // cycle-traffic13
                                || schdule_for_n_trace == 25) { // cycle-traffic14
                            node_number_ = 12;
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
                    auto handy_func_x = [sch_size, this](double sch_time, int dstnode, int i, int times) {
                        for (double rt = sch_time; rt < sch_time + times; rt += 1.0) {
                            std::cout << "bundle send schedule: time=" << rt << ";node-" << i << "send " << sch_size << " size-pkt to node-" << dstnode << std::endl;
                            this->apps_[i]->ScheduleTx(Seconds(rt), dstnode, sch_size);
                        }
                    };
                    {
                        // following code is just handy used, not essential, you can do it yourself
                        auto handy_func_x1 = [this, handy_func]() {
                            handy_func(22.1, 10, 1);
                            handy_func(32.1, 10, 1);
                            handy_func(71.1, 10, 4);
                            handy_func(255.1, 4, 0);
                        };
                        auto handy_func_x2 = [this, handy_func]() {
                            handy_func(70.1, 10, 2);
                            handy_func(42.1, 10, 5);
                            handy_func(72.1, 10, 4);
                            handy_func(75.1, 10, 4);
                            handy_func(70.1, 10, 4);
                            handy_func(78.1, 10, 4);
                            handy_func(256.1, 11, 4);
                            handy_func(256.1, 11, 4);
                            handy_func(256.1, 11, 5);
                            handy_func(256.1, 4, 1);
                            handy_func(257.1, 4, 2);
                            handy_func(258.1, 4, 3);
                        };
                        auto handy_func_x3 = [this, handy_func, handy_func_x]() {
                            handy_func_x(178.1, 10, 5, 3);
                            handy_func_x(178.1, 10, 3, 3);
                            // 6
                            handy_func(12.0, 1, 6);
                            handy_func(120.0, 6, 3);
                            handy_func(240.0, 4, 6);
                            handy_func(367.0, 3, 6);
                            // 7
                            handy_func(13.0, 0, 7);
                            handy_func(166, 2, 7);
                            handy_func(378.0, 5, 7);
                            handy_func(480, 4, 7);
                            // 8
                            handy_func(8.0, 0, 8);
                            handy_func(95, 1, 8);
                            handy_func(200.0, 2, 8);
                            handy_func(300.0, 5, 8);
                            handy_func(430.0, 8, 4);
                            handy_func(560.0, 0, 8);
                        };
                        auto handy_func_x4 = [this, handy_func, handy_func_x]() {
                            handy_func_x(178.1, 5, 0, 4);
                            handy_func_x(8.1, 3, 0, 4);
                            handy_func_x(228.1, 11, 5, 4);
                            // 9
                            handy_func(70, 1, 9);
                            handy_func(140, 2, 9);
                            handy_func(210, 5, 9);
                            handy_func(270, 4, 9);
                            handy_func(400, 0, 9);
                        };
                        auto handy_func_x5 = [this, handy_func, handy_func_x]() {
                            // 6
                            handy_func(12.0, 6, 1);
                            handy_func(120.0, 3, 6);
                            handy_func(240.0, 6, 4);
                            handy_func(367.0, 6, 3);
                            // 7
                            handy_func(13.0, 7, 0);
                            handy_func(166, 7, 2);
                            handy_func(378.0, 7, 5);
                            handy_func(480, 7, 4);
                            // 8
                            handy_func(8.0, 8, 0);
                            handy_func(95, 8, 1);
                            handy_func(200.0, 8, 2);
                            handy_func(300.0, 8, 5);
                            handy_func(430.0, 8, 4);
                            handy_func(560.0, 8, 0);
                            // 9
                            handy_func(70, 9, 1);
                            handy_func(140, 9, 2);
                            handy_func(210, 9, 5);
                            handy_func(270, 9, 4);
                            handy_func(400, 9, 0);
                            handy_func(470, 9, 1);
                            handy_func(540, 9, 2);
                            handy_func(610, 9, 5);
                            handy_func(670, 9, 4);
                            handy_func(780, 9, 0);
                            handy_func(470, 1, 9);
                            handy_func(540, 2, 9);
                            handy_func(610, 5, 9);
                            handy_func(670, 4, 9);
                            handy_func(780, 0, 9);
                        };
                        auto handy_func_x6 = [this, handy_func_x]() {
                            handy_func_x(222, 0, 1, 4);
                            handy_func_x(333, 0, 2, 3);
                            handy_func_x(111, 10, 3, 3);
                        };
                        auto handy_func_x7 = [this, handy_func_x]() {
                            handy_func_x(300, 2, 4, 5);
                            handy_func_x(300, 1, 5, 4);
                        };
                        auto handy_func_x8 = [this, handy_func_x]() {
                            handy_func_x(300, 0, 10, 4);
                            handy_func_x(600, 0, 4, 2);
                            handy_func_x(200, 4, 1, 4);
                        };
                        //FIXME
                        const int schdule_for_n_trace = 25;
                        if (schdule_for_n_trace == 3) {
                            // line
                            // this three would be transmit to node-1, and carry to node-2
                            handy_func(3, 1, 3);
                            handy_func(5.0, 2, 0);
                            handy_func(22.0, 2, 0);
                            // this two would wait node-1 to transmit
                            handy_func(54.0, 0, 2);
                            handy_func(66.0, 1, 2);
                        } else if (schdule_for_n_trace == 10) {
                            // cycle with traffic-1
                            handy_func_x1();
                        } else if (schdule_for_n_trace == 17) {
                            // cycle with traffic-2
                            handy_func_x1();
                            handy_func_x2();
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
                            // cycle with traffic-3
                            handy_func_x1();
                            handy_func_x2();
                            handy_func_x3();
                        } else if (schdule_for_n_trace == 11) {
                            // cycle with traffic-4
                            handy_func_x2();
                            handy_func_x3();
                            handy_func_x4();
                            handy_func_x6();
                        } else if (schdule_for_n_trace == 12) {
                            // cycle with traffic-5
                            handy_func_x1();
                            handy_func_x2();
                            handy_func_x3();
                            handy_func_x4();
                            handy_func_x5();
                            handy_func_x6();
                        } else if (schdule_for_n_trace == 15) {
                            // cycle with traffic-6
                            handy_func_x1();
                            handy_func_x1();
                            handy_func_x2();
                            handy_func_x2();
                            handy_func_x3();
                            handy_func_x3();
                            handy_func_x4();
                            handy_func_x5();
                            handy_func_x6();
                            handy_func_x7();
                            handy_func_x7();
                        } else if (schdule_for_n_trace == 16) {
                            // cycle with traffic-7
                            handy_func_x1();
                            handy_func_x1();
                            handy_func_x1();
                            handy_func_x1();
                            handy_func_x2();
                            handy_func_x2();
                            handy_func_x2();
                            handy_func_x2();
                            handy_func_x3();
                            handy_func_x3();
                            handy_func_x4();
                            handy_func_x5();
                            handy_func_x6();
                            handy_func_x6();
                            handy_func_x7();
                            handy_func_x7();
                            handy_func_x7();
                            handy_func_x8();
                            handy_func_x8();
                        } else if (schdule_for_n_trace == 19) {
                            // cycle with traffic-8
                            for (int k = 8; k > 0; k--) {
                                handy_func_x1();
                                handy_func_x2();
                            }
                            for (int k = 2; k > 0; --k) {
                                handy_func_x3();
                                handy_func_x4();
                                handy_func_x5();
                                handy_func_x6();
                                handy_func_x7();
                                handy_func_x7();
                                handy_func_x8();
                            }
                        } else if (schdule_for_n_trace == 20) {
                            // cycle with traffic-9
                            for (int k = 12; k > 0; k--) {
                                handy_func_x1();
                                handy_func_x2();
                            }
                            for (int k = 3; k > 0; --k) {
                                handy_func_x3();
                                handy_func_x4();
                                handy_func_x5();
                                handy_func_x6();
                                handy_func_x7();
                                handy_func_x7();
                                handy_func_x8();
                            }
                        } else if (schdule_for_n_trace == 21) {
                            // cycle with traffic-10
                            for (int k = 16; k > 0; k--) {
                                handy_func_x1();
                                handy_func_x2();
                            }
                            for (int k = 4; k > 0; --k) {
                                handy_func_x3();
                                handy_func_x4();
                                handy_func_x5();
                                handy_func_x6();
                                handy_func_x7();
                                handy_func_x7();
                                handy_func_x8();
                            }
                        } else if (schdule_for_n_trace == 22) {
                            // cycle with traffic-11
                            for (int k = 25; k > 0; k--) {
                                handy_func_x1();
                                handy_func_x2();
                            }
                            for (int k = 6; k > 0; --k) {
                                handy_func_x3();
                                handy_func_x4();
                                handy_func_x5();
                                handy_func_x6();
                                handy_func_x7();
                                handy_func_x7();
                                handy_func_x8();
                            }
                        } else if (schdule_for_n_trace == 23) {
                            // cycle with traffic-12
                            for (int k = 33; k > 0; k--) {
                                handy_func_x1();
                                handy_func_x2();
                            }
                            for (int k = 8; k > 0; --k) {
                                handy_func_x3();
                                handy_func_x4();
                                handy_func_x5();
                                handy_func_x6();
                                handy_func_x7();
                                handy_func_x7();
                                handy_func_x8();
                            }
                        } else if (schdule_for_n_trace == 24) {
                            // cycle with traffic-13
                            for (int k = 44; k > 0; k--) {
                                handy_func_x1();
                                handy_func_x2();
                            }
                            for (int k = 11; k > 0; --k) {
                                handy_func_x3();
                                handy_func_x4();
                                handy_func_x5();
                                handy_func_x6();
                                handy_func_x7();
                                handy_func_x7();
                                handy_func_x8();
                            }
                        } else if (schdule_for_n_trace == 25) {
                            // cycle with traffic-14
                            for (int k = 55; k > 0; k--) {
                                handy_func_x1();
                                handy_func_x2();
                            }
                            for (int k = 15; k > 0; --k) {
                                handy_func_x3();
                                handy_func_x4();
                                handy_func_x5();
                                handy_func_x6();
                                handy_func_x7();
                                handy_func_x7();
                                handy_func_x8();
                            }
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
