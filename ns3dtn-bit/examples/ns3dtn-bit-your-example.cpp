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
#include <algorithm>
#include <vector>
#include "ns3/ns3dtn-bit-example-interface.h" 

using namespace ns3;

namespace ns3 {
    namespace ns3dtnbit {

        class YourExample : public DtnExampleInterface {
            public :
                YourExample(int nn, std::string method) : DtnExampleInterface() {
                    SetSchduleN(nn);
                    // simulation time should be less than trace_file_ time !Important
                    node_number_ = -1;           // change me!!
                    simulation_duration_ = -1;     // change me!!
                    print_wifi_log_ = false;
                    if (method == "Spray") {
                        ex_rm_ = DtnApp::RoutingMethod::SprayAndWait;
                    } else if (method == "Heuristic") {
                        ex_rm_ = DtnApp::RoutingMethod::Other;
                    } else if (method == "TEG") {
                        ex_rm_ = DtnApp::RoutingMethod::TimeExpanded;
                    } else if (method == "CGR") {
                        ex_rm_ = DtnApp::RoutingMethod::CGR;
                    } else if (method == "DirectForward") {
                        ex_rm_ = DtnApp::RoutingMethod::DirectForward;
                    } else if (method == "QM") {
                        ex_rm_ = DtnApp::RoutingMethod::QM;
                    } else {
                        std::cerr << "can't find routing method name or don't assign one." << std::endl;
                        abort();
                    }

                    {                           
                        // assign node_number_ and simulation_duration_ by different scenario_number                
                        auto cycle_traffic_number = std::vector<int>{101, // name in json is loopcycle101
                        102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,};
                        auto tx1_traffic_number = std::vector<int>{201, // tx1201
                        202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,};
                        auto ra1_traffic_number = std::vector<int>{301, // ran301
                        302,303,304,305,306,307,308,309,310,311,312,313,314,315,316,};
                        auto switch_traffic_number = std::vector<int>{401, // switch401
                        402,403,404,405,406,407,408,409,410,411,412,413,414,415,416,};
                        auto current_traffic_number = std::vector<int>{501, // current501
                        502,503,504,505,506,507,508,509,510,511,512,513,514,515,516,};
                        auto loadbalance_traffic_number = std::vector<int>{601, // load601
                        602,603,604,605,606,607,608,609,610,611,612,613,614,615,616,};
                        
                        if (std::find(cycle_traffic_number.begin(), cycle_traffic_number.end(), scenario_number) != cycle_traffic_number.end()) { 
                            // 0100_current_trace.tcl + 0100_teg.txt
                            node_number_ = 12;
                            simulation_duration_ = 802;
                        } else if (std::find(tx1_traffic_number.begin(), tx1_traffic_number.end(), scenario_number) != tx1_traffic_number.end()) { 
                            // 0200_current_trace.tcl + 0200_teg.txt
                            node_number_ = 7;
                            simulation_duration_ = 1000;
                        } else if (std::find(ra1_traffic_number.begin(), ra1_traffic_number.end(), scenario_number) != ra1_traffic_number.end()) { 
                            // 0300_current_trace.tcl + 0300_teg.txt
                            node_number_ = 6;
                            simulation_duration_ = 1000;
                        } else if (std::find(switch_traffic_number.begin(), switch_traffic_number.end(), scenario_number) != switch_traffic_number.end()) { 
                            // 0400_current_trace.tcl + 0400_teg.txt
                            node_number_ = 8;
                            simulation_duration_ = 1000;
                        } else if (std::find(current_traffic_number.begin(), current_traffic_number.end(), scenario_number) != current_traffic_number.end()) { 
                            // 0500_current_trace.tcl + 0500_teg.txt
                            node_number_ = 10;
                            simulation_duration_ = 2000;
                        } else if (std::find(loadbalance_traffic_number.begin(), loadbalance_traffic_number.end(), scenario_number) != loadbalance_traffic_number.end()) { 
                            // 0600_current_trace.tcl + 0600_teg.txt
                            node_number_ = 10;
                            simulation_duration_ = 2000;
                        } else {
                            cout << "warn: " << __FILE__ << __LINE__ << endl;
                            std::abort();
                        }
                    }
                } 

                void InitStorage() override {
                    bool bool_var = true;
                    if (bool_var) {
                      if (scenario_number / 100 == 4) {
                        config_storage_max_[0] = 12;
                        config_storage_max_[1] = 12;
                        config_storage_max_[2] = 40;
                        config_storage_max_[3] = 50;
                        config_storage_max_[4] = 50;
                        config_storage_max_[5] = 100;
                        config_storage_max_[6] = 12;
                        config_storage_max_[7] = 100;
                      } else if (scenario_number / 100 == 6) {
                        config_storage_max_[3] = 10;
                        config_storage_max_[5] = 20;
                        config_storage_max_[6] = 20;
                        config_storage_max_[7] = 30;
                        config_storage_max_[8] = 30;
                        config_storage_max_[9] = 10;
                      }
                    }
                }

                void ReportEx(std::ostream& os) override;

