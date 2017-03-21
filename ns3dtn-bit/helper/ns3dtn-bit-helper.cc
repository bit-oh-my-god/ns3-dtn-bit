/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3dtn-bit-helper.h"

namespace ns3 {

    namespace ns3dtnbit
    {
        DtnExampleRunner::DtnExampleRunner() {

        }

        /* TODO
         * add some configure check code here
         */
        DtnExampleRunner& DtnExampleRunner::RunnerLoad(std::unique_ptr<DtnExampleInterface>& ex_p) {
            hold_ex_p = std::move(ex_p);
            return *this;
        }

        void DtnExampleRunner::RunIt(int argc, char** argv) {
            hold_ex_p->ConfigureEx(argc, argv);
            hold_ex_p->RunEx();
            hold_ex_p->ReportEx(std::cout);
        }
    } /* ns3dtnbit */ 
/* ... */

}

