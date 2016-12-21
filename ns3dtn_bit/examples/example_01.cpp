#include "ns3/core-module.h"
#include "ns3/ns3dtn_bit"

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
    
} /* ns3  */ 

int main() {
    // support c++11
#if (__cplusplus==201103L)
    auto test = ns3::ns3dtn_bit::Example_01();
#else 
    ns3::ns3dtn_bit::Example_01 test();
#endif

    if (! test.Configure()) {
        NS_FATAL_ERROR("configure fail");
    }

    test.Run();
}
