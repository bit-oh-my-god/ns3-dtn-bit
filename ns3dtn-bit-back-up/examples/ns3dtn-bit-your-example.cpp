/*
 * 1. generate your trace file
 * 2. define your example, mainly how node are connected,
 *  you can override it, but must implement ReportEx(),
 *  !!Important it was designed to let you do it.
 * 3. run your example
 */
#include "ns3/ns3dtn-bit-helper.h"
#include "ns3/ns3dtn-bit-example-interface.h"
#include "ns3/ns3dtn-bit.h"

using namespace ns3;
namespace ns3 {
    namespace ns3dtnbit {
        class YourExample : public DtnExampleInterface {
            public :
                YourExample() : DtnExampleInterface() {
                    node_number_ = 2;
                    // simulation time should be less than trace_file_ time !Important
                    simulation_duration_ = 804;
                    print_log_boolean_ = true;
                    ex_rm_ = DtnApp::RoutingMethod::SprayAndWait;
                    // if you want do your method, add code in DtnApp, 
                    // TODO
                    // may be I can abstract one interface like 'routing method interface' for user to iheritance from.
                }
                void ReportEx(std::ostream& os) override {
                    os << "Here In DtnExampleInterface::ReportEx" << endl;
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
    //LogComponentEnableAll (LOG_PREFIX_TIME);
    //LogComponentEnableAll(LOG_PREFIX_NODE); 
    //

    assert(std::is_move_constructible<ns3dtnbit::YourExample>::value);
    assert(std::is_move_assignable<ns3dtnbit::YourExample>::value);
    std::unique_ptr<ns3dtnbit::DtnExampleInterface> exp(new ns3dtnbit::YourExample());
    auto runner = ns3dtnbit::DtnExampleRunner();
    runner.RunnerLoad(exp).RunIt(argc, argv);
    return 0;
}
