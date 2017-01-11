/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3dtn-bit.h"
#include "./common_header.h"

namespace ns3 {

    namespace ns3dtnbit {

        using std::vector;
        using std::string;

        /* refine TODO
        */
        void DtnApp::ToSendHello(Ptr<Socket> socket, dtn_time_t simulation_end_time, Time hello_interval, bool hello_right_now_boolean) {
            if (hello_right_now_boolean) {
                if (Simulator::Now().GetSeconds() > simulation_end_time) {
                    daemon_socket_handle_->Close();
                } else {
                    do {
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
                        int32_t tmp_number = ((daemon_bundle_queue_->GetSize() + daemon_antipacket_queue_->GetSize()) - (dtn_seqno_t)(congestion_control_parameter_ * daemon_baq_bytes_max_));
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
                    } while (0);
                    // this call would use msg.str() as the 'hello content'
                    CreateHelloBundleAndSendDetail(msg.str());
                    Simulator::Schedule(Seconds(0.1), &DtnApp::ToSendHello, this, socket, simulation_end_time, Seconds(HI_TIME), true);
                }
            } else {
                Simulator::Schedule(hello_interval, &DtnApp::ToSendHello, this, socket, simulation_end_time, hello_interval, true);
            }
        }

        /* refine TODO
        */
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
                    NeighborInfo tmp_neighbor_info {
                        addr,
                            0,
                            0,
                            vector<dtn_seqno_t>(),
                            vector<dtn_seqno_t>(),
                            vector<dtn_seqno_t>(),
                            vector<dtn_time_t>()
                    };
                    neighbor_info_vec_.push_back(tmp_neighbor_info);
                }
                neighbor_info_vec_[j].info_last_seen_time_ = Simulator::Now().GetSeconds();
                neighbor_info_vec_[j].info_baq_seqno_vec_ = vector<dtn_seqno_t>(HNBAQ_MAX, 0);
                std::string msg;
                // this call would abstract 'hello content' from p_pkt
                ReceiveHelloBundleDetail(p_pkt, msg);
                // dangerous cast, but it's ok here
                std::stringstream pkt_str_stream(msg);
                do {
                    // parse raw content of pkt and update 'neighbor_info_vec_'
                    pkt_str_stream >> neighbor_info_vec_[j].info_daemon_baq_available_bytes_;
                    int pkt_pktseqno_number;
                    // the rest seqno is the seqno for antipacket
                    pkt_str_stream >> pkt_pktseqno_number;
                    int m = 0;
                    for (; m < pkt_bundleseqno_number; m++) {
                        pkt_str_stream >> neighbor_info_vec_[j].info_baq_seqno_vec_[m];
                    }
                    // check whether 'stringstream' has sth to read
                    while (pkt_str_stream.rebuf()->in_avail() != 0) {
                        dtn_seqno_t tmp_antipacket_seqno;
                        pkt_str_stream >> tmp_antipacket_seqno;
                        // we need negative to indicate this seqno is from antipacket
                        neighbor_info_vec_[j].info_baq_seqno_vec_[m++] = tmp_antipacket_seqno;
                        bool anti_found = false;
                        for (int k = 0; ((k < HNBAQ_MAX) && (anti_found)); k++) {
                            anti_found = neighbor_info_vec_[j].info_sent_ap_seqno_vec_[k] == tmp_antipacket_seqno ? true : false;
                            if (anti_found) {
                                UpdateNeighborInfoDetail(2, j, k);
                                UpdateNeighborInfoDetail(3, j, k);
                            }
                        }
                    }
                } while (0);
            }
        }

        /* refine TODO
         */
        void DtnApp::ToSendAck(BPHeader const& ref_bp_header, Ipv4Address response_ip) {
            std::string tmp_payload_str;
            do {
                // fill up payload
                std::stringstream tmp_sstream;
                tmp_sstream << ref_bp_header.get_source_ip();
                tmp_sstream << " ";
                tmp_sstream << ref_bp_header.get_source_seqno();
                tmp_sstream << " ";
                tmp_sstream << ref_bp_header.get_payload_size();
                tmp_sstream << " ";
                tmp_sstream << ref_bp_header.get_retransmission_count();
                tmp_payload_str = tmp_sstream.str();
            } while (0);
            Ptr<Packet> p_pkt = Create<Packet>(static_cast<uint8_t const*>(tmp_payload_str.c_str()), tmp_payload_str.size());
            BPHeader bp_header;
            do {
                // fill up bp_header
                SemiFillBPHeaderDetail(&bp_header);
                bp_header.set_bundle_type(TransmissionAck);
                bp_header.set_destination_ip(response_ip);
                bp_header.set_source_seqno(p_pkt->GetUid());
                bp_header.set_payload_size(tmp_payload_str.size());
            } while (0);
            p_pkt->AddHeader(bp_header);
            InetSocketAddress response_addr(response_ip, PORT_NUMBER);
            SocketSendDetail(p_pkt, 0, response_addr)
        }
        /* refine TODO
         * 
         */
        void DtnApp::ToSendBundle(uint32_t dstnode_number, uint32_t payload_size) {
            std::string tmp_payload_str;
            do {
                // fill up payload 
                tmp_payload_str = "just_one_content_no_meaning"
            } while (0);
            Ptr<Packet> p_pkt = Create<Packet>(static_cast<uint8_t const*>(tmp_payload_str.c_str()), tmp_payload_str.size());
            BPHeader bp_header;
            do {
                // fill up bp_header
                SemiFillBPHeaderDetail(&bp_header);
                bp_header.set_bundle_type(BundlePacket);
                char dststring[1024] = "";
                sprintf(dststring, "10.0.0.%d", dstnode_number + 1);
                bp_header.set_destination_ip(dststring);
                bp_header.set_source_seqno(p_pkt->GetUid());
                bp_header.set_payload_size(tmp_payload_str.size());
            } while (0);
            p_pkt->AddHeader(bp_header);
            if ((daemon_antipacket_queue_->GetNBytes() + daemon_bundle_queue_->GetNBytes + p_pkt->GetNBytes() <= daemon_baq_bytes_max_)) {
                daemon_bundle_queue_->Enqueue(p_pkt);
                // NORMAL LOG TODO
            } else {
                // ERROR LOG TODO
            }
        }

        /* refine TODO
         * create and fill up antipacket-bundle then enqueue
         */
        void DtnApp::ToSendAntipacketBundle(BPheader const& ref_bp_header) {
            string anti_packet_payload_str;
            do {
                // fill up payload buffer
                std::stringstream msg;
                Ipv4Address srcip = ref_bp_header.get_source_ip();
                srcip.Print(msg);
                msg << " ";
                Ipv4Address dstip = ref_bp_header.get_destination_ip();
                dstip.Print(msg);
                msg << " ";
                msg << ref_bp_header.get_source_seqno() << " ";
                msg << END_OF_PAYLOAD;
            } while (0);
            Ptr<Packet> p_pkt = Create<Packet>(anti_packet_payload_str.c_str(), anti_packet_payload_str.size());
            BPHeader bp_header;
            do {
                // fill up bp_header
                SemiFillBPHeaderDetail(&bp_header);
                bp_header.set_bundle_type(AntiPacket);
                bp_header.set_destination_ip(ref_bp_header.get_source_ip());
                bp_header.set_source_seqno(p_pkt->GetUid());
                bp_header.set_payload_size(anti_packet_payload_str.size());
            } while (0);
            p_pkt->AddHeader(bp_header);
            daemon_antipacket_queue_->Enqueue(p_pkt);
            do {
                // LOG TODO
            } while (0);
        }

        /* implement TODO
         *     'ack bundle' : call 'ToTransmit' to transmit more(if have more) to the 'ack sender'
         *     'not ack bundle' : send a 'ack bundle', then get the information in that pkt
         *         'hello packet'
         *         'normal bundle'
         *         'antipacket bundle'
         * use daemon_reception_info_vec_ and daemon_reception_packet_buffer_vec_
         */
        void DtnApp::ReceiveBundle(Ptr<socket> socket) {
            Address own_addr;
            socket->GetSockName(own_addr);
            InetSocketAddress own_s_addr = InetSocketAddress::ConvertFrom(own_addr);
            while (socket->GetRxAvailable() > 0) {
                Address from_addr;
                Ptr<Packet> p_pkt = socket->RecvFrom(from_addr);
                BPHeader bp_header;
                p_pkt->RemoveHeader(bp_header);
                InetSocketAddress from_s_addr = InetSocketAddress::ConvertFrom(from_addr);
                Ipv4Address from_ip = Ipv4Address::ConvertFrom(from_addr);
                if (bp_header.get_bundle_type() == TransmissionAck) {
                    // if is 'ack' update state, get ack src-dst seqno, timestamp, bytes. then call 'ToTransmit' to transmit more
                    Ipv4Address ip_was_acked;
                    dtn_seqno_t seqno_was_acked;
                    uint32_t bytes_was_acked; 
                    uint32_t retransmit_count;
                    std::stringstream ss;
                    p_pkt->CopyData(ss, bp_header->get_payload_size());
                    ss >> ip_was_acked >> seqno_was_acked >> bytes_was_acked >> retransmit_count;
                    DaemonBundleHeaderInfo tmp_bh_info {
                        ip_was_acked,
                            retransmit_count,
                            seqno_was_acked
                    };
                    for (int k = 0; k < daemon_transmission_info_vec_.size(); k++) {
                        if (tmp_bh_info == daemon_transmission_bh_info_vec_[k]) { break; }
                    }
                    daemon_transmission_info_vec_[k].info_transmission_current_sent_acked_bytes_ += 
                        daemon_transmission_info_vec_[k].info_transmission_bundle_last_sent_bytes_;
                    ToTransmit(tmp_header_info);
                } else {
                    // if not, send 'ack'
                    ToSendAck(bp_header, from_ip);
                }
                do {
                    // process the receiving pkt, then shrift state
                    DaemonReceptionInfo tmp_recept_info {
                        p_pkt->GetSize(),
                        bp_header.get_payload_size(),
                        Simulator::Now().GetSeconds(),
                        bp_header.get_source_ip(),
                        bp_header.get_destination_ip(),
                        bp_header.get_source_seqno(),
                        from_ip,
                        bp_header.get_bundle_type(),
                        vector<Ptr<Packet>>()
                    };
                    if (tmp_recept_info.info_bundle_type_ == AntiPacket) {
                        // antipacket must not be fragment, it's safe to directly process TODO
                        // note that the from_ip can be a delay node in the antipacket cast procession
                    } else if (tmp_recept_info.info_bundle_type_ == BundlePacket) {
                        bool reception_info_found = false;
                        int k = 0;
                        while (; k < daemon_reception_info_vec_.size(); k++) {
                            if (tmp_recept_info.info_bundle_source_ip_ == daemon_reception_info_vec_[k].info_bundle_source_ip_ 
                                    && tmp_recept_info.info_bundle_seqno_ == daemon_reception_info_vec_[k].info_bundle_seqno_
                                    && tmp_recept_info.info_trasmission_receive_from_ip_ == daemon_reception_info_vec_[k].info_trasmission_receive_from_ip_) {
                                reception_info_found = true;
                            }
                        }
                        if (reception_info_found) {
                            // is recorded, keep receiving and check order of fragment, add new fragment to 'daemon_reception_packet_buffer_vec_' when all fragment received, parse this packet, deal with it
                            p_pkt->AddHeader(bp_header);
                            tmp_recept_info.info_fragment_pkt_pointer_vec_.push_back(p_pkt);
                            FragmentReassembleDetail(k);
                        } else {
                            // not recored
                            daemon_reception_info_vec_.push_back(tmp_recept_info);
                            p_pkt->AddHeader(bp_header);
                            daemon_reception_packet_buffer_vec_.push_back(p_pkt);
                        }
                    } else {}
                } while (0);
            }
        }

        /* implement TODO
         */
        void FragmentReassembleDetail(int k) {

        }

        /*implement TODO
         * use bh_info as key get all msg, 
         * check whether it's able to call 'SocketSendDetail'
         * check retransmit or transmit more
         * use daemon_retransmission_packet_buffer_vec_
         * note the packet size problem, size < TS_MAX
         * update 'daemon_transmission_info_vec_'
         * you should fragment big pkt, the fragment rule is : first fragment.set_payload_size(total_size), second fragment.set_payload_size(actual_size)
         */
        void DtnApp::ToTransmit(DaemonBundleHeaderInfo bh_info) {
            bool index_found = false;
            bool is_retransmist = false;
            bool is_transmit_more = false;
            int index = 0;
            while () {
                // find the index of the 'bundle to transmit' to 'daemon_transmission_info_vec_' using 'bh_info'
            }
            if (!index_found) {

            } else {
                int j = 0;
                bool neighbor_found = false;
                bool real_send_boolean = false;
                while () {
                    // find the neighbor should be transmit, if this neighbor was not recently seen, schedule 'ToTransmit' later, otherwise, set real_send_boolean
                }
                while () {
                    // set is_retransmist or is_transmit_more boolean
                }
                if (real_send_boolean) {
                    if (is_retransmist) {
                        // call 'RetransmitDetail'
                        // note schedule
                    } else if (is_transmit_more) {
                        // call 'TransmitMoreDetail'
                    } else {
                        // also call 'TransmitMoreDetail'
                    }
                    // update state
                }
            }
        }

        /* implement TODO
        */
        void DtnApp::RetransmitDetail(int index) {

        }

        /* implement TODO
        */
        void DtnApp::TransmitMoreDetail(int index) {
            if (daemon_transmission_info_vec_[index].info_transmission_current_sent_bytes_ < daemon_transmission_info_vec_[index].info_transmission_total_send_bytes_) {
                // call 'SocketSendDetail' schedule to 'ToTransmit'
            } else {
                daemon_transmission_info_vec_[index].info_transmission_bundle_last_sent_bytes_ = 0;
            }
        }

        /* refine TODO
         * check your 'bundle queue' buffer and other related buffer periodly
         * make code refactory to this 
         * do check_state == state_2 means that it was an Antipacket? and != 2 means that it was bundlepacket? TODO
         */
        void DtnApp::CheckBuffer(CheckState check_state) {
            // one time one pkt would be sent
            bool real_send_boolean = false;
            int decision_neighbor = 0;
            pair<int, int> real_send_info;
            Ptr<Packet> p_pkt;
            BPHeader bp_header;
            // remove expired antipackets and bundles
            if (check_state == State_2) {
                RemoveExpiredBAQ();
            }
            // prepare and set real_send_boolean
            // being less than 2 means, wifi mac layer is not busy
            if (daemon_mac_queue_->GetSize() < 2) {
                if (check_state == State_2) {
                    // go through daemon_antipacket_queue_ to find whether real_send_boolean should be set true
                    int pkt_number = daemon_antipacket_queue_->GetNPackets(), n = 0;
                    while ((n++ < pkt_number) && (!real_send_boolean)) {
                        p_pkt = daemon_antipacket_queue_->Dequeue();
                        p_pkt->RemoveHeader(bp_header);
                        if ((Simulator::Now().GetSeconds() - bp_header.get_hop_time_stamp.GetSeconds() > 0.2)) {
                            real_send_boolean = BPHeaderBasedSendDecisionDetail(bp_header, decision_neighbor, check_state);
                        }
                        p_pkt->AddHeader(bp_header);
                        Ptr<Packet> p_pkt_copy = p_pkt->Copy();
                        daemon_antipacket_queue_->Enqueu(p_pkt_copy);
                    }
                } else {
                    // go though daemon_bundle_queue_ to find whether real_send_boolean should be set true
                    int pkt_number = daemon_bundle_queue_->GetNPackets(), n = 0;
                    while ((n++ < pkt_number) && (!real_send_boolean)) {
                        p_pkt = daemon_bundle_queue_->Dequeue();
                        p_pkt->RemoveHeader(bp_header);
                        if (Simulator::Now().GetSeconds() - bp_header.get_src_time_stamp().GetSeconds() < HB_TIME) {
                            if (Simulator::Now().GetSeconds() - bp_header.get_src_time_stamp().GetSeconds() > 0.2) {
                                real_send_boolean = BPHeaderBasedSendDecisionDetail(bp_header, decision_neighbor, check_state);
                            }
                            p_pkt->AddHeader(bp_header);
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
                                        if (neighbor_info_vec_[j].info_sent_bp_seqno_vec_[m] == bp_header.get_source_seqno()) {
                                            sent_found = true;
                                        }
                                        if (send_found) {
                                            UpdateNeighborInfoDetail(1, j, m);
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
                int j = decision_neighbor;
                InetSocketAddress dst_addr(neighbor_info_vec_[j].info_address_, PORT_NUMBER);
                DaemonBundleHeaderInfo tmp_header_info {
                    dst_addr.GetIpv4(),
                        bp_header.get_source_seqno(),
                        bp_header.get_retransmission_count()
                }
                daemon_transmission_bh_info_vec_.push_back(tmp_header_info);
                DaemonTransmissionInfo tmp_transmission_info {
                    p_pkt->GetSize(),
                        std::min(TS_MAX, p_pkt->GetSize()),
                        Simulator::Now().GetSeconds(),
                        Simulator::Now().GetSeconds(),
                        std::min(TS_MAX, p_pkt->GetSize())
                }
                daemon_transmission_info_vec_.push_back(tmp_transmission_info);
                if (check_state != State_2) {
                    daemon_flow_count_++;
                    // we should let 'ToTransmit' to do this //TODO
                    //if (p_pkt->GetSize() > TS_MAX) {
                    //    p_pkt->RemoveAtEnd(p_pkt->GetSize() - TS_MAX);
                    //}
                    p_pkt->AddPacketTag(FlowIdTag(bp_header.get_source_seqno()));
                    p_pkt->AddPacketTag(QosTag(bp_header.get_retransmission_count()));
                    // don't need to schedule, because totransmit will autometally schedule it
                    // Simulator::Schedule(Seconds(1.0), &DtnApp::ToTransmit, this, tmp_header_info);
                } else {
                    // to author's intention every sequence number of antipacket would be negative
                    p_pkt->AddPacketTag(FlowIdTag(- (bp_header.get_source_seqno())));
                    p_pkt->AddPacketTag(QosTag(4));
                }
                // call 'ToTransmit' instead of this, or not ? TODO
                daemon_retransmission_packet_buffer_vec_.push_back(p_pkt->Copy());
                ToTransmit(tmp_header_info);
                // SocketSendDetail(p_pkt, 0, dst_addr);
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

        /* refine
         * handle normal bundle and antipacket send decision
         */
        bool DtnApp::BPHeaderBasedSendDecisionDetail(BPHeader const& ref_bp_header, int& return_index_of_neighbor, CheckState const& check_state,) {
            bool real_send_boolean = false;
            switch (ref_bp_header.get_bundle_type()) {
                case BundleType : { 
                                      Ipv4Address dst_ip = bp_header.get_destination_ip();
                                      uint32_t spray_value = bp_header.get_spray();
                                      for (int j = 0; (j < neighbor_info_vec_.size()) && (!real_send_boolean); j++) {
                                          if (
                                                  (((check_state == State_0) && (spray_value > 0) && ((congestion_control_method_ == 0) || (neighbor_info_vec_[j].info_daemon_baq_available_bytes_ > ref_bp_header->get_payload_size() + ref_bp_header->GetSerializedSize()))) || (dst_ip != neighbor_info_vec_[j].info_address_.GetIpv4()))
                                                  && ((Simulator::Now().GetSeconds() - neighbor_info_vec_[j].info_last_seen_time_) < 0.1) 
                                                  && (neighbor_info_vec_[j].info_address_.GetIpv4() != bp_header.get_source_ip()) 
                                             ) {
                                              // 'neighbor_has_bundle is false' maens that neighbor do not carry this bundle
                                              // 'bundle_sent is false' means that you haven't send this bundle to this neighbor
                                              bool neighbor_has_bundle = false, bundle_sent = false;
                                              for (int m = 0; (!neighbor_has_bundle) && (neighbor_info_vec_[j].info_baq_seqno_vec_.size() > m); m++) {
                                                  if (neighbor_info_vec_[j].info_baq_seqno_vec_[m] == bp_header.get_source_seqno()) {
                                                      neighbor_has_bundle = true;
                                                  }
                                              }
                                              for (int m = 0; (!neighbor_has_bundle) && (!bundle_sent) && (neighbor_info_vec_[j].info_sent_bp_seqno_vec_.size() > m); m++) {
                                                  if (neighbor_info_vec_[j].info_sent_bp_seqno_vec_[m] == bp_header.get_source_seqno()) {
                                                      bundle_sent = true;
                                                  }
                                              }
                                              if ((!neighbor_has_bundle) && (!bundle_sent)) {
                                                  real_send_boolean = true;
                                                  return_index_of_neighbor = j;
                                                  if (check_state == State_0) {
                                                      if (routing_method_ == SprayAndWait) {
                                                          ref_bp_header.set_spray(bp_header.get_spray() / 2);
                                                      }
                                                      if (congestion_control_method_ != NoControl) {
                                                          if (ref_bp_header->get_payload_size() + ref_bp_header->GetSerializedSize()
                                                                  >= neighbor_info_vec_[j].info_daemon_baq_available_bytes_) {
                                                              neighbor_info_vec_[j].info_daemon_baq_available_bytes_ = 0;
                                                          } else {
                                                              neighbor_info_vec_[j].info_daemon_baq_available_bytes_ -=
                                                                  ref_bp_header->get_payload_size() + ref_bp_header->GetSerializedSize();
                                                          }
                                                      }
                                                  } else {
                                                      bp_header.set_hop_time_stamp(Simulator::Now() + Seconds(5.0));
                                                  }
                                              }
                                          }
                                      }
                                      break;
                                  }
                case AntiPacket : {
                                      for (int j = 0; (j < neighbor_info_vec_.size()) && (real_send_boolean == false; j++)) {
                                          // neighbor already has this antipacket or this antipacket has been sent to this neighbor else
                                          bool neighbor_has_bundle = false, anti_pkt_sent = false; 
                                          for (int m = 0; (!neighbor_has_bundle) && (neighbor_info_vec_[j].info_baq_seqno_vec_.size() > m) && (m < 2 * HNBAQ_MAX); m++) {
                                              if (neighbor_info_vec_[j].info_baq_seqno_vec_[m] == -(dtn_seqno_t)bp_header.get_source_seqno()) {
                                                  neighbor_has_bundle = true;
                                              } 
                                          }
                                          for (int m = 0; (!anti_pkt_sent) && (!neighbor_has_bundle) && (neighbor_info_vec_[j].info_sent_ap_seqno_vec_.size() > m) && m < (HNBAQ_MAX); m++) {
                                              if (neighbor_info_vec_[j].info_sent_ap_seqno_vec_[m] == bp_header.get_source_seqno() && (Simulator::Now().GetSeconds() - neighbor_info_vec_[j].info_sent_ap_time_vec_[m] < 1.5)) {
                                                  anti_pkt_sent = true;
                                              }
                                          }
                                          if (neighbor_has_bundle == false && anti_pkt_sent == false) {
                                              real_send_boolean = true;
                                              return_index_of_neighbor = j;
                                              //   update this in 'ToTransmit'TODO
                                              //   // why do we find the 'm' by this way? TODO
                                              //   int m = 0;
                                              //   while ((should_update_time_boolean) && (neighbor_info_vec_[j].info_sent_ap_seqno_vec_.size() > m) && (neighbor_info_vec_[j].info_sent_ap_seqno_vec_[m] != bp_header.get_source_seqno())) {
                                              //       m++;
                                              //   }
                                              //   // 'negative' means that this antipack is sent out of this node
                                              //   neighbor_info_vec_[j].info_sent_ap_seqno_vec_[m] = bp_header.get_source_seqno();
                                              //   neighbor_info_vec_[j].info_sent_ap_time_vec_[m] = Simulator::Now().GetSeconds();
                                          }
                                      }
                                      break;
                                  }
                default :   break;
            }
            return real_send_boolean;
        }

        /* implement TODO
        */
        void DtnApp::UpdateNeighborInfoDetail(int which_info, int which_neighbor, int which_pkt_index) {
            switch (which_info) {
                case 0 : {
                             // info_baq_seqno_vec_
                             break;
                         }
                case 1 : {
                             // info_sent_bp_seqno_vec_
                             break;
                         }
                case 2 : {
                             // info_sent_ap_seqno_vec_
                             break;
                         }
                case 3 : {
                             // info_sent_ap_time_vec_
                             break;
                         }
                default : break;
            }
        }

        /* this func would be invoked only in ReceiveBundle()
         * it would find the pkt in daemon_bundle_queue_, then dequeue it 
         * and update neighbor_sent_bp_seqno_vec_ & daemon_transmission_bh_info_vec_
         */
        void DtnApp::RemoveBundleDetail(Ptr<Packet> pkt) {}

        /*implement TODO
        */
        void DtnApp::ReceiveHelloBundleDetail(Ptr<Packet> p_pkt, std::string& msg_str) {

        }

        /* implement TODO 
         * check whether one packet is already in your 'antipacket_queue' 
         * by check the 'uni seqno number of the packet'
         */
        bool DtnApp::IsAntipacketExistDetail(Ptr<packet> pkt) {

        }

        /* implement TODO
         * check whether one packet is already in bundle queue
         */
        bool DtnApp::IsDuplicatedDetail(BPHeader bp_header) {}

        /*implement TODO
        */
        void DtnApp::ReceiveBundleDetail() {

        }

        /* refine TODO
         * should handle more condition
         * LOG some
         */
        bool DtnApp::SocketSendDetail(Ptr<Packet> p_pkt, uint32_t flags, const Address& dst_addr) {
            // LOG
            int result = daemon_socket_handle_.SendTo(p_pkt, flags, dst_addr);
            return result != -1 ? true : false;
        }

        /* refine
        */
        bool DtnApp::CreateSocketDetail() {
            daemon_socket_handle_ = Socket::CreateSocket(GetNode(), TypeId::LookupByName("ns3::UdpSocketFactory"));
            Ptr<Ipv4> ipv4 = node_->GetObject<Ipv4>();
            Ipv4Address ipip = (ipv4->GetAddress(1, 0)).GetLocal();
            InetSocketAddress local = InetSocketAddress(ipip, PORT_NUMBER);
            daemon_socket_handle_->Bind(local);
        }

        /* implement TODO
         * using daemon_hello_send_buffer_queue_
         */
        void DtnApp::ReorderDaemonBundleQueueDetail() {

        }

        /* refine
         * create and fill up then send
         */
        void DtnApp::CreateHelloBundleAndSendDetail(string const& msg_str, Ptr<Socket> broad_cast_skt) {
            Ptr<Packet> p_pkt = Create<Packet>(msg_str.c_str(), msg_str.size());
            BPHeader bp_header;
            do {
                // fill up bp_header
                SemiFillBPHeaderDetail(&bp_header);
                bp_header.set_bundle_type(HelloPacket);
                bp_header.set_source_ip(Ipv4Address("255.255.255.255"));
                bp_header.set_source_seqno(p_pkt->GetUid());
                bp_header.set_payload_size(msg_str.size());
            } while (0);
            p_pkt->AddHeader(bp_header);
            p_pkt->AddPacketTag(QosTag(6));
            broad_cast_skt->Send(p_pkt);
        }

        /* refine 
        */
        void DtnApp::RemoveExpiredBAQDetail() {
            uint32_t pkt_number = 0, n = 0;
            // remove expired bundle queue packets
            pkt_number = daemon_bundle_queue_->GetNPackets();
            for (int i = 0; i < pkt_number; ++i) {
                Ptr<Packet> p_pkt = daemon_bundle_queue_->Dequeue();
                BPHeader bp_header();
                p_pkt->RemoveHeader(bp_header);
                //if not expired
                if (((Simulator::Now().GetSeconds() - bp_header.get_src_time_stamp()) < HB_TIME) || (bp_header.get_hop_count() == 0)) {
                    p_pkt->AddHeader(bp_header);
                    daemon_bundle_queue_->Enqueue(p_pkt);
                } else {
                    for (int j = 0; j < neighbor_count_; j++) {
                        int m = 0, found_expired = 0;
                        while (m < HNBAQ_MAX && found_expired == 0) {
                            // hop count equals zero means that it came from a neighbor
                            if (bp_header.get_source_seqno() == neighbor_info_vec_[j].info_sent_bp_seqno_vec_[m]) {
                                found_expired = 1;
                            } else {
                                m++;
                            }
                            if (found_expired == 1) {
                                UpdateNeighborInfoDetail(1, n, j);
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
                            if (bp_header.get_source_seqno == - NeighborInfo[j].info_sent_ap_seqno_vec_[k]) {
                                found_expired = 1;
                                k++;
                            }
                            if (found_expired == 1) {
                                UpdateNeighborInfoDetail(2, j, k);
                                UpdateNeighborInfoDetail(3, j, k);
                            }
                        }
                    }
                }
            }
        }

        /* refine
         * you should fill following fields your self
         * 'bundle type'
         * 'dstip'
         * 'seqno'
         * 'payload size'
         */
        void DtnApp::SemiFillBPHeaderDetail(BPHeader* p_bp_header) {
            char srcstring[1024] = "";
            sprintf(srcstring, "10.0.0.%d", (node_->GetId() + 1));
            p_bp_header->set_source_ip(srcstring);
            p_bp_header->set_hop_count(0);
            p_bp_header->set_spray(16);
            p_bp_header->set_retransmission_count(0);
            p_bp_header->set_src_time_stamp(Simulator::Now());
            p_bp_header->set_hop_time_stamp(Simulator::Now());
        }
    } /* ns3dtnbit */ 
    /* ... */
}

