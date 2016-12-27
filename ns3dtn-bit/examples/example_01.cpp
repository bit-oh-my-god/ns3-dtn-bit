#include "../model/common_header.h"
#include "ns3/core-module.h"
#include "ns3/ns3dtn-bit-module.h"
#include "ns3/ns3dtn-bit-helper.h"

namespace ns3 {
    namespace ns3dtnbit {

        class Example_01 {
            public:
                Example_01 ();
                virtual ~Example_01 ();
                bool Configure();
                void Run();

            private:

        };

        Example_01::Example_01() {

        }

        bool Example_01::Configure() {
            return true;

        }

        void Example_01::Run() {

        }

    } /* ns3dtnbit */ 


    int main() {
        // support c++11
#if (__cplusplus==201103L)
        auto test = ns3::ns3dtnbit::Example_01();
        std::cout << "we can use c++11" << std::endl;
#else 
        ns3::ns3dtnbit::Example_01 test();
        std::cout << "we can't use c++11 " << std::endl;
#endif

        if (! test.Configure()) {
            NS_FATAL_ERROR("configure fail");
        }

        test.Run();
        return 0;
    }
} /* ns3  */ 

int main() {
    return ns3::main();
}
