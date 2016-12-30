/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3dtn-bit.h"
#include "./common_header.h"

namespace ns3 {

    namespace ns3dtnbit {
        
        void DtnApp::SendHello(Ptr<Socket> socket, double simulation_end_time, Time hello_interval, bool hello_right_now_boolean) {

        }

        void DtnApp::RemoveExpiredBAQ() {
            uint32_t pkt_number = 0, n = 0;
            // remove expired bundle queue packets
            pkt_number = daemon_bundle_queue_->GetNPackets();
            for (int i = 0; i < pkt_number; ++i) {
                Ptr<Packet> p_pkt = daemon_bundle_queue_->Dequeue();
                BPHeader bp_header();
                p_pkt->RemoveHeader(bp_header);
                //if not expired
                if (((Simulator::Now().GetSeconds() - bp_header.get_src_time_stamp()) < 750.0) || (bp_header.get_hop_count() == 0)) {
                    p_pkt->AddHeader(bp_header);
                    daemon_bundle_queue_->Enqueue(p_pkt);
                } else {
                    for (int j = 0; j < neighbor_count_; j++) {
                        int m = 0, found_expired = 0, number = NS3DTNBIT_HYPOTHETIC_NEIGHBOR_BAQ_NUMBER_MAX;
                        while (m < number && found_expired == 0) {
                            // hop count equals zero means that it came from a neighbor
                            if (bp_header.get_source_seqno() == neighbor_info_vec_[j].info_sent_bp_seqnof_vec_[m]) {
                                found_expired = 1;
                            } else {
                                m++;
                            }
                            if (found_expired == 1) {
                                UpdateNeighborInfo(1, n, j);
                            }
                        }
                    }
                }
            }
        }

        void DtnApp::UpdateNeighborInfo(int which_info, int which_neighbor, int which_pkt_index) {
            switch (which_info) {
                // info_baq_seqnof_vec_
                case 0 : {
                             break;
                         }
                // info_sent_bp_seqnof_vec_
                case 1 : {
                             break;
                         }
                // info_sent_ap_seqnof_vec_
                case 2 : {
                             break;
                         }
                default : break;
            }
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

