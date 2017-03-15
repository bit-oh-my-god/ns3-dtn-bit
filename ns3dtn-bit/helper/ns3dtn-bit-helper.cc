/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3dtn-bit-helper.h"

namespace ns3 {

    namespace ns3dtnbit
    {
        DtnExampleRunner::DtnExampleRunner() {

        }

        DtnExampleRunner& DtnExampleRunner::RunnerLoad(std::unique_ptr<DtnExampleInterface>& ex_p) {
            hold_ex_p = std::move(ex_p);
            return *this;
        }

        void DtnExampleRunner::RunIt(int argc, char** argv) {
            hold_ex_p->Configure(argc, argv);
            hold_ex_p->Run();
            hold_ex_p->Report(std::cout);
        }
    } /* ns3dtnbit */ 
/* ... */

}

