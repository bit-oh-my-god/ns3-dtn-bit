/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3dtn-bit.h"
#include "./common_header.h"

namespace ns3 {

    namespace ns3dtnbit {

        void DtnApp::ToSendHello(Ptr<Socket> socket, double simulation_end_time, Time hello_interval, bool hello_right_now_boolean) {
            if (hello_right_now_boolean) {
                if (Simulator::Now().GetSeconds() > simulation_end_time) {
                    daemon_socket_handle_->Close();
                } else {
                    // prepare 'msg' to write into hello bundle
                    std::stringstream msg;
                    char tmp_msg_00[1024] = "";
                    if (congestion_control_method_ == DynamicControl) {
                        if ((drops_count_ == 0) && (congestion_control_parameter_ < 0.9)) {
                            congestion_control_parameter_ += 0.01;
                        } else {
                            if ((drops_count_ > 0) && (congestion_control_parameter_ > 0.5)) {
                                congestion_control_parameter_ *= 0.8;
                            } 
                            drops_count_ = 0;
                        }
                    }
                    int32_t tmp_number = ((daemon_bundle_queue_->GetSize() + daemon_antipacket_queue_->GetSize()) - (dtn_seqnof_t)(congestion_control_parameter_ * daemon_queue_bytes_max_));
                    if (tmp_number <= 0) {
                        sprintf(tmp_msg_00, "%d", 0);
                    } else {
                        sprintf(tmp_msg_00, "%d", tmp_number);
                    }
                    msg << tmp_msg_00;
                    ReorderDaemonBundleQueueDetail();
                    char tmp_msg_01[1024] = "";
                    int pkts = daemon_bundle_queue_->GetNPackets();
                    int anti_pkts = daemon_antipacket_queue_->GetNPackets();
                    sprintf(tmp_msg_01, "%d", pkts);
                    for (int i = 0; i < pkts; i++) {
                        Ptr<Packet> p_pkt = daemon_bundle_queue_->Dequeue();
                        if (msg.str().length() < HS_MAX) {
                            BPHeader bp_header;
                            p_pkt->RemoveHeader(bp_header);
                            dtn_seqno_t tmp_seqno = bp_header.get_source_seqno();
                            char tmp_msg_02[1024] = "";
                            sprintf(tmp_msg_02, "%d", tmp_seqno);
                            msg << tmp_msg_02;
                            p_pkt->AddHeader(bp_header);
                        } else {
                            // ERROR LOG TODO
                            // 'at time, too big hello bundle'
                        }
                        daemon_bundle_queue_->Enqueue(p_pkt);
                    }
                    for (int i = 0; i < anti_pkts; i++) {
                        Ptr<Packet> p_pkt = daemon_antipacket_queue_->Dequeue();
                        if (msg.str().length() < HS_MAX) {
                            BPHeader bp_header;
                            p_pkt->RemoveHeader(bp_header);
                            dtn_seqno_t tmp_seqno = bp_header.get_source_seqno;
                            char tmp_msg_03[1024] = "";
                            sprintf(tmp_msg_03, "%d", tmp_msg_03);
                            msg << tmp_msg_03;
                            p_pkt->AddHeader(bp_header);
                        } else {
                            // ERROR LOG TODO
                        }
                        daemon_antipacket_queue_->Enqueue(p_pkt);
                    }
                    CreateHelloBundleAndSendDetail(msg.str());
                    Simulator::Schedule(Seconds(0.1), &DtnApp::ToSendHello, this, socket, simulation_end_time, Seconds(HI_TIME), true);
                }
            } else {
                Simulator::Schedule(hello_interval, &DtnApp::ToSendHello, this, socket, simulation_end_time, hello_interval, true);
            }
        }

