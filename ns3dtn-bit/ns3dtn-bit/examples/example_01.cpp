#include "../model/common_header.h"
#include "ns3/core-module.h"
#include "ns3/ns3dtn-bit-module.h"
#include "ns3/ns3dtn-bit-helper.h"

int main() {
        // support c++11
#if (__cplusplus==201103L)
        auto test = ns3::ns3dtnbit::DtnExample();
        std::cout << "we can use c++11" << std::endl;
#else 
        ns3::ns3dtnbit::Example_01 test();
        std::cout << "we can't use c++11 " << std::endl;
#endif
        test.Configure();
        test.Run();
        test.Report(std::cout);
        return 0;
    }
}
