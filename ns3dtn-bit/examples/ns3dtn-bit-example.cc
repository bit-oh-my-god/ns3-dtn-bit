/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/ns3dtn-bit-module.h"
#include "ns3/ns3dtn-bit-helper.h"
#include "../model/common_header.h"

using namespace ns3;

int main (int argc, char *argv[]) {
#if (__cplusplus==201103L)
    std::cout << "==========================================" << std::endl;
    std::cout << "before configure, we can use c++11" << std::endl;
    std::cout << "==========================================" << std::endl;
#else
#endif
    LogComponentEnable ("DtnRunningLog",LOG_LEVEL_DEBUG);
    LogComponentEnableAll (LOG_PREFIX_TIME);
    LogComponentEnableAll(LOG_PREFIX_NODE); 
    ns3dtnbit::DtnExample test = ns3dtnbit::DtnExample();
    test.Configure(argc, argv);
    test.Run();
    test.Report(std::cout);
    return 0;
}


