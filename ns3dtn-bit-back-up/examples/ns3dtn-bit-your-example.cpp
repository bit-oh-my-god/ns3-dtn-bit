/*
 * 1. generate your trace file
 * 2. define your example, mainly how node are connected,
 *  you can override it, but must implement ReportEx(),
 * 3. run your example
 */
#include "ns3/ns3dtn-bit-helper.h"
#include "ns3/ns3dtn-bit-example-interface.h"
#include "ns3/ns3dtn-bit.h"

using namespace ns3;
namespace ns3 {
    namespace ns3dtnbit {

        class YouRouting : public RoutingMethodInterface {
            public :
                YouRouting(DtnApp& dp) : RoutingMethodInterface(dp) {}
                int DoRoute(int s, int d) override {
                    const DtnApp::Adob& ref_adob = RoutingMethodInterface::get_adob();
                    auto g = ref_adob.get_graph_for_now();
                    std::cout << "acess adob, In your method, abort!" << std::endl;
                    std::abort();
                }
        };

        class YourExample : public DtnExampleInterface {
            public :
                YourExample() : DtnExampleInterface() {
                    node_number_ = 5;
                    // simulation time should be less than trace_file_ time !Important
                    simulation_duration_ = 802;
                    print_log_boolean_ = true;
                    ex_rm_ = DtnApp::RoutingMethod::SprayAndWait;
                }
                void ReportEx(std::ostream& os) override {
                    os << "Here In DtnExampleInterface::ReportEx" << endl;
                }
                std::unique_ptr<RoutingMethodInterface> CreateRouting(DtnApp& dtn) override {
                    auto p = new YouRouting(dtn);
                    return std::unique_ptr<RoutingMethodInterface>(p);
                }
        };

    } /* ns3dtnbit */ 

} /* ns3  */ 

int main(int argc, char *argv[]) {
    //!important LOG control
    //LogComponentEnable ("DtnRunningLog",LOG_LEVEL_WARN);
    //LogComponentEnable ("DtnRunningLog",LOG_LEVEL_DEBUG);
    LogComponentEnable ("DtnRunningLog",LOG_LEVEL_INFO);
    //LogComponentEnable ("DtnRunningLog",LOG_LEVEL_LOGIC);

    assert(std::is_move_constructible<ns3dtnbit::YourExample>::value);
    assert(std::is_move_assignable<ns3dtnbit::YourExample>::value);
    std::unique_ptr<ns3dtnbit::DtnExampleInterface> exp(new ns3dtnbit::YourExample());
    auto runner = ns3dtnbit::DtnExampleRunner();
    runner.RunnerLoad(exp).RunIt(argc, argv);
    return 0;
}
