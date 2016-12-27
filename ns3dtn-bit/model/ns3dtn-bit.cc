/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3dtn-bit.h"

namespace ns3 {

    namespace ns3dtnbit {
        
        void DtnApp::SendHello(Ptr<Socket> socket, double simulation_end_time, Time hello_interval, bool hello_right_now_boolean) {

        }

        void DtnApp::CheckBuffer(CheckState check_state) {

            bool real_send_boolean = false;

            // remove expired antipackets and bundles
            if (check_state == State_2) {

            }

            // prepare and set send_bundle_boolean
            if (mac_queue->GetSize() < 2) {
                if (check_state == State_2) {
                    
                } else {

                }
            } else {
                check_state = State_0;
            }

            // do real send stuff
            if (real_send_boolean) {

            }

            // switch check_state and reschedule
            switch (check_state) {
                case State_0 : {
                                   break;
                               }
                case State_1 : {
                                   break;
                               }
                case State_2 : {
                                   break;
                               }
                case State_3 : {
                                   break;
                               }
                default : {
                              break;
                          }
            }
        }
    } /* ns3dtnbit */ 
/* ... */


}