        void DtnApp::ReceiveHello(Ptr<Socket> socket_handle) {
            Ptr<Packet> p_pkt;
            Address from_addr;
            while (p_pkt = socket_handle->RecvFrom(from_addr)) {
                InetSocketAddress addr = InetSocketAddress::ConvertFrom(from_addr);
                bool neighbor_found = false;
                int j = 0;
                for (; j < neighbor_info_vec_.size(); j++) {
                    neighbor_found = neighbor_info_vec_[j].info_address_ == addr ? true : false;
                }
                if (!neighbor_found) {
                    //TODO
                }
                neighbor_info_vec_[j].info_last_seen_time_ = Simulator::Now().GetSeconds();
                neighbor_info_vec_[j].info_baq_seqnof_vec_ = vector<dtn_seqnof_t>(HNBAQ_MAX, 0);
                uint8_t* msg = new uint8_t[p_pkt->GetSize() + 1];
                p_pkt->CopyData(msg, p_pkt->GetSize());
                msg[p_pkt->GetSize()] = "\0";
                // dangerous cast, but it's ok here
                const char* p_char_raw_pkt = std::reinterpret_cast<const char*>(msg);
                std::stringstream pkt_str_stream = std::stringstream(std::string(p_char_raw_pkt));
                do {
                    // parse raw content of pkt
                    int pkt_avilable_bytes;
                    pkt_str_stream >> pkt_available_bytes;
                    neighbor_info_vec_[j].info_daemon_baq_available_bytes_ = pkt_available_bytes;
                    int pkt_pktseqno_number;
                    // the rest seqno is the seqno for antipacket
                    pkt_str_stream >> pkt_pktseqno_number;
                    int m = 0;
                    for (; m < pkt_bundleseqno_number; m++) {
                        dtn_seqnof_t tmp_bundle_seqnof;
                        pkt_str_stream >> tmp_bundle_seqnof;
                        neighbor_info_vec_[j].info_baq_seqnof_vec_[m] = tmp_bundle_seqnof;
                    }
                    // check whether 'stringstream' has sth to read
                    while (pkt_str_stream.rebuf()->in_avail() != 0) {
                        dtn_seqnof_t tmp_antipacket_seqnof;
                        pkt_str_stream >> tmp_antipacket_seqnof;
                        // we need negative to indicate this seqno is from antipacket
                        neighbor_info_vec_[j].info_baq_seqnof_vec_[m++] = - tmp_antipacket_seqnof;
                        bool anti_found = false;
                        for (int k = 0; ((k < HNBAQ_MAX) && (anti_found)); k++) {
                            anti_found = neighbor_info_vec_[j].info_sent_ap_seqnof_vec_[k] == - tmp_antipacket_seqnof ? true : false;
                            if (anti_found) {
                                UpdateNeighborInfo(2, j, k);
                                UpdateNeighborInfo(3, j, k);
                            }
                        }
                    }
                } while (0);
                delete msg;
            }
        }
        
        //implement TODO
        void DtnApp::CreateHelloBundleAndSendDetail(string msg_str) {

        }

        // implement TODO
        void DtnApp::ReorderDaemonBundleQueueDetail() {

        }

