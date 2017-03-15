/*
 * 1. generate your trace file
 * 2. define your example, mainly how node are connected
 * 3. run your example
 */
#include "ns3/ns3dtn-bit-helper.h"
#include "ns3/ns3dtn-bit.h"

using namespace ns3;
namespace ns3 {
    namespace ns3dtnbit {
        class YourExample : public DtnExampleInterface {
            public :
                YourExample() : DtnExampleInterface() {

                }
            private :
                void CreateNodes() override {

                }
                void CreateDevices() override {

                }
                void InstallInternetStack() override {

                }
                void InstallApplications() override {

                }
                // overwrite
                void Run() {
                    std::cout << "do nothing" << std::endl;
                }

        };

    } /* ns3dtnbit */ 

} /* ns3  */ 

int main(int argc, char *argv[]) {
    assert(std::is_move_constructible<ns3dtnbit::DtnExampleInterface>::value);
    assert(std::is_move_assignable<ns3dtnbit::DtnExampleInterface>::value);
    std::unique_ptr<ns3dtnbit::DtnExampleInterface> exp(new ns3dtnbit::YourExample());
    auto runner = ns3dtnbit::DtnExampleRunner();
    runner.RunnerLoad(exp).RunIt(argc, argv);
    return 0;
}