                void ScheduleTask() override {
                    int headerlenth = 80; //  有hops， 变长
                    int sch_size = NS3DTNBIT_HYPOTHETIC_CACHE_FACTOR - headerlenth;
                    NS_ASSERT(sch_size > 400);
                    /*
                    auto handy_func = [sch_size, this](double sch_time, int dstnode, int i) {
                        std::cout << "bundle send schedule: time=" << sch_time << ";node-" << i << "send " << sch_size << " size-pkt to node-" << dstnode << std::endl;
                        this->apps_[i]->ScheduleTx(Seconds(sch_time), dstnode, sch_size);
                    };
                    */
                    auto handy_func_x = [sch_size, this](double sch_time, int dstnode, int srcnode, int times, double interval=5.0) {
                        for (double rt = sch_time; rt < sch_time + times * interval; rt += interval) {
                            std::cout << "bundle send schedule: time=" << rt << ";node-" << srcnode << "send " << sch_size << " size-pkt to node-" << dstnode << std::endl;
                            this->apps_[srcnode]->ScheduleTx(Seconds(rt), dstnode, sch_size);
                        }
                    };
                    {
                        // Schedule differently by scenario_number
                        // following code is just handy used, not essential, you can do it yourself
                        if (scenario_number > 100) {
                            std::srand(1102);
                            auto amount = scenario_number % 100;
                            if ((scenario_number / 100) == 3) {
                                int rand1 = std::rand() % 6;
                                int rand2 = std::rand() % 6;
                                int rand3 = std::rand() % 6;
                                while (rand1 == rand2) {rand2 = std::rand() % 6;}
                                while (rand2 == rand3 || rand1 == rand3) {rand3 = std::rand() % 6;}
                                handy_func_x(100, rand1, rand2, 10 * amount, 27.0 / double(amount));
                                handy_func_x(100, rand2, rand3, 10 * amount, 27.0 / double(amount));
                                handy_func_x(100, rand3, rand1, 10 * amount, 27.0 / double(amount));
                            } else if ((scenario_number / 100) == 2) {
                                handy_func_x(100, 5, 6, 5 * amount, 15.0 / double(amount));
                                handy_func_x(300, 6, 5, 5 * amount, 14.0 / double(amount));
                                handy_func_x(500, 5, 6, 5 * amount, 24.0 / double(amount));
                                handy_func_x(700, 5, 6, 5 * amount, 24.0 / double(amount));
                            } else if ((scenario_number / 100) == 1) {
                                handy_func_x(200, 4, 10, 16 * amount, 21.0 / double(amount));
                                handy_func_x(100, 0, 10, 16 * amount, 20.0 / double(amount));
                            } else if ((scenario_number / 100) == 4) {
                                // switch401
                                handy_func_x(10, 3, 2, 30 * amount, 16.0 / double(amount));
                            } else if ((scenario_number / 100) == 5) {
                                // current501
                                handy_func_x(10, 0, 2, 30 * amount, 16.0 / double(amount));
                            } else if ((scenario_number / 100) == 6) {
                                // loadbal601
                                handy_func_x(1.0, 2, 0, 10 * amount, 90.0 / double(amount));
                            } else if (scenario_number > 500) {
                                cout << "warn: " << __FILE__ << __LINE__ << endl;
                                abort();
                            } else {
                                cout << "warn: " << __FILE__ << __LINE__ << endl;
                                abort();
                            }
                        } else {
                            cout << "WARN: can't find schdule, define it or use default random one " << endl;
                            std::abort();
                        }
                    }
                }

                void SetSchduleN(size_t n) override {
                    assert(n != 0);
                    scenario_number = n;
                    set_th(scenario_number);
                }

            private :
                //size_t schdule_for_n_trace = 0; // the number of example you are running, different example number may have different packet schedule and node moving
                size_t scenario_number = 0; // refactor from schdule_for_n_trace to this
        };

        void YourExample::ReportEx(std::ostream& os) {
                    os << "Here In DtnExampleInterface::ReportEx , good end of simulation\n" 
                        << "BundleTrace:node_number_=" << node_number_ 
                        << ";simulation_duration_=" << simulation_duration_ << endl;
                }

    } /* ns3dtnbit */ 
} /* ns3  */ 

int main(int argc, char *argv[]) {
    //!important LOG control
    LogComponentEnable ("DtnCGRRouting",LOG_LEVEL_DEBUG);
    //LogComponentEnable ("DtnCGRRouting",LOG_LEVEL_INFO);

    LogComponentEnable ("DtnCGRQMRouting",LOG_LEVEL_DEBUG);
    //LogComponentEnable ("DtnCGRQMRouting",LOG_LEVEL_INFO);

    //LogComponentEnable ("DtnRouting",LOG_LEVEL_DEBUG);
    //LogComponentEnable ("DtnRouting",LOG_LEVEL_INFO);

    LogComponentEnable ("DtnApp",LOG_LEVEL_DEBUG);
    //LogComponentEnable ("DtnApp",LOG_LEVEL_INFO);
    //LogComponentEnable ("DtnApp",LOG_LEVEL_LOGIC);
    //LogComponentEnable ("DtnApp", LOG_LEVEL_ALL);

    LogComponentEnable("Ns2MobilityHelper", LOG_LEVEL_INFO);
    //LogComponentEnable("UdpSocket", LOG_LEVEL_INFO);
    //LogComponentEnable("UdpL4Protocol", LOG_LEVEL_INFO);

    assert(std::is_move_constructible<ns3dtnbit::YourExample>::value);
    assert(std::is_move_assignable<ns3dtnbit::YourExample>::value);
    int n = 102;  // the number of YourExample you are running
    std::string dtn_route_would_be_used = "Spray";

    if (argc == 1) {
        std::cout << "please give some argument to identify the example number you want to run. If you were building this, let it go and remenber to run the excutable with argument." << std::endl;
        //std::abort();
    } else {
        auto ss = std::string(argv[1]);
        dtn_route_would_be_used = std::string(argv[2]);
        n = stoi(ss);
        std::cout << "example number(scenario_number) is '" << n << "'" << std::endl;
    }
    std::unique_ptr<ns3dtnbit::DtnExampleInterface> exp(
        new ns3dtnbit::YourExample(n, dtn_route_would_be_used)
        );
    auto runner = ns3dtnbit::DtnExampleRunner();
    runner.RunnerLoad(exp).RunIt(argc, argv);
    return 0;
}