        void DtnApp::RemoveExpiredBAQDetail() {
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
                        int m = 0, found_expired = 0;
                        while (m < HNBAQ_MAX && found_expired == 0) {
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
            // remove expired antipacket queue packets
            pkt_number = daemon_antipacket_queue_->GetNPackets();
            for (int i = 0; i < pkt_number; ++i) {
                Ptr<Packet> p_pkt = daemon_antipacket_queue_->Dequeue();
                BPHeader bp_header();
                p_pkt->RemoveHeader(bp_header);
                assert(bp_header.get_bundle_type() == Antipacket);
                // if not expired
                if ((Simulator::Now().GetSeconds() - bp_header.get_src_time_stamp().GetSeconds() < 1000.0)) {
                    p_pkt->AddHeader(bp_header);
                    daemon_antipacket_queue_->Enqueue(p_pkt);
                } else {
                    for (int j = 0; j < neighbor_count_; j++) {
                        int k = 0, found_expired = 0;
                        while (k < HNBAQ_MAX && found_expired == 0) {
                            if (bp_header.get_source_seqno == - NeighborInfo[j].info_sent_ap_seqnof_vec_[k]) {
                                found_expired = 1;
                                k++;
                            }
                            if (found_expired == 1) {
                                UpdateNeighborInfo(2, j, k);
                                UpdateNeighborInfo(3, j, k);
                            }
                        }
                    }
                }
            }
        }

        void DtnApp::UpdateNeighborInfo(int which_info, int which_neighbor, int which_pkt_index) {
            switch (which_info) {
                case 0 : {
                // info_baq_seqnof_vec_
                             break;
                         }
                case 1 : {
                         // info_sent_bp_seqnof_vec_
                             break;
                         }
                case 2 : {
                         // info_sent_ap_seqnof_vec_
                             break;
                         }
                case 3 : {
                         // info_sent_ap_time_vec_
                             break;
                         }
                default : break;
            }
        }

        void DtnApp::CheckBuffer(CheckState check_state) {
            // one time one pkt would be sent
            bool real_send_boolean = false;
            pair<int, int> real_send_info;
            Ptr<Packet> p_pkt;
            BPHeader bp_header;
            // remove expired antipackets and bundles
            if (check_state == State_2) {
                RemoveExpiredBAQ();
            }

            // prepare and set real_send_boolean
            if (mac_queue->GetSize() < 2) {
                if (check_state == State_2) {
                    // go through daemon_antipacket_queue_ to find whether real_send_boolean should be set true
                    int pkt_number = daemon_antipacket_queue_->GetNPackets(), n = 0;
                    while ((n++ < pkt_number) && (!real_send_boolean)) {
                        p_pkt = daemon_antipacket_queue_->Dequeue();
                        p_pkt->RemoveHeader(bp_header);
                        Ptr<Packet> p_pkt_copy = p_pkt->Copy();
                        daemon_antipacket_queue_->Enqueue(p_pkt_copy);
                        if ((Simulator::Now().GetSeconds() - bp_header.get_hop_time_stamp.GetSeconds() > 0.2)) {
                            for (int j = 0; (j < neighbor_info_vec_.size()) && (real_send_boolean == false; j++)) {
                                // neighbor already has this antipacket or this antipacket has been sent to somewhere else
                                bool neighbor_has_bundle = false, anti_pkt_sent = false, should_update_time_boolean = false;
                                int m = 0;
                                for (int m = 0; (!neighbor_has_bundle) && (neighbor_info_vec_[j].info_baq_seqnof_vec_[m] != 0) && (m < 2 * HNBAQ_MAX); m++) {
                                    // I think there should be no negative, is I right? // TODO
                                    if (neighbor_info_vec_[j].info_baq_seqnof_vec_[m] == -(dtn_seqno_t)bp_header.get_source_seqno()) {
                                        neighbor_has_bundle = true;
                                    } 
                                }
                                for (int m = 0; (!anti_pkt_sent) && (!neighbor_has_bundle) && (neighbor_info_vec_[j].info_sent_ap_seqnof_vec_[m] != 0) && m < (HNBAQ_MAX); m++) {
                                    if (neighbor_info_vec_[j].info_sent_ap_seqnof_vec_[m] == - (dtn_seqno_t)bp_header.get_source_seqno() && (Simulator::Now().GetSeconds() - neighbor_info_vec_[j].info_sent_ap_time_vec_[m] < 1.5)) {
                                        anti_pkt_sent = true;
                                    } else {
                                        should_update_time_boolean = true;
                                    }
                                }
                                if (neighbor_has_bundle == false && anti_pkt_sent == false) {
                                    real_send_boolean = true;
                                    std::pair p{n, j};
                                    real_send_info(p);
                                    // why do we find the 'm' by this way? TODO
                                    int m = 0;
                                    while ((should_update_time_boolean) && (neighbor_info_vec_[j].info_sent_ap_seqnof_vec_[m] != 0) && (neighbor_info_vec_[j].info_sent_ap_seqnof_vec_[m] != - (dtn_seqno_t)bp_header.get_source_seqno())) {
                                        m++;
                                    }
                                    // 'negative' means that this antipack is sent out of this node
                                    neighbor_info_vec_[j].info_sent_ap_seqnof_vec_[m] = - (dtn_seqno_t)bp_header.get_source_seqno();
                                    neighbor_info_vec_[j].info_sent_ap_time_vec_[m] = Simulator::Now().GetSeconds();
                                }
                            }
                        }
                    }
                } else {
                    // go though daemon_bundle_queue_ to find whether real_send_boolean should be set true
                    int pkt_number = daemon_bundle_queue_->GetNPackets(), n = 0;
                    while ((n++ < pkt_number) && (!real_send_boolean)) {
                        p_pkt = daemon_bundle_queue_->Dequeue();
                        BPHeader bp_header();
                        p_pkt->RemoveHeader(bp_header);
                        if (Simulator::Now().GetSeconds() - bp_header.get_src_time_stamp().GetSeconds() < 750.0) {
                            if (Simulator::Now().GetSeconds() - bp_header.get_src_time_stamp().GetSeconds() > 0.2) {
                                Ipv4Address dst_addr = bp_header.get_destination_ip(); uint32_t spray_value = bp_header.get_spray();
                                for (int j = 0; (j < neighbor_info_vec_.size()) && (!real_send_boolean); j++) {
                                    if (
                                            (((check_state == State_0) && (spray_value > 0) && ((congestion_control_method_ == 0) || (neighbor_info_vec_[j].info_daemon_baq_available_bytes_ > p_pkt->GetSize()))) || (dst_addr != neighbor_info_vec_[j].info_address_.GetIpv4()))
                                            && ((Simulator::Now().GetSeconds() - neighbor_info_vec_[j].info_last_seen_time_) < 0.1) 
                                            && (neighbor_info_vec_[j].info_address_.GetIpv4() != bp_header.get_source_ip()) 
                                       ) {
                                        bool neighbor_has_bundle = false, bundle_sent = false;
                                        for (int m = 0; (!neighbor_has_bundle) && (neighbor_info_vec_[j].info_baq_seqnof_vec_[m] != 0); m++) {
                                            if (neighbor_info_vec_[j].info_baq_seqnof_vec_[m] == bp_header.get_source_seqno()) {
                                                neighbor_has_bundle = true;
                                            }
                                        }
                                        for (int m = 0; (!neighbor_has_bundle) && (!bundle_sent) && (neighbor_info_vec_[j].info_sent_bp_seqnof_vec_[m] != 0); m++) {
                                            if (neighbor_info_vec_[j].info_sent_bp_seqnof_vec_[m] == bp_header.get_source_seqno()) {
                                                bundle_sent = true;
                                            }
                                        }
                                        if ((!neighbor_has_bundle) && (!bundle_sent)) {
                                            real_send_boolean = true;
                                            // use spray, which confuse me a little //TODO
                                            if (check_state == State_0) {
                                                if (routing_method_ == SprayAndWait) {
                                                    bp_header.set_spray(bp_header.get_spray() / 2);
                                                }
                                                if (congestion_control_method_ != NoControl) {
                                                    if (p_pkt->GetSize() >= neighbor_info_vec_[j].info_daemon_baq_available_bytes_) {
                                                        neighbor_info_vec_[j].info_daemon_baq_available_bytes_ = 0;
                                                    } else {
                                                        neighbor_info_vec_[j].info_daemon_baq_available_bytes_ -= p_pkt->GetSize();
                                                    }
                                                }
                                            } else {
                                                bp_header.set_hop_time_stamp(Simulator::Now() + Seconds(5.0));
                                            }
                                            // update neighbor_info_vec_
                                            while (neighbor_info_vec_[j].info_sent_bp_seqnof_vec_[m] != 0 && m < HNBAQ_MAX) {
                                                m++;
                                            }
                                            neighbor_info_vec_[j].info_sent_bp_seqnof_vec_[m] = bp_header->get_source_seqno();
                                        }
                                    }
                                }
                            }
                            p_pkt->AddPacket(bp_header);
                            Ptr<Packet> p_pkt_copy = p_pkt->Copy();
                            daemon_bundle_queue_->Enqueue(p_pkt_copy);
                        } else {
                            // rearrange, do nothing about real_send_boolean 
                            if ((Simulator::Now().GetSeconds() - bp_header.get_src_time_stamp().GetSeconds() > 1000.0) && (bp_header.get_hop_count() == 0) && (bp_header.get_retransmission_count() < 3)) {
                                // this bundle had not been sent out, so we can reset time for it and then retransmit it without influent others
                                bp_header.set_src_time_stamp(Simulator::Now());
                                bp_header.set_hop_count(bp_header.get_hop_count() + 1);
                                for (int j = 0; j < neighbor_info_vec_.size(); j++) {
                                    bool send_found = false;
                                    for (int m = 0; (m < HNBAQ_MAX) && (!send_found); m++) {
                                        // this seqno is not negative means that this bpheader is send out from this node and received at neighbor
                                        if (neighbor_info_vec_[j].info_sent_bp_seqnof_vec_[m] == bp_header.get_source_seqno()) {
                                            sent_found = true;
                                        }
                                        if (send_found) {
                                            UpdateNeighborInfo(1, j, m);
                                        }
                                    }
                                }
                            }
                            if ((Simulator::Now().GetSeconds() - bp_header.get_src_time_stamp().GetSeconds() <= 1000.0) && (bp_header.get_hop_count() == 0)) {
                                p_pkt->AddPacket(bp_header);
                                daemon_bundle_queue_->Enqueue(p_pkt);
                            }
                        }
                    }
                }
            } else {
                check_state = State_0;
            }

            // do real send stuff
            if (real_send_boolean) {
                if (!daemon_socket_handle_) {
                    CreateSocketDetail();
                }
                int j = get<1>(real_send_info);
                InetSocketAddress dst_addr(neighbor_info_vec_[j].info_address_, PORT_NUMBER);
                if (check_state != State_2) {
                    daemon_flow_count_++;
                    struct DaemonBundleHeaderInfo tmp_header_info {
                        dst_addr.GetIpv4(),
                            bp_header.get_source_seqno(),
                            bp_header.get_retransmission_count()
                    }
                    daemon_sent_bh_info_vec_.push_back(tmp_header_info);
                    struct DaemonTransmissionInfo tmp_transmission_info {
                        p_pkt->GetSize(),
                            std::min(TS_MAX, p_pkt->GetSize()),
                            Simulator::Now().GetSeconds(),
                            Simulator::Now().GetSeconds(),
                            std::min(TS_MAX, p_pkt->GetSize())
                    }
                    daemon_transmission_info_vec_.push_back(tmp_transmission_info);
                    // we should let 'SocketSendDetail' to do this //TODO
                    if (p_pkt->GetSize() > TS_MAX) {
                        p_pkt->RemoveAtEnd(p_pkt->GetSize() - 1472);
                    }
                    // what tags means ? // TODO
                    p_pkt->AddPacketTag(FlowIdTag(bp_header.get_source_seqno()));
                    p_pkt->AddPacketTag(QosTag(bp_header.get_retransmission_count()));
                    daemon_retransmission_packet_buffer_vec_.push_back(p_pkt->Copy());
                    Simulator::Schedule(Seconds(1.0), &DtnApp::ToTransmit, this, tmp_header_info);
                } else {
                    // to author's intention every sequence number of antipacket would be negative
                    p_pkt->AddPacketTag(FlowIdTag(- (bp_header.get_source_seqno())));
                    p_pkt->AddPacketTag(QosTag(4));
                }
                SocketSendDetail(p_pkt, 0, dst_addr);
            }

            // switch check_state and reschedule
            switch (check_state) {
                case State_0 : {
                                   if (real_send_boolean == 0) {Simulator::Schedule(Seconds(0.01), &DtnApp::CheckBuffer(), this, 2);}
                                   else {Simulator::Schedule(Seconds(0.001), &DtnApp::CheckBuffer(), this, 2);}
                                   break;
                               }
                case State_1 : {
                                   if (real_send_boolean == 0) {CheckBuffer(0);}
                                   else {Simulator::Schedule(Seconds(0.001), &DtnApp::CheckBuffer(), this, 2);}
                                   break;
                               }
                case State_2 : {
                                   if (real_send_boolean == 0) {CheckBuffer(1);}
                                   else {Simulator::Schedule(Seconds(0.001), &DtnApp::CheckBuffer(), this, 2);}
                                   break;
                               }
                default : {
                              break;
                          }
            }
        }

        // should handle more condition // TODO
        bool DtnApp::SocketSendDetail(Ptr<Packet> p_pkt, uint32_t flags, const Address& dst_addr) {
            int result = daemon_socket_handle_.SendTo(p_pkt, flags, dst_addr);
            return result != -1 ? true : false;
        }

        bool DtnApp::CreateSocketDetail() {
            daemon_socket_handle_ = Socket::CreateSocket(GetNode(), TypeId::LookupByName("ns3::UdpSocketFactory"));
            Ptr<Ipv4> ipv4 = node_->GetObject<Ipv4>();
            Ipv4Address ipaddr = (ipv4->GetAddress(1, 0)).GetLocal();
            InetSocketAddress local = InetSocketAddress(ipaddr, PORT_NUMBER);
            daemon_socket_handle_->Bind(local);
        }
    } /* ns3dtnbit */ 
    /* ... */
}

