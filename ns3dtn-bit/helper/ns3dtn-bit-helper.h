/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef NS3DTN_BIT_HELPER_H
#define NS3DTN_BIT_HELPER_H

#include <memory>
#include "ns3/ns3dtn-bit.h"
#include "ns3/ns3dtn-bit-example-interface.h"

using std::unique_ptr;

namespace ns3 {
    namespace ns3dtnbit
    {
        class DtnExampleRunner {
            public :
                DtnExampleRunner();
                DtnExampleRunner& RunnerLoad(std::unique_ptr<DtnExampleInterface>& ex_p);
                void RunIt(int argc, char** argv);
            private :
                std::unique_ptr<DtnExampleInterface> hold_ex_p;
        };
    } /* ns3dtnbit */ 

/* ... */

}

#endif /* NS3DTN_BIT_HELPER_H */

