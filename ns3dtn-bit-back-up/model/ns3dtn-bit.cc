/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3dtn-bit.h"

#include "../config.txt"
extern std::string root_path;

namespace ns3 {
    namespace ns3dtnbit {
        NS_LOG_COMPONENT_DEFINE ("DtnRunningLog");
        /*
         * Can't use CreateObject<>, so do it myself
         */
        Ptr<QueueItem> Packet2Queueit(Ptr<Packet> p_pkt) {
            QueueItem* p = new QueueItem(p_pkt);
            return Ptr<QueueItem>(p);
        }

        Ipv4Address NodeNo2Ipv4(int node_no) {
            auto ip_base = Ipv4Address("10.0.0.1");
            auto ip_v = ip_base.Get();
            ip_v += node_no;
            Ipv4Address result;
            result.Set(ip_v);
            std::cout << result << std::endl;
            return result;
        }

        int Ipv42NodeNo(Ipv4Address ip) {
            auto ip_base = Ipv4Address("10.0.0.1");
            auto ip_base_v = ip_base.Get();
            auto ip_v = ip.Get();
            return ip_v - ip_base_v;
        }

        int nodeid2neighborvecindex(vector<DtnApp::NeighborInfo>& neighbor_info_vec_, int id) {
            int indx;
            auto ip_base = NodeNo2Ipv4(id);
            for (int i = 0; i < neighbor_info_vec_.size(); i++) {
                // result to index of neighbor vec
                auto nip = neighbor_info_vec_[i].info_address_.GetIpv4();
                if (nip.IsEqual(ip_base)) { indx = i; break; }
            }
            return indx;
        }

        tuple<int, bool> ip2neighborvecindex(vector<DtnApp::NeighborInfo>& neighbor_info_vec_, Ipv4Address ip_from) {
            int j = 0;
            bool neighbor_found = false;
            for (; j < neighbor_info_vec_.size(); j++) {
                auto ip_n = neighbor_info_vec_[j].info_address_.GetIpv4();
                neighbor_found = (ip_n.IsEqual(ip_from)) ? true : false;
                if (neighbor_found) {break;}
            }
            return tuple<int, bool>(j, neighbor_found);
        }

        std::string DtnApp::LogPrefix() {
            std::stringstream ss;
            ss << "[time-" << Simulator::Now().GetSeconds() 
                << ";node-" << node_->GetId() << ";";
            return ss.str();
        }

#define LogPrefixMacro LogPrefix()<<"line-"<<__LINE__<<"]"

        /*
         * Aim : inherited method
         * */
        void DtnApp::StartApplication() {
            NS_LOG_DEBUG(LogPrefixMacro << "NOTE:Enter startapplication()");
            try {
                if (1 != node_->GetNDevices()) {
                    std::stringstream ss;
                    ss << "GetNDevices" << node_->GetNDevices();
                    throw std::runtime_error(ss.str());
                }
            } catch (const std::exception& r_e) {
                NS_LOG_DEBUG(LogPrefixMacro << "NOTE:" << r_e.what()); 
            }
            Ptr<WifiNetDevice> wifi_d = DynamicCast<WifiNetDevice> (node_->GetDevice(0));
            wifi_ph_p = wifi_d->GetPhy();
            CheckBuffer(CheckState::State_2);
            StateCheckDetail();
            NS_LOG_LOGIC(LogPrefixMacro << "Out of startapplication()");
        }

        /* refine
         * Aim : inherited method
         */
        void DtnApp::SetUp(Ptr<Node> node) {
            node_ = node;
            congestion_control_parameter_ = 1;
            retransmission_interval_ = NS3DTNBIT_RETRANSMISSION_INTERVAL;
            congestion_control_method_ = CongestionControlMethod::NoControl; 

            daemon_antipacket_queue_ = CreateObject<DropTailQueue>();
            daemon_bundle_queue_ = CreateObject<DropTailQueue>();
            daemon_consume_bundle_queue_ = CreateObject<DropTailQueue>();
            daemon_reorder_buffer_queue_ = CreateObject<DropTailQueue>();

            Ptr<UniformRandomVariable> y = CreateObject<UniformRandomVariable>();

            daemon_antipacket_queue_->SetAttribute("MaxPackets", UintegerValue(1000));
            daemon_bundle_queue_->SetAttribute("MaxPackets", UintegerValue(1000));
            daemon_consume_bundle_queue_->SetAttribute("MaxPackets", UintegerValue(1000));
            daemon_reorder_buffer_queue_->SetAttribute("MaxPackets", UintegerValue(1000));

            daemon_baq_pkts_max_ = 13753;
        }

        /* refine 
         * Aim : send hello
         */
        void DtnApp::ToSendHello(Ptr<Socket> socket, dtn_time_t simulation_end_time, Time hello_interval, bool hello_right_now_boolean) {
            NS_LOG_LOGIC(LogPrefixMacro << "Enter ToSendHello()");
            Ptr<Packet> p_pkt; 
            BPHeader bp_header;
            if (hello_right_now_boolean) {
                if (Simulator::Now().GetSeconds() > simulation_end_time) {
                    daemon_socket_handle_->Close();
                } else {
                    std::stringstream msg;
                    {
                        // prepare 'msg' to write into hello bundle
                        // tmp_msg_00 ---------- Available bytes
                        // tmp_msg_01 ---------- number of pkt seqno
                        // tmp_msg_02 ---------- list of pkt seqno
                        // tmp_msg_03 ---------- list of anti seqno

                        // tmp_msg_00
                        char tmp_msg_00[1024] = "";
                        if (congestion_control_method_ == CongestionControlMethod::DynamicControl) {
                            // not implement yet
                            NS_LOG_ERROR(LogPrefixMacro << "ERROR:not implement yet");
                            std::abort();
                            if ((drops_count_ == 0) && (congestion_control_parameter_ < 0.9)) {
                                congestion_control_parameter_ += 0.01;
                            } else {
                                if ((drops_count_ > 0) && (congestion_control_parameter_ > 0.5)) {
                                    congestion_control_parameter_ *= 0.8;
                                } 
                                drops_count_ = 0;
                            }
                        }
                        int32_t tmp_number = ((dtn_seqno_t)(congestion_control_parameter_ * daemon_baq_pkts_max_ * NS3DTNBIT_HYPOTHETIC_CACHE_FACTOR) - (daemon_bundle_queue_->GetNBytes() + daemon_antipacket_queue_->GetNBytes()));
                        if (tmp_number <= 0) {
                            sprintf(tmp_msg_00, "%d ", 0);
                        } else {
                            sprintf(tmp_msg_00, "%d ", tmp_number);
                        }
                        msg << tmp_msg_00;
                        // TODO we shouldn't invoke this call here
                        //PeriodReorderDaemonBundleQueueDetail();
                        // tmp_msg_01
                        char tmp_msg_01[1024] = "";
                        int pkts = daemon_bundle_queue_->GetNPackets();
                        int anti_pkts = daemon_antipacket_queue_->GetNPackets();
                        sprintf(tmp_msg_01, "%d ", pkts);
                        msg << tmp_msg_01;
                        // tmp_msg_02
                        char tmp_msg_02[1024] = "";
                        for (int i = 0; i < pkts; i++) {
                            p_pkt = daemon_bundle_queue_->Dequeue()->GetPacket();
                            if (msg.str().length() < NS3DTNBIT_HELLO_BUNDLE_SIZE_MAX) {
                                BPHeader bp_header;
                                p_pkt->RemoveHeader(bp_header);
                                dtn_seqno_t tmp_seqno = bp_header.get_source_seqno();
                                sprintf(tmp_msg_02, "%d ", tmp_seqno);
                                p_pkt->AddHeader(bp_header);
                            } else {
                                // ERROR LOG
                                NS_LOG_ERROR("Error : too big hello");
                                std::abort();
                            }
                            daemon_bundle_queue_->Enqueue(Packet2Queueit(p_pkt));
                        }
                        msg << tmp_msg_02;
                        // tmp_msg_03
                        char tmp_msg_03[1024] = "";
                        for (int i = 0; i < anti_pkts; i++) {
                            p_pkt = daemon_antipacket_queue_->Dequeue()->GetPacket();
                            if (msg.str().length() < NS3DTNBIT_HELLO_BUNDLE_SIZE_MAX) {
                                p_pkt->RemoveHeader(bp_header);
                                dtn_seqno_t tmp_seqno = bp_header.get_source_seqno();
                                sprintf(tmp_msg_03, "%d ", tmp_seqno);
                                p_pkt->AddHeader(bp_header);
                            } else {
                                // ERROR LOG
                                NS_LOG_ERROR("too big hello");
                                std::abort();
                            }
                            daemon_antipacket_queue_->Enqueue(Packet2Queueit(p_pkt));
                        }
                        msg << tmp_msg_03;
                    }
                    // this call would use msg.str() as the 'hello content'
                    CreateHelloBundleAndSendDetail(msg.str(), socket);
                    Simulator::Schedule(Seconds(NS3DTNBIT_HELLO_BUNDLE_INTERVAL_TIME), &DtnApp::ToSendHello, this, socket, simulation_end_time, Seconds(NS3DTNBIT_HELLO_BUNDLE_INTERVAL_TIME), true);
                }
            } else {
                Simulator::Schedule(hello_interval, &DtnApp::ToSendHello, this, socket, simulation_end_time, hello_interval, true);
            }
            NS_LOG_LOGIC(LogPrefixMacro << "Out of " << "ToSendHello()");
        }

        /* refine 
         * Aim : ReceiveHello
         */
        void DtnApp::ReceiveHello(Ptr<Socket> socket_handle) {
            NS_LOG_LOGIC(LogPrefixMacro << "enter of receive hello");
            Ptr<Packet> p_pkt;
            string avli_s;
            BPHeader bp_header;
            Address from_addr;
            while ((p_pkt = socket_handle->RecvFrom(from_addr))) {
                InetSocketAddress addr = InetSocketAddress::ConvertFrom(from_addr);
                auto ip_from = addr.GetIpv4();
                addr.SetPort(NS3DTNBIT_PORT_NUMBER);
                bool neighbor_found = false;
                int j = 0;
                /*
                   for (; j < neighbor_info_vec_.size(); j++) {
                   auto ip_n = neighbor_info_vec_[j].info_address_.GetIpv4();
                   neighbor_found = (ip_n.IsEqual(ip_from)) ? true : false;
                   if (neighbor_found) {break;}
                   }
                   */
                tie(j, neighbor_found) = ip2neighborvecindex(neighbor_info_vec_, ip_from);
                if (!neighbor_found) {
                    NeighborInfo tmp_neighbor_info = {
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
                std::stringstream pkt_str_stream;
                p_pkt->RemoveHeader(bp_header);
                int hello_packet_size = p_pkt->GetSize();
                assert(hello_packet_size == bp_header.get_payload_size());
                p_pkt->CopyData(&pkt_str_stream, bp_header.get_payload_size());
                // parse raw content of pkt and update 'neighbor_info_vec_'
                pkt_str_stream >> avli_s;
                neighbor_info_vec_[j].info_daemon_baq_available_bytes_ = stoi(avli_s);
                int pkt_seqno_number;
                // the rest seqno is the seqno for antipacket
                pkt_str_stream >> pkt_seqno_number;
                NS_LOG_INFO(LogPrefixMacro << "receive hello, node-" << Ipv42NodeNo(ip_from) << " to node-" << node_->GetId() << ", ip=" << ip_from << ";found =" << neighbor_found << "; found node id=" <<  Ipv42NodeNo(neighbor_info_vec_[j].info_address_.GetIpv4()) << ";pkt_seqno_number=" << pkt_seqno_number << ";Available_bytes_=" << neighbor_info_vec_[j].info_daemon_baq_available_bytes_ << ";avli_s=" << avli_s << ";size()=" << bp_header.get_payload_size());
                {
                    // going to fill baq
                    auto tmpbaq_vec = vector<dtn_seqno_t>();
                    // fill up baq with 'b' pkt
                    if (pkt_seqno_number > 0) {
                        NS_LOG_INFO(LogPrefixMacro << "in Receive hello, string stream has sth to read, for hello bundle-seqno, pkt_seqno_number=" << pkt_seqno_number);
                        if (pkt_seqno_number > 100) {
                            NS_LOG_ERROR(LogPrefixMacro << "Error: pkt_seqno_number > 100"
                                    << ",hello packet it receive is " << hello_packet_size
                                    << " bytes,follow info may help you know what happened");
                            for (int i = 0; i < pkt_seqno_number; ++i) {
                                dtn_seqno_t iv;
                                pkt_str_stream >> iv;
                                NS_LOG_ERROR(LogPrefixMacro << "error, seqno of " << i << "-th = " << iv);
                            }
                            std::abort();
                        }
                        for (int m = 0; m < pkt_seqno_number; m++) {
                            dtn_seqno_t tttmp;
                            pkt_str_stream >> tttmp;
                            tmpbaq_vec.push_back(tttmp);
                        }
                    }
                    // fill up baq with 'a' pkt
                    // check whether 'stringstream' has sth to read
                    for (int tmppp = pkt_str_stream.rdbuf()->in_avail(); tmppp > 1; tmppp = pkt_str_stream.rdbuf()->in_avail()) {
                        NS_LOG_INFO(LogPrefixMacro << "string stream has sth to read, hello anti seqno, tmppp=" << tmppp);
                        assert(tmppp < 100);
                        dtn_seqno_t tmp_antipacket_seqno;
                        pkt_str_stream >> tmp_antipacket_seqno;
                        // we need negative to indicate this seqno is from antipacket
                        tmpbaq_vec.push_back(tmp_antipacket_seqno);
                        bool anti_found = false;
                        int sentapseqno_size = neighbor_info_vec_[j].info_sent_ap_seqno_vec_.size();
                        assert(sentapseqno_size <= NS3DTNBIT_HYPOTHETIC_NEIGHBOR_BAQ_NUMBER_MAX);
                        for (int k = 0; ((k < sentapseqno_size) && (!anti_found)); k++) {
                            anti_found = neighbor_info_vec_[j].info_sent_ap_seqno_vec_[k] == tmp_antipacket_seqno ? true : false;
                            if (anti_found) {
                                UpdateNeighborInfoDetail(3, j, k);
                                UpdateNeighborInfoDetail(2, j, k);
                            }
                        }
                    }
                    neighbor_info_vec_[j].info_baq_seqno_vec_ = tmpbaq_vec;
                }
            }
            NS_LOG_LOGIC(LogPrefixMacro << "Out of " << "ReceiveHello()");
        }

        /* refine 
         * Aim : send ack imediatly
         */
        void DtnApp::ToSendAck(BPHeader& ref_bp_header, Ipv4Address response_ip) {
            NS_LOG_INFO(LogPrefixMacro << "enter ToSendAck()");
            std::string tmp_payload_str;
            {
                // fill up payload
                std::stringstream tmp_sstream;
                tmp_sstream << own_ip_; 
                tmp_sstream << " ";
                tmp_sstream << ref_bp_header.get_source_seqno();
                tmp_sstream << " ";
                tmp_sstream << ref_bp_header.get_payload_size();
                tmp_sstream << " ";
                tmp_sstream << ref_bp_header.get_offset();
                tmp_sstream << " ";
                tmp_sstream << ref_bp_header.get_retransmission_count();
                tmp_payload_str = tmp_sstream.str();
            }
            Ptr<Packet> p_pkt = Create<Packet>(tmp_payload_str.c_str(), tmp_payload_str.size());
            BPHeader bp_header;
            {
                // fill up bp_header
                SemiFillBPHeaderDetail(&bp_header);
                bp_header.set_bundle_type(BundleType::TransmissionAck);
                bp_header.set_destination_ip(response_ip);
                bp_header.set_source_seqno(p_pkt->GetUid());
                bp_header.set_payload_size(tmp_payload_str.size());
                bp_header.set_offset(tmp_payload_str.size());
            }
            p_pkt->AddHeader(bp_header);
            InetSocketAddress response_addr = InetSocketAddress(response_ip, NS3DTNBIT_PORT_NUMBER);
            if (!SocketSendDetail(p_pkt, 0, response_addr)) {
                NS_LOG_ERROR("SOCKET send error");
                std::abort();
            }
            //NS_LOG_INFO("out of ToSendAck()");
            NS_LOG_LOGIC(LogPrefixMacro << "Out of " << "ToSendAck()");
        };

        /* refine 
         * 
         * Aim : enqueue bundle
         */
        void DtnApp::ToSendBundle(uint32_t dstnode_number, uint32_t payload_size) {
            NS_LOG_DEBUG(LogPrefixMacro << "enter ToSendBundle()");
            // fill up payload 
            std::string tmp_payload_str;
            tmp_payload_str = std::string(payload_size, 'x');
            Ptr<Packet> p_pkt = Create<Packet>(tmp_payload_str.c_str(), tmp_payload_str.size());
            BPHeader bp_header;
            {
                // fill up bp_header
                SemiFillBPHeaderDetail(&bp_header);
                bp_header.set_bundle_type(BundleType::BundlePacket);
                char dststring[1024] = "";
                sprintf(dststring, "10.0.0.%d", dstnode_number + 1);
                bp_header.set_destination_ip(dststring);
                bp_header.set_source_seqno(p_pkt->GetUid());
                bp_header.set_payload_size(tmp_payload_str.size());
                bp_header.set_offset(tmp_payload_str.size());
            }
            assert(p_pkt->GetSize() == payload_size);
            NS_LOG_DEBUG(LogPrefixMacro << "ScheduleTx,inToSendBundle(),bp_header=" << bp_header);
            p_pkt->AddHeader(bp_header);
            if ((daemon_antipacket_queue_->GetNBytes() + daemon_bundle_queue_->GetNBytes() + p_pkt->GetSize() <= daemon_baq_pkts_max_ * NS3DTNBIT_HYPOTHETIC_CACHE_FACTOR)) {
                daemon_bundle_queue_->Enqueue(Packet2Queueit(p_pkt));
                // NORMAL LOG
                NS_LOG_DEBUG(LogPrefixMacro << "normal out of ToSendBundle()");
            } else {
                // ERROR LOG
                NS_LOG_ERROR("Error : bundle is too big or no space");
                std::abort();
            }
            NS_LOG_LOGIC(LogPrefixMacro << "Out of " << "ToSendBundle()");
        }

        /* refine 
         * Aim : enqueue anti_pkts
         * create and fill up antipacket-bundle then enqueue
         */
        void DtnApp::ToSendAntipacketBundle(BPHeader& ref_bp_header) {
            NS_LOG_DEBUG(LogPrefixMacro << "enter ToSendAntipacketBundle()");
            string anti_packet_payload_str;
            {
                // fill up payload buffer
                std::stringstream msg;
                Ipv4Address srcip = ref_bp_header.get_source_ip();
                srcip.Print(msg);
                msg << " ";
                Ipv4Address dstip = ref_bp_header.get_destination_ip();
                dstip.Print(msg);
                msg << " ";
                msg << ref_bp_header.get_source_seqno();
                anti_packet_payload_str = msg.str();
                NS_LOG_DEBUG(LogPrefixMacro << "NOTE: anti content is : " << anti_packet_payload_str);
            }
            Ptr<Packet> p_pkt = Create<Packet>(anti_packet_payload_str.c_str(), anti_packet_payload_str.size());
            BPHeader bp_header;
            {
                // fill up bp_header
                SemiFillBPHeaderDetail(&bp_header);
                bp_header.set_bundle_type(BundleType::AntiPacket);
                bp_header.set_destination_ip(ref_bp_header.get_source_ip());
                bp_header.set_source_seqno(p_pkt->GetUid());
                bp_header.set_payload_size(anti_packet_payload_str.size());
                bp_header.set_offset(anti_packet_payload_str.size());
            }
            p_pkt->AddHeader(bp_header);
            daemon_antipacket_queue_->Enqueue(Packet2Queueit(p_pkt));
            {
                // LOG 
                NS_LOG_DEBUG(LogPrefixMacro << "out of ToSendAntipacketBundle()");
            }
            NS_LOG_LOGIC(LogPrefixMacro << "Out of " << "ToSendAntipacketBundle()");
        }

        /* refine 
         * Aim : handle reception of bundle except hello one.
         * Detail :
         *     'ack bundle' : call 'ToTransmit' to transmit more(if have more) to the 'ack sender'
         *     'not ack bundle' : send a 'ack bundle', then get the information in that pkt
         *         'hello packet'
         *         'normal bundle'
         *         'antipacket bundle'
         * use daemon_reception_info_vec_ and daemon_reception_packet_buffer_vec_
         */
        void DtnApp::ReceiveBundle(Ptr<Socket> socket) {
            NS_LOG_DEBUG(LogPrefixMacro << "enter ReceiveBundle()");
            Address own_addr;
            socket->GetSockName(own_addr);
            InetSocketAddress tmp_own_s = InetSocketAddress::ConvertFrom(own_addr);
            own_ip_ = tmp_own_s.GetIpv4();
            int loop_count = 0;
            while (socket->GetRxAvailable() > 0) {
                NS_LOG_DEBUG(LogPrefixMacro << "ReceiveBundle(), loop_count =" << loop_count++
                        << ";socket->GetRxAvailable =" << socket->GetRxAvailable());
                Address from_addr;
                Ptr<Packet> p_pkt = socket->RecvFrom(from_addr);
                InetSocketAddress from_s_addr = InetSocketAddress::ConvertFrom(from_addr);
                from_s_addr.SetPort(NS3DTNBIT_PORT_NUMBER);
                Ipv4Address from_ip = from_s_addr.GetIpv4();
                BPHeader bp_header;
                p_pkt->RemoveHeader(bp_header);
                if (p_pkt->GetSize() == 0) {
                    NS_LOG_ERROR(LogPrefixMacro << "ERROR: can't be size = 0, bp_header.get_payload_size =" << bp_header.get_payload_size() << ";bundle_type=" << bp_header.get_bundle_type());
                    std::abort();
                }
                NS_LOG_INFO(LogPrefixMacro << "receive bundle_type_ = " << bp_header.get_bundle_type());
                if (bp_header.get_bundle_type() == BundleType::TransmissionAck) {
                    // if is 'ack' update state, get ack src-dst seqno, timestamp, bytes. then call 'ToTransmit' to transmit more
                    NS_LOG_INFO(LogPrefixMacro << "Receive a ACK");
                    Ipv4Address ip_ack_from;
                    dtn_seqno_t seqno_was_acked;
                    uint32_t bundle_total_payload; // note fragment
                    uint32_t acked_offset; 
                    uint32_t retransmit_count;
                    std::stringstream ss;
                    p_pkt->CopyData(&ss, bp_header.get_payload_size());
                    ss >> ip_ack_from >> seqno_was_acked >> bundle_total_payload >> acked_offset >> retransmit_count;
                    DaemonBundleHeaderInfo tmp_bh_info = {
                        ip_ack_from,
                        retransmit_count,
                        seqno_was_acked
                    };
                    NS_LOG_LOGIC(LogPrefixMacro << "tmp_bh_info - seqno=" << tmp_bh_info.info_source_seqno_);
                    int k = 0;
                    for (int kk = 0; kk < transmit_assister_.daemon_transmission_info_vec_.size(); kk++) {
                        if (tmp_bh_info == transmit_assister_.daemon_transmission_bh_info_vec_[kk]) { k = kk; break; }
                    }
                    int total = transmit_assister_.daemon_transmission_info_vec_[k].info_transmission_total_send_bytes_, current = transmit_assister_.daemon_transmission_info_vec_[k].info_transmission_current_sent_acked_bytes_, last =transmit_assister_.daemon_transmission_info_vec_[k].info_transmission_bundle_last_sent_bytes_;
                    if (total ==  current + last) {
                        NS_LOG_DEBUG(LogPrefixMacro << "good! we know the bundle has been accept, this transmit-session can be close");
                        transmit_assister_.daemon_transmission_info_vec_[k].info_transmission_current_sent_acked_bytes_ += last;
                    } else if (total > current + last){
                        transmit_assister_.daemon_transmission_info_vec_[k].info_transmission_current_sent_acked_bytes_ += last;
                        NS_LOG_DEBUG(LogPrefixMacro << "here, before ToTransmit(), to transmit more"
                                << "\ntotal =" << total << "\ncurrent_sent=" 
                                << current << "\nlast_sent=" << last);
                        ToTransmit(tmp_bh_info, false);
                    } else {
                        NS_LOG_ERROR(LogPrefixMacro << "can't be true, total, cur, last =" << total << " " << current << " " << last 
                                << "\n k =" << k  << " " << transmit_assister_.daemon_transmission_info_vec_.size() 
                                << " " << transmit_assister_.daemon_retransmission_packet_buffer_vec_.size() 
                                << " " << transmit_assister_.daemon_transmission_bh_info_vec_.size()
                                << "\n" << "from_ip = " << transmit_assister_.daemon_transmission_bh_info_vec_[k].info_transmit_addr_);
                        std::abort();
                    }
                    return;
                } else {
                    // if not, send 'transmission ack' first
                    NS_LOG_DEBUG(LogPrefixMacro << "here, received anti or bundle, send ack back first, before ToSendAck" << "; ip=" << from_ip 
                            << "; bp_header=" << bp_header);
                    ToSendAck(bp_header, from_ip);
                    seqno2fromid_map_[bp_header.get_source_seqno()] = Ipv42NodeNo(from_ip);
                    {
                        // process bundle or anti-pkt
                        NS_LOG_DEBUG(LogPrefixMacro << "here; after send ack back, process recept bp_header: " << bp_header);
                        if (bp_header.get_bundle_type() == BundleType::AntiPacket) {
                            // antipacket must not be fragment, it's safe to directly process
                            // keep antipacket and remove the bundle 'corresponded to'
                            BPHeader tmp_bp_header = bp_header;
                            p_pkt->AddHeader(bp_header);
                            if (!IsDuplicatedDetail(tmp_bp_header)) {
                                daemon_antipacket_queue_->Enqueue(Packet2Queueit(p_pkt->Copy()));
                                RemoveBundleFromAntiDetail(p_pkt);
                            } else {
                                NS_LOG_WARN(LogPrefixMacro << "WARN:duplicate anti-pkt, may happen");
                            }
                        } else if (bp_header.get_bundle_type() == BundleType::BundlePacket) {
                            // init the receiving pkt info
                            DaemonReceptionInfo tmp_recept_info = {
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
                            if (bp_header.get_payload_size() > p_pkt->GetSize()) {
                                NS_LOG_WARN(LogPrefixMacro << "WARN:fragment bundle? it may happens, bp_header.get_payload_size =" << bp_header.get_payload_size() << ";p_pkt->GetSize=" << p_pkt->GetSize());
                                bool reception_info_found = false;
                                int k = 0;
                                // if one bundle is fragment, it should be searched in 'daemon_reception_info_vec_'
                                for (int i = 0; i < daemon_reception_info_vec_.size(); i++) {
                                    if (tmp_recept_info.info_bundle_source_ip_.IsEqual(daemon_reception_info_vec_[i].info_bundle_source_ip_)
                                            && tmp_recept_info.info_bundle_seqno_ == daemon_reception_info_vec_[i].info_bundle_seqno_
                                            && tmp_recept_info.info_trasmission_receive_from_ip_.IsEqual(daemon_reception_info_vec_[i].info_trasmission_receive_from_ip_)) {
                                        reception_info_found = true;
                                        k = i;
                                        break;
                                    }
                                }
                                // is recorded, keep receiving and check order of fragment, 
                                // add new fragment to 'daemon_reception_packet_buffer_vec_' 
                                // when all fragment received, parse this packet, deal with it
                                if (reception_info_found) {
                                    p_pkt->AddHeader(bp_header);
                                    daemon_reception_info_vec_[k].info_fragment_pkt_pointer_vec_.push_back(p_pkt);
                                    FragmentReassembleDetail(k);
                                } else {
                                    // not recorded, recorded
                                    daemon_reception_info_vec_.push_back(tmp_recept_info);
                                    p_pkt->AddHeader(bp_header);
                                    daemon_reception_packet_buffer_vec_.push_back(p_pkt);
                                }
                            } else if (bp_header.get_payload_size() == p_pkt->GetSize()){
                                daemon_reception_info_vec_.push_back(tmp_recept_info);
                                p_pkt->AddHeader(bp_header);
                                daemon_reception_packet_buffer_vec_.push_back(p_pkt);
                            } else {
                                NS_LOG_ERROR("ERROR:payload > pkt-size, can't be!");
                                std::abort();
                            }
                            BundleReceptionTailWorkDetail();
                        } else {
                            NS_LOG_ERROR(LogPrefixMacro << "ERROR:error! no possible");
                            std::abort();
                        } // later usage
                        NS_LOG_DEBUG(LogPrefixMacro << "out of recervebundle");
                    }
                }

            }
            NS_LOG_LOGIC(LogPrefixMacro << "Out of " << "ReceiveBundle()");
        }

        /* refine 
         * check whether all fragment received
         * use AddAtEnd
         * set hop time
         */
        void DtnApp::FragmentReassembleDetail(int k) {
            NS_LOG_INFO(LogPrefixMacro << "enter FragmentReassembleDetail()");
            Ptr<Packet> first_arrive_p_pkt = daemon_reception_packet_buffer_vec_[k];
            Ptr<Packet> reassemble_p_pkt;
            BPHeader tmp_bph;
            int current_fragment_bytes = 0,totoal_bytes = 0;
            vector<std::pair<int, int>> x_info_vec;
            daemon_reception_info_vec_[k].info_fragment_pkt_pointer_vec_.push_back(first_arrive_p_pkt);
            for (int i = 0; i < daemon_reception_info_vec_[k].info_fragment_pkt_pointer_vec_.size(); i++) {
                BPHeader bp_header;
                daemon_reception_info_vec_[k].info_fragment_pkt_pointer_vec_[i]->RemoveHeader(bp_header);
                totoal_bytes = bp_header.get_payload_size();
                current_fragment_bytes += daemon_reception_info_vec_[k].info_fragment_pkt_pointer_vec_[i]->GetSize();
                x_info_vec.push_back({ i , bp_header.get_offset() });
                daemon_reception_info_vec_[k].info_fragment_pkt_pointer_vec_[i]->AddHeader(bp_header);
            }
            if (totoal_bytes == current_fragment_bytes) {
                std::sort(x_info_vec.begin(), x_info_vec.end(), [](std::pair<int, int> a, std::pair<int, int> b) { return std::get<1>(a) < std::get<1>(b); });
                for (auto t : x_info_vec) {
                    Ptr<Packet> tmp_p;
                    tmp_p = daemon_reception_info_vec_[k].info_fragment_pkt_pointer_vec_[std::get<0>(t)];
                    tmp_p->RemoveHeader(tmp_bph);
                    reassemble_p_pkt->AddAtEnd(tmp_p);
                }
                reassemble_p_pkt->AddHeader(tmp_bph);
                daemon_reception_packet_buffer_vec_[k] = reassemble_p_pkt;
            } else {
                daemon_reception_info_vec_[k].info_fragment_pkt_pointer_vec_.pop_back();
            }
            NS_LOG_LOGIC(LogPrefixMacro << "Out of " << "FragmentReassembleDetail()");
        }

        /* refine 
         * Aim : recept tail work
         * Detail :
         * update state 
         */
        void DtnApp::BundleReceptionTailWorkDetail() {
            NS_LOG_INFO(LogPrefixMacro << "enter BundleReceptionTailWorkDetail()");
            auto it = daemon_reception_packet_buffer_vec_.rbegin();
            Ptr<Packet> tmp_p_pkt = (*it)->Copy();
            BPHeader bp_header;
            tmp_p_pkt->RemoveHeader(bp_header);
            if (bp_header.get_payload_size() == tmp_p_pkt->GetSize()) {
                // this bundle is non-fragment or a already reassemble one
                if (IsDuplicatedDetail(bp_header)) {
                    // check Duplicates here
                    NS_LOG_WARN(LogPrefixMacro << "WARN: receive a duplicated bundle, this may happen");
                    return;
                }
                if (bp_header.get_destination_ip().IsEqual(own_ip_)) {
                    NS_LOG_DEBUG(LogPrefixMacro << "NOTE:BundleTrace:Great! one bundle arrive destination! bp_header=" << bp_header);
                    ToSendAntipacketBundle(bp_header);
                    tmp_p_pkt->AddHeader(bp_header);
                    daemon_consume_bundle_queue_->Enqueue(Packet2Queueit(tmp_p_pkt->Copy()));
                    // this is a heuristic method to make hello, to let others know it already has it.
                    daemon_bundle_queue_->Enqueue(Packet2Queueit(tmp_p_pkt->Copy()));
                } else {
                    if (bp_header.get_bundle_type() == BundleType::BundlePacket) {
                        NS_LOG_DEBUG(LogPrefixMacro << "NOTE:BundleTrace:good! one bundle recept, it's one hop! bp_header=" << bp_header);
                    } else {
                        NS_LOG_ERROR(LogPrefixMacro << "ERROR: can't be");
                    }
                    tmp_p_pkt->AddHeader(bp_header);
                    daemon_bundle_queue_->Enqueue(Packet2Queueit(tmp_p_pkt->Copy()));
                }
            } else {
                NS_LOG_ERROR(LogPrefixMacro << "fragment not solved!");
                std::abort();
            }
            NS_LOG_LOGIC(LogPrefixMacro << "Out of " << "BundleReceptionTailWorkDetail()");
        }

        void DtnApp::TransmitSessionFailCheck(DaemonBundleHeaderInfo bh_info, int last_time_current) {
            NS_LOG_INFO(LogPrefixMacro << "Enter TransmitSessionFailCheck");
            bool this_session_was_acked = false;
            int index = 0;
            for (auto v : transmit_assister_.daemon_transmission_bh_info_vec_) {
                if (v == bh_info) {
                    if (last_time_current < transmit_assister_.daemon_transmission_info_vec_[index].info_transmission_current_sent_acked_bytes_) {
                        this_session_was_acked = true;
                    }
                    break;
                } else {
                    index += 1;
                }
            }
            if (this_session_was_acked) {
                NS_LOG_DEBUG(LogPrefixMacro << "seqno=" << bh_info.info_source_seqno_ << "; this is acked, successed! ; detail is : last_time_current =" << last_time_current << "; now_current =" << transmit_assister_.daemon_transmission_info_vec_[index].info_transmission_current_sent_acked_bytes_ );
            } else {
                // we don't need to roll back spray_map_, here
                NS_LOG_DEBUG(LogPrefixMacro << "seqno=" << bh_info.info_source_seqno_ << "; this is not acked, retransmit!");
                ToTransmit(bh_info, true);
            }
            NS_LOG_LOGIC(LogPrefixMacro << "Out of " << "TransmitSessionFailCheck()");
        }
        /* refine 
         * Aim : handle send bundle stuff after checkbuffer
         * Detail :
         * use bh_info as key get all msg, 
         * check whether it's able to call 'SocketSendDetail'
         * handle retransmit and transmit more
         * use transmit_assister_.daemon_retransmission_packet_buffer_vec_
         * note the packet size problem, size < NS3DTNBIT_HYPOTHETIC_TRANS_SIZE_FRAGMENT_MAX
         * update 'transmit_assister_.daemon_transmission_info_vec_'
         * anti-pkt or bundle-pkt
         */
        void DtnApp::ToTransmit(DaemonBundleHeaderInfo bh_info, bool is_retransmit) {
            NS_LOG_DEBUG(LogPrefixMacro << "enter ToTransmit()");
            NS_LOG_DEBUG("----- seqno=" << bh_info.info_source_seqno_);
            bool real_send_boolean = false;
            int index = 0, j = 0;
            for (int ii = 0; ii < transmit_assister_.daemon_transmission_bh_info_vec_.size(); ii++) {
                // find the index of the 'bundle to transmit' to 'transmit_assister_.daemon_transmission_info_vec_' using 'bh_info'
                if (transmit_assister_.daemon_transmission_bh_info_vec_[ii] == bh_info) { index = ii; break; }
            }
            // check state, cancel transmission if condition
            if (transmit_assister_.daemon_transmission_info_vec_[index].info_transmission_total_send_bytes_ == transmit_assister_.daemon_transmission_info_vec_[index].info_transmission_current_sent_acked_bytes_) {
                NS_LOG_DEBUG(LogPrefixMacro << "this transmit-session has been done! we would return and drop this transmit." 
                        << "transmit-to-ip=" << bh_info.info_transmit_addr_.GetIpv4()
                        << ";seqno=" << bh_info.info_source_seqno_
                        << "; if this transmit-to-ip is equal to last, the robin-round schedule may not work");
                return;
            }
            if (transmit_assister_.daemon_transmission_bh_info_vec_[index].info_retransmission_count_ > NS3DTNBIT_MAX_RETRANSMISSION) {
                NS_LOG_DEBUG(LogPrefixMacro << "this transmit-session is over max-retranmission time , would drop this transmit." 
                        << "transmit-to-ip=" << bh_info.info_transmit_addr_.GetIpv4()
                        << ";seqno=" << bh_info.info_source_seqno_);
                return;
            }
            for (int jj = 0; jj < neighbor_info_vec_.size(); jj++) {
                // find the neighbor should be transmit, if this neighbor was not recently seen, schedule 'ToTransmit' later, otherwise, set real_send_boolean
                auto ip_n = neighbor_info_vec_[jj].info_address_.GetIpv4();
                auto ip_to = bh_info.info_transmit_addr_.GetIpv4();
                if (ip_n.IsEqual(ip_to)) {
                    if (neighbor_info_vec_[jj].IsLastSeen()) {
                        real_send_boolean = true;
                        j = jj;
                        break;
                    } else {
                        Simulator::Schedule(Seconds(NS3DTNBIT_HELLO_BUNDLE_INTERVAL_TIME * 3), &DtnApp::ToTransmit, this, bh_info, is_retransmit);
                        NS_LOG_WARN(LogPrefixMacro << "WARN:to transmit: can't find neighbor or neighbor not recently seen. This might happen when ToTransmit() with is_retransmit = true, we would cancel this try, and retry next time." << "j=" << jj << ",last seen time=" << (double)neighbor_info_vec_[jj].info_last_seen_time_ << ";base time=" << Simulator::Now().GetSeconds() - (NS3DTNBIT_HELLO_BUNDLE_INTERVAL_TIME));
                        return;
                    }
                }
            }
            BPHeader tran_bp_header;
            int offset_value;
            if (real_send_boolean) {
                //ref_tran_p_pkt = transmit_assister_.daemon_retransmission_packet_buffer_vec_[index]->Copy(); 
                Ptr<Packet>& ref_tran_p_pkt= transmit_assister_.daemon_retransmission_packet_buffer_vec_[index];
                if (is_retransmit) {
                    NS_LOG_DEBUG("---- seqno=" << bh_info.info_source_seqno_ << ";retransmit_count  = " << transmit_assister_.daemon_transmission_bh_info_vec_[index].info_retransmission_count_);
                }
                ref_tran_p_pkt->RemoveHeader(tran_bp_header);
                if (transmit_assister_.daemon_transmission_info_vec_[index].info_transmission_total_send_bytes_ > NS3DTNBIT_HYPOTHETIC_TRANS_SIZE_FRAGMENT_MAX) {
                    NS_LOG_WARN(LogPrefixMacro << "WARN:fragment may have error");
                    {
                        // prepare bytes
                        int need_to_bytes = transmit_assister_.get_need_to_bytes(index);
                        NS_LOG_INFO(LogPrefixMacro << "here" << ";daemon_reception_packet_buffer_vec_.size()" << daemon_reception_packet_buffer_vec_.size() << ";index" << index);
                        ref_tran_p_pkt->RemoveAtStart(transmit_assister_.daemon_transmission_info_vec_[index].info_transmission_current_sent_acked_bytes_);
                        if (need_to_bytes > NS3DTNBIT_HYPOTHETIC_TRANS_SIZE_FRAGMENT_MAX) {
                            ref_tran_p_pkt->RemoveAtEnd(need_to_bytes - NS3DTNBIT_HYPOTHETIC_TRANS_SIZE_FRAGMENT_MAX);
                        }
                        tran_bp_header.set_offset(transmit_assister_.daemon_transmission_info_vec_[index].info_transmission_current_sent_acked_bytes_ + ref_tran_p_pkt->GetSize());
                        if (is_retransmit) {
                            tran_bp_header.set_retransmission_count(tran_bp_header.get_retransmission_count() + 1);
                            transmit_assister_.daemon_transmission_bh_info_vec_[index].info_retransmission_count_ += 1;
                        }
                        offset_value = ref_tran_p_pkt->GetSize() + transmit_assister_.daemon_transmission_info_vec_[index].info_transmission_current_sent_acked_bytes_;
                    }
                } else {
                    offset_value = ref_tran_p_pkt->GetSize();
                    assert(offset_value == tran_bp_header.get_payload_size());
                    if (offset_value == 0) {
                        NS_LOG_ERROR(LogPrefixMacro << "ref_tran_p_pkt.size() = 0" << " bp_header :" << tran_bp_header);
                        std::abort();
                    }
                }
                assert(ref_tran_p_pkt->GetSize()!=0);
                tran_bp_header.set_offset(offset_value);
                ref_tran_p_pkt->AddHeader(tran_bp_header);
                {
                    // update state
                    transmit_assister_.daemon_transmission_info_vec_[index].info_transmission_bundle_last_sent_time_ = Simulator::Now().GetSeconds();
                    transmit_assister_.daemon_transmission_info_vec_[index].info_transmission_bundle_last_sent_bytes_ = ref_tran_p_pkt->GetSize();
                    transmit_assister_.daemon_transmission_bh_info_vec_[index].info_retransmission_count_ = tran_bp_header.get_retransmission_count();
                    if (tran_bp_header.get_bundle_type() == BundleType::AntiPacket) {
                        neighbor_info_vec_[j].info_sent_ap_seqno_vec_.push_back(tran_bp_header.get_source_seqno());
                        neighbor_info_vec_[j].info_sent_ap_time_vec_.push_back(Simulator::Now().GetSeconds());
                    } else {
                        neighbor_info_vec_[j].info_sent_bp_seqno_vec_.push_back(tran_bp_header.get_source_seqno());
                    }
                }
                {
                    // fail check
                    int last_time_current = transmit_assister_.daemon_transmission_info_vec_[index].info_transmission_current_sent_acked_bytes_;
                    Simulator::Schedule(Seconds(retransmission_interval_), &DtnApp::TransmitSessionFailCheck, this, bh_info, last_time_current);
                }
                NS_LOG_DEBUG(LogPrefixMacro << "before SocketSendDetail,ref_tran_p_pkt.size()=" << ref_tran_p_pkt->GetSize() << ";transmit ip=" << neighbor_info_vec_[j].info_address_.GetIpv4() << ";tran_bp_header : " << tran_bp_header);
                if (!SocketSendDetail(ref_tran_p_pkt, 0, neighbor_info_vec_[j].info_address_)) {
                    NS_LOG_ERROR("SocketSendDetail fail");
                    std::abort();
                }
            }
            NS_LOG_LOGIC(LogPrefixMacro << "Out of " << "ToTransmit()");
        }

        /*
         * Aim : A RoutingMethod switcher
         * TODO
         * define your decision method
         */
        bool DtnApp::FindTheNeighborThisBPHeaderTo(BPHeader& ref_bp_header, int& return_index_of_neighbor_you_dedicate, DtnApp::CheckState check_state) {
            NS_LOG_INFO(LogPrefixMacro << "enter FindTheNeighborThisBPHeaderTo()");
            if (routing_assister_.IsSet() && routing_assister_.get_rm() == RoutingMethod::SprayAndWait) {
                NS_LOG_INFO(LogPrefixMacro << "RoutingMethod is SprayAndWait");
                auto ip_d = ref_bp_header.get_destination_ip();
                if (ip_d == own_ip_) {return false;}
                // this method is default one
                vector<int> available = BPHeaderBasedSendDecisionDetail(ref_bp_header, check_state);
                if (available.size() > 0) {
                    return_index_of_neighbor_you_dedicate = available[0];
                    return true;
                } else {
                    return false;
                }
            } else if (routing_assister_.IsSet() && routing_assister_.get_rm() == RoutingMethod::Other) {
                NS_LOG_INFO(LogPrefixMacro << "RoutingMethod is Other");
                int s, d, indx = -1, result;
                {
                    // init s, d
                    auto ip_s = ref_bp_header.get_source_ip();
                    auto ip_d = ref_bp_header.get_destination_ip();
                    if (ip_d == own_ip_) {NS_LOG_INFO(LogPrefixMacro << "routing self, would return false and continue."); return false;} 
                    s = Ipv42NodeNo(ip_s);
                    d = Ipv42NodeNo(ip_d);
                    s = node_->GetId();
                }
                NS_LOG_DEBUG(LogPrefixMacro << "NOTE: before YouRouting method");
                result = routing_assister_.RouteIt(node_->GetId(), d);
                if (result == node_->GetId()) {NS_LOG_WARN(LogPrefixMacro << "WARN: routing self! s=" << s << ";d=" << d << ";result = " << result);}
                NS_LOG_DEBUG(LogPrefixMacro << "NOTE: after YouRouting method, result =" << result << "source =" << s << "dest=" << d);
                if (result != -1) {
                    /*
                       auto ip_base = NodeNo2Ipv4(result);
                       for (int i = 0; i < neighbor_info_vec_.size(); i++) {
                       auto nip = neighbor_info_vec_[i].info_address_.GetIpv4();
                       if (nip.IsEqual(ip_base)) { indx = i; break; }
                       }
                       */
                    indx = nodeid2neighborvecindex(neighbor_info_vec_, result);
                    vector<int> available = BPHeaderBasedSendDecisionDetail(ref_bp_header, check_state);
                    for (auto v : available) {
                        if (v == indx) {
                            return_index_of_neighbor_you_dedicate = indx;
                            NS_LOG_DEBUG(LogPrefixMacro << "NOTE:your decision is made, to-node-id =" << result << "; index of neighbor_info_vec_ is = " << indx);
                            return true;
                        }
                    }
                    if (available.size() == 0) {
                        NS_LOG_WARN(LogPrefixMacro << "WARN: available is none.");
                        return false;
                    } else {
                        NS_LOG_INFO(LogPrefixMacro << "routing decision is not in available, or have be sent; we would transmit this to an available," << " to-node-id =" << result << "; index of correspond neighbor of result is = " << indx << "all available is: ");
                        for (auto v : available) { NS_LOG_INFO("v = " << v << "."); }
                        return_index_of_neighbor_you_dedicate = available[0];
                        return true;
                    }
                } else {
                    return false;
                }
            } else if (routing_assister_.IsSet() && routing_assister_.get_rm() == RoutingMethod::TimeExpanded) {
                NS_LOG_INFO(LogPrefixMacro << "RoutingMethod is TimeExpanded");
                // s, d is index of node, indx is index of neighbor
                int s, d, indx = -1, result;
                {
                    // init s, d
                    auto ip_s = ref_bp_header.get_source_ip();
                    auto ip_d = ref_bp_header.get_destination_ip();
                    if (ip_d == own_ip_) {NS_LOG_INFO(LogPrefixMacro << "WARN: to self?"); return false;}
                    s = Ipv42NodeNo(ip_s);
                    d = Ipv42NodeNo(ip_d);
                }
                vector<int> available = BPHeaderBasedSendDecisionDetail(ref_bp_header, check_state);
                // Note that this method just indicate which node the next hop would be,
                // if the next hop is not available yet, should wait for it till available
                NS_LOG_DEBUG(LogPrefixMacro << ">>NOTE: before TimeExpanded method");
                result = routing_assister_.RouteIt(node_->GetId(), d);
                NS_LOG_DEBUG(LogPrefixMacro << ">>NOTE: after TimeExpanded method, result =" << result);
                if (result == node_->GetId()) {NS_LOG_ERROR(LogPrefixMacro << "Error: routing ! s=" << s << ";d=" << d << ";result = " << result); std::abort();}
                if (result != -1) {
                    /*
                       auto ip_base = NodeNo2Ipv4(result);
                       for (int i = 0; i < neighbor_info_vec_.size(); i++) {
                    // result to index of neighbor vec
                    auto nip = neighbor_info_vec_[i].info_address_.GetIpv4();
                    if (nip.IsEqual(ip_base)) { indx = i; break; }
                    }
                    */
                    indx = nodeid2neighborvecindex(neighbor_info_vec_, result);
                    bool result_is_in_available = false;
                    for (auto v : available) {
                        if (v == indx) {
                            result_is_in_available = true;
                            return_index_of_neighbor_you_dedicate = indx;
                        }
                    }
                    if (available.size() == 0) { NS_LOG_WARN(LogPrefixMacro << "WARN: available is none."); return false; }
                    if (result_is_in_available) {
                        NS_LOG_DEBUG(LogPrefixMacro << "NOTE:your decision is made, to-node-id =" << result << "; index of neighbor_info_vec_ is = " << indx);
                        return true;
                    } else {
                        NS_LOG_INFO(LogPrefixMacro << "routing decision is not available, or have been sent;" << " to-node-id =" << result << "; index of neighbor_info_vec_ is = " << indx << "all available is: ");
                        return false;
                    }
                } else {
                    return false;
                }
            } else if (routing_assister_.IsSet() && routing_assister_.get_rm() == RoutingMethod::CGR) {
                NS_LOG_INFO(LogPrefixMacro << "RoutingMethod is CGR");
                int destination_id = Ipv42NodeNo(ref_bp_header.get_destination_ip());
                int from_id = -1;
                auto found = seqno2fromid_map_.find(ref_bp_header.get_source_seqno());
                if (found != seqno2fromid_map_.end()) {
                    from_id = seqno2fromid_map_[ref_bp_header.get_source_seqno()];
                } else {
                    from_id = node_->GetId();
                }
                vector<int> vec_of_current_neighbor;
                for (auto nei : neighbor_info_vec_) {
                    if (nei.IsLastSeen()) {
                        vec_of_current_neighbor.push_back(Ipv42NodeNo(nei.info_address_.GetIpv4()));
                    }
                }
                int own_id = node_->GetId();
                dtn_time_t expired_time = ref_bp_header.get_src_time_stamp().GetSeconds();
                expired_time += NS3DTNBIT_HYPOTHETIC_BUNDLE_EXPIRED_TIME;
                int bundle_size = ref_bp_header.get_payload_size();
                int flag = 0;

                int s, d, indx = -1, result;
                {
                    // init s, d
                    auto ip_s = ref_bp_header.get_source_ip();
                    auto ip_d = ref_bp_header.get_destination_ip();
                    if (ip_d == own_ip_) {NS_LOG_INFO(LogPrefixMacro << "WARN: to self, this happens when bundle remains in destination node, so this destination of this bundle is this node. It's purposed."); return false;}
                    s = Ipv42NodeNo(ip_s);
                    d = Ipv42NodeNo(ip_d);
                }

                // -------------- dividing ----------
                routing_assister_.p_rm_in_->GetInfo(destination_id, from_id, vec_of_current_neighbor, own_id, expired_time, bundle_size, flag, id2cur_exclude_vec_of_id_);
                vector<int> available = BPHeaderBasedSendDecisionDetail(ref_bp_header, check_state);
                NS_LOG_DEBUG(LogPrefixMacro << ">>NOTE: Before CGR method");
                result = routing_assister_.RouteIt(node_->GetId(), d);
                NS_LOG_DEBUG(LogPrefixMacro << ">>NOTE: After CGR method, result =" << result);
                // -------------- dividing ----------
                if (result == node_->GetId()) {NS_LOG_ERROR(LogPrefixMacro << "Error: routing ! s=" << s << ";d=" << d << ";result = " << result); std::abort();}
                if (result != -1) {
                    indx = nodeid2neighborvecindex(neighbor_info_vec_, result);
                    bool result_is_in_available = false;
                    for (auto v : available) {
                        if (v == indx) {
                            result_is_in_available = true;
                            return_index_of_neighbor_you_dedicate = indx;
                        }
                    }
                    if (available.size() == 0) { NS_LOG_WARN(LogPrefixMacro << "WARN: available is none."); return false; }
                    if (result_is_in_available) {
                        NS_LOG_DEBUG(LogPrefixMacro << "NOTE:your decision is made, to-node-id =" << result << "; index of neighbor_info_vec_ is = " << indx);
                        return true;
                    } else {
                        NS_LOG_INFO(LogPrefixMacro << "routing decision is not available, or have been sent;" << " to-node-id =" << result << "; index of neighbor_info_vec_ is = " << indx << "all available is: ");
                        return false;
                    }
                } else {
                    NS_LOG_WARN(LogPrefixMacro << "WARN:result = -1, not good");
                    return false;
                }
            } else {
                NS_LOG_ERROR("can't find the routing method or method not assigned, routing_assister_ is set=" << routing_assister_.IsSet());
                std::abort();
            }
            NS_LOG_LOGIC(LogPrefixMacro << "Out of " << "FindTheNeighborThisBPHeaderTo()");
        }

        bool DtnApp::NeighborInfo::IsLastSeen() {
            return Simulator::Now().GetSeconds() - info_last_seen_time_ < NS3DTNBIT_HELLO_BUNDLE_INTERVAL_TIME;
        }

        void DtnApp::StateCheckDetail() {
            NS_LOG_DEBUG(LogPrefixMacro << "NOTE: Statecheck :"
                    << "\nneighbor_info_vec_.size()=" <<neighbor_info_vec_.size()
                    // TODO not using PeriodReorderDaemonBundleQueueDetail yet
                    << "\ndaemon_reorder_buffer_queue_->GetNPackets()=" << daemon_reorder_buffer_queue_->GetNPackets()
                    << "\ndaemon_antipacket_queue_->GetNPackets()=" << daemon_antipacket_queue_->GetNPackets()
                    << "\ndaemon_consume_bundle_queue_->GetNPackets()=" << daemon_consume_bundle_queue_->GetNPackets()
                    << "\ndaemon_bundle_queue_->GetNPackets()=" << daemon_bundle_queue_->GetNPackets()
                    << "\n === reception ==="
                    << "\ndaemon_reception_packet_buffer_vec_.size()=" << daemon_reception_packet_buffer_vec_.size()
                    << "\ndaemon_reception_info_vec_.size()=" <<daemon_reception_info_vec_.size()
                    << "\n === transmition ==="
                    << "\ntransmit_assister_.daemon_retransmission_packet_buffer_vec_.size()=" <<transmit_assister_.daemon_retransmission_packet_buffer_vec_.size()
                    << "\ntransmit_assister_.daemon_transmission_info_vec_.size()=" <<transmit_assister_.daemon_transmission_info_vec_.size()
                    << "\ntransmit_assister_.daemon_transmission_bh_info_vec_.size()=" <<transmit_assister_.daemon_transmission_bh_info_vec_.size()
                    << "\n === count ==="
                    << "\n bundle_send_count_ =" << bundle_send_count_
                    << "\n anti_send_count_ =" << anti_send_count_
                    << "\n ack_send_count_ =" << ack_send_count_
                    << "\n === END ==="
                    );
            for (int c = 0; c < daemon_bundle_queue_->GetNPackets(); c++) {
                Ptr<Packet> cp = daemon_bundle_queue_->Dequeue()->GetPacket();
                BPHeader ch;
                cp->RemoveHeader(ch);
                NS_LOG_DEBUG("daemon bundle queue - bundle header : " << ch);
                cp->AddHeader(ch);
                daemon_bundle_queue_->Enqueue(Packet2Queueit(cp));
            }
            if (daemon_antipacket_queue_->GetNPackets() +
                    daemon_consume_bundle_queue_->GetNPackets() +
                    daemon_reorder_buffer_queue_->GetNPackets() +
                    daemon_bundle_queue_->GetNPackets() +
                    daemon_reception_packet_buffer_vec_.size() +
                    transmit_assister_.daemon_retransmission_packet_buffer_vec_.size() +
                    daemon_reception_info_vec_.size() +
                    neighbor_info_vec_.size() +
                    transmit_assister_.daemon_transmission_info_vec_.size() +
                    transmit_assister_.daemon_transmission_bh_info_vec_.size() < daemon_baq_pkts_max_ * 2) {
            } else {
                NS_LOG_ERROR(LogPrefixMacro << "ERROR: queue and vecotr too big");
                std::abort();
            }
            //if (Simulator::Now().GetSeconds() > 150) { std::abort(); }
            Simulator::Schedule(Seconds(10), &DtnApp::StateCheckDetail, this);
            NS_LOG_LOGIC(LogPrefixMacro << "Out of " << "StateCheckDetail()");
        }

        /*
         *  real send flags == 1
         * */
        bool DtnApp::SprayGoodDetail(BPHeader& bp_header, int flag) {
            int v = bp_header.get_source_seqno();
            if (flag == 1) {
                bp_header.set_hop_time_stamp(Simulator::Now());
                bp_header.set_hop_ip(NodeNo2Ipv4(node_->GetId()));
            }
            auto found = spray_map_.find(v);
            if (found == spray_map_.end()) {
                spray_map_[v] = bp_header.get_spray();
                return true;
            } else {
                if (spray_map_[v] > 0) {
                    spray_map_[v] -= flag;
                    return true;
                } else {
                    return false;
                }
            }
        }

        /* refine 
         * Aim : The main loop for this app
         * Usage : checkbuffer is only responsible to init 'transmit session' and invoke 'totransmit'
         * Detail :
         * check your 'bundle queue' buffer and other related buffer periodly
         */
        void DtnApp::CheckBuffer(CheckState check_state) {
            string strrr;
            if (check_state == CheckState::State_2) { strrr = "anti check"; } else if (check_state == CheckState::State_1) {strrr = "bundle check";}
            NS_LOG_INFO(LogPrefixMacro << "Enter check buffer()" << strrr);
            if (!daemon_socket_handle_) {
                CreateSocketDetail();
                if (daemon_socket_handle_) {
                    NS_LOG_WARN(LogPrefixMacro << "NOTE:deamon_socket_handle not init, init now");
                } else {
                    NS_LOG_ERROR(LogPrefixMacro << "ERROR: can't init socket");
                }
            }
            // remove expired antipackets and bundles
            RemoveExpiredBAQDetail();
            //PeriodReorderDaemonBundleQueueDetail();
            // one time one pkt would be sent
            Ptr<Packet> p_pkt;
            BPHeader bp_header;
            // check and set real_send_boolean
            int decision_neighbor = -1;
            bool real_send_boolean = false;
            if (wifi_ph_p->IsStateIdle()) {
                NS_LOG_LOGIC(LogPrefixMacro << "is stateidle");
                if (check_state == CheckState::State_2) {
                    // go through daemon_antipacket_queue_ to find whether real_send_boolean should be set true
                    NS_LOG_DEBUG(LogPrefixMacro << "we have NPackets = " << daemon_antipacket_queue_->GetNPackets());
                    for (int n = 0; n < daemon_antipacket_queue_->GetNPackets() && !real_send_boolean; n++) {
                        p_pkt = daemon_antipacket_queue_->Dequeue()->GetPacket();
                        p_pkt->RemoveHeader(bp_header);
                        NS_LOG_INFO("p-" << n << bp_header);
                        bool anti_good_check = SprayGoodDetail(bp_header, 0);
                        bool anti_lazy_transmit_check = Simulator::Now().GetSeconds() - bp_header.get_hop_time_stamp().GetSeconds() > NS3DTNBIT_HELLO_BUNDLE_INTERVAL_TIME;
                        if (anti_good_check && anti_lazy_transmit_check) {
                            real_send_boolean = FindTheNeighborThisBPHeaderTo(bp_header, decision_neighbor, check_state);
                        } else {
                            NS_LOG_INFO("pkt not available, anti_good_check, anti_lazy_transmit_check=" << anti_good_check << anti_lazy_transmit_check);
                        }
                        p_pkt->AddHeader(bp_header);
                        Ptr<Packet> p_pkt_copy = p_pkt->Copy();
                        daemon_antipacket_queue_->Enqueue(Packet2Queueit(p_pkt_copy));
                    }
                } else if (check_state == CheckState::State_1){
                    // go though daemon_bundle_queue_ to find whether real_send_boolean should be set true
                    NS_LOG_DEBUG(LogPrefixMacro << "we have NPackets = " << daemon_bundle_queue_->GetNPackets());
                    for (int n = 0; n < daemon_bundle_queue_->GetNPackets() && !real_send_boolean; n++) {
                        p_pkt = daemon_bundle_queue_->Dequeue()->GetPacket();
                        p_pkt->RemoveHeader(bp_header);
                        NS_LOG_INFO("check pkt-" << n << bp_header);
                        if (Simulator::Now().GetSeconds() - bp_header.get_src_time_stamp().GetSeconds() < NS3DTNBIT_HYPOTHETIC_BUNDLE_EXPIRED_TIME) {
                            bool bundle_check_good = (SprayGoodDetail(bp_header, 0));
                            bool bundle_lazy_transmit_check = Simulator::Now().GetSeconds() - bp_header.get_hop_time_stamp().GetSeconds() > NS3DTNBIT_HELLO_BUNDLE_INTERVAL_TIME;
                            bool bundle_dest_not_this_check = !bp_header.get_destination_ip().IsEqual(own_ip_);
                            if (bundle_check_good && bundle_dest_not_this_check && bundle_lazy_transmit_check) {
                                real_send_boolean = FindTheNeighborThisBPHeaderTo(bp_header, decision_neighbor, check_state);
                            } else {
                                NS_LOG_INFO("pkt not available, bundle_check_good, bundle_dest_not_this_check, bundle_lazy_transmit_check=" << bundle_check_good << bundle_dest_not_this_check << bundle_lazy_transmit_check);
                            }
                        } else {
                            NS_LOG_ERROR(LogPrefixMacro << "Error:expired pkt, should be removed before reach here"); std::abort();
                        }
                        p_pkt->AddHeader(bp_header);
                        Ptr<Packet> p_pkt_copy = p_pkt->Copy();
                        daemon_bundle_queue_->Enqueue(Packet2Queueit(p_pkt_copy));
                    }
                }
            } else {
                check_state = CheckState::State_0;
            }
            if (real_send_boolean) {
                // init transmission session info, and invoke "totransmit'
                // refactory this transmit related code into one nested-class
                int j = decision_neighbor;
                DaemonBundleHeaderInfo tmp_header_info = {
                    neighbor_info_vec_[j].info_address_,
                    bp_header.get_retransmission_count(),
                    bp_header.get_source_seqno()
                };
                DaemonTransmissionInfo tmp_transmission_info = {
                    p_pkt->GetSize(),
                    0,
                    Simulator::Now().GetSeconds(),
                    Simulator::Now().GetSeconds(),
                    0
                };
                bool transmist_session_already = false;
                for (auto ele : transmit_assister_.daemon_transmission_bh_info_vec_) {
                    if (ele == tmp_header_info) {
                        transmist_session_already = true;
                    }
                }
                if (transmist_session_already) {
                    BPHeader bbh;
                    p_pkt->RemoveHeader(bbh);
                    NS_LOG_WARN(LogPrefixMacro << "WARN:transmit-session already exist, head = " << bbh);
                    p_pkt->AddHeader(bbh);
                } else {
                    NS_LOG_INFO(LogPrefixMacro << "transmission session Enqueue");
                    transmit_assister_.daemon_transmission_bh_info_vec_.push_back(tmp_header_info);
                    transmit_assister_.daemon_transmission_info_vec_.push_back(tmp_transmission_info);
                    transmit_assister_.daemon_retransmission_packet_buffer_vec_.push_back(p_pkt->Copy());
                }
                ToTransmit(tmp_header_info, false);
            }
            CheckBufferSwitchStateDetail(real_send_boolean, check_state);
            NS_LOG_LOGIC(LogPrefixMacro << "Out of " << "CheckBuffer()");
        }

        /*
         * Aim : switch state of app
         * */
        void DtnApp::CheckBufferSwitchStateDetail(bool real_send_boolean, DtnApp::CheckState check_state) {
            // refine switch schedule
            switch (check_state) {
                // switch check_state and reschedule
                case CheckState::State_0 : {
                                               if (real_send_boolean) {
                                                   NS_LOG_ERROR(LogPrefixMacro << "error, check state can't be state_0 when real_send_boolean is set true");
                                                   std::abort();
                                               } else {
                                                   Simulator::Schedule(Seconds(NS3DTNBIT_BUFFER_CHECK_INTERVAL), &DtnApp::CheckBuffer, this, CheckState::State_2);
                                               }
                                               break;
                                           }
                case CheckState::State_1 : {
                                               if (real_send_boolean) {
                                                   Simulator::Schedule(Seconds(NS3DTNBIT_BUFFER_CHECK_INTERVAL), &DtnApp::CheckBuffer, this, CheckState::State_1);
                                               } else {
                                                   Simulator::Schedule(Seconds(NS3DTNBIT_BUFFER_CHECK_INTERVAL), &DtnApp::CheckBuffer, this, CheckState::State_2);
                                               }
                                               break;
                                           }
                case CheckState::State_2 : {
                                               if (real_send_boolean) {
                                                   Simulator::Schedule(Seconds(NS3DTNBIT_BUFFER_CHECK_INTERVAL), &DtnApp::CheckBuffer, this, CheckState::State_1);
                                               } else {
                                                   Simulator::Schedule(Seconds(NS3DTNBIT_BUFFER_CHECK_INTERVAL), &DtnApp::CheckBuffer, this, CheckState::State_1);
                                               }
                                               break;
                                           }
                default : {break;}
            }
        }

        /* refine
         * Aim :
         * This is not a routing method, just find all avilables.
         * Detail :
         * handle normal bundle and antipacket send decision
         * bundle_sent if logic included by transmit_session_already 
         */
        vector<int> DtnApp::BPHeaderBasedSendDecisionDetail(BPHeader& ref_bp_header, enum CheckState check_state) {
            NS_LOG_INFO(LogPrefixMacro << "enter BPHeaderBasedSendDecisionDetail()");
            vector<int> result;
            NS_LOG_INFO(LogPrefixMacro << "neighbor_info_vec_.size() =" << neighbor_info_vec_.size());
            for (int j = 0; j < neighbor_info_vec_.size(); j++) {
                //bool nei_last_seen_bool = Simulator::Now().GetSeconds() - neighbor_info_vec_[j].info_last_seen_time_ < NS3DTNBIT_HELLO_BUNDLE_INTERVAL_TIME;
                bool nei_last_seen_bool = neighbor_info_vec_[j].IsLastSeen();
                bool nei_have_space = neighbor_info_vec_[j].info_daemon_baq_available_bytes_ > ref_bp_header.get_payload_size() + ref_bp_header.GetSerializedSize();
                bool nei_is_not_source = !neighbor_info_vec_[j].info_address_.GetIpv4().IsEqual(ref_bp_header.get_source_ip());
                // 'transmit_session_already' is true, means that totransmit() has been called and shouldn't init a now one.
                // 'neighbor_has_bundle is false' maens that neighbor do not carry this bundle
                // 'bundle_sent is false' means that you haven't send this bundle to this neighbor
                bool transmit_session_already = false, neighbor_has_bundle = false, bundle_sent = false, pre = nei_last_seen_bool && nei_have_space && nei_is_not_source;
                {
                    // set transmit_session_already, neighbor_has_bundle, bundle_sent
                    for (int i = 0; i < transmit_assister_.daemon_transmission_bh_info_vec_.size(); i++) {
                        auto no1 = transmit_assister_.daemon_transmission_bh_info_vec_[i].info_source_seqno_;
                        auto no2 = ref_bp_header.get_source_seqno();
                        auto ip1 = transmit_assister_.daemon_transmission_bh_info_vec_[i].info_transmit_addr_.GetIpv4();
                        auto ip2 = neighbor_info_vec_[j].info_address_.GetIpv4();
                        if (no1 == no2 && ip1.IsEqual(ip2)) {
                            transmit_session_already = true;
                            break;
                        }
                    }
                    for (int m = 0; !transmit_session_already && m < neighbor_info_vec_[j].info_baq_seqno_vec_.size(); m++) {
                        if (neighbor_info_vec_[j].info_baq_seqno_vec_[m] == ref_bp_header.get_source_seqno()) {
                            neighbor_has_bundle = true;
                            break;
                        }
                    }
                    if (ref_bp_header.get_bundle_type() == BundleType::BundlePacket) {
                        for (int m = 0; (!transmit_session_already) && (!neighbor_has_bundle) && (m < neighbor_info_vec_[j].info_sent_bp_seqno_vec_.size()); m++) {
                            if (neighbor_info_vec_[j].info_sent_bp_seqno_vec_[m] == ref_bp_header.get_source_seqno()) { bundle_sent = true; break; }
                        }
                    } else if (ref_bp_header.get_bundle_type() == BundleType::AntiPacket) { 
                        for (int m = 0; (!transmit_session_already) && !neighbor_has_bundle && m < neighbor_info_vec_[j].info_sent_ap_seqno_vec_.size(); m++) {
                            if (neighbor_info_vec_[j].info_sent_ap_seqno_vec_[m] == ref_bp_header.get_source_seqno()) { bundle_sent = true; break; }
                        }
                    }
                }

                if (pre && (!transmit_session_already) && (!neighbor_has_bundle) && (!bundle_sent)) {
                    NS_LOG_DEBUG(LogPrefixMacro << "neighbor, node id=" << Ipv42NodeNo(neighbor_info_vec_[j].info_address_.GetIpv4()) << "is good, pre, already,has,sent is :" << pre << " " << transmit_session_already << " " << neighbor_has_bundle << " " << bundle_sent);
                    result.push_back(j);
                } else {
                    NS_LOG_INFO(LogPrefixMacro << "neighbor, node id =" << Ipv42NodeNo(neighbor_info_vec_[j].info_address_.GetIpv4()) << "isn't available" << ",pre, already,has,sent is :" << pre << " " << transmit_session_already << " " << neighbor_has_bundle << " " << bundle_sent);
                }
            }
            return result;
            NS_LOG_LOGIC(LogPrefixMacro << "Out of " << "CheckBuffer()");
        }

        /* refine 
         * TODO refactory this after implement transmit related class transmit assister
         * roll back : if 'the sent pkt' has been received and info-ed by hello, then,
         * update the 'state of pkt' from 'info_sent' to 'neighbor have' semantically.
         * this function is the 'from'
         */
        void DtnApp::UpdateNeighborInfoDetail(int which_info, int which_neighbor, int which_pkt_index) {
            NS_LOG_INFO(LogPrefixMacro << "enter UpdateNeighborInfoDetail()");
            NS_LOG_WARN(LogPrefixMacro << "WARN:receive hello ROLL BACK, this may make this node send duplicate packet if anti-pkt didn't affect bundle queue");
            switch (which_info) {
                case 0 : {
                             // info_baq_seqno_vec_
                             break;
                         }
                case 1 : {
                             // info_sent_bp_seqno_vec_
                             neighbor_info_vec_[which_neighbor].info_sent_bp_seqno_vec_.erase(neighbor_info_vec_[which_neighbor].info_sent_bp_seqno_vec_.begin() + which_pkt_index);
                             break;
                         }
                case 2 : {
                             // info_sent_ap_seqno_vec_
                             neighbor_info_vec_[which_neighbor].info_sent_ap_seqno_vec_.erase(neighbor_info_vec_[which_neighbor].info_sent_ap_seqno_vec_.begin() + which_pkt_index);
                             break;
                         }
                case 3 : {
                             // info_sent_ap_time_vec_
                             neighbor_info_vec_[which_neighbor].info_sent_ap_time_vec_.erase(neighbor_info_vec_[which_neighbor].info_sent_ap_time_vec_.begin() + which_pkt_index);
                             break;
                         }
                default : break;
            }
            NS_LOG_LOGIC(LogPrefixMacro << "Out of " << "UpdateNeighborInfoDetail()");
        }

        /* refine 
         * this func would be invoked only in ReceiveBundle()
         * it would find the pkt in daemon_bundle_queue_, then dequeue it 
         * and update neighbor_sent_bp_seqno_vec_ & transmit_assister_.daemon_transmission_bh_info_vec_
         * it would find the pkt in daemon_antipacket_queue_, then dequeue it
         */
        void DtnApp::RemoveBundleFromAntiDetail(Ptr<Packet> p_anti_pkt) {
            NS_LOG_INFO(LogPrefixMacro << "enter RemoveBundleFromAntiDetail()");
            BPHeader lhs_bp_header;
            p_anti_pkt->RemoveHeader(lhs_bp_header);
            int number = daemon_bundle_queue_->GetNPackets();
            std::stringstream lhs_ss;
            p_anti_pkt->CopyData(&lhs_ss, lhs_bp_header.get_payload_size());
            std::string source_ip, destination_ip;
            dtn_seqno_t lhs_seqno_value;
            lhs_ss >> source_ip >> destination_ip >> lhs_seqno_value;
            NS_LOG_DEBUG(LogPrefixMacro << "anti-pkt : anti- source ip=" << source_ip
                    << ";anti-destination_ip=" << destination_ip
                    << ";anti-seqno=" << lhs_seqno_value);
            Ipv4Address lhs_source_ip(source_ip.c_str()), lhs_destination_ip(destination_ip.c_str());
            while (number--) {
                Ptr<Packet> rhs_p_pkt = daemon_bundle_queue_->Dequeue()->GetPacket();
                BPHeader rhs_bp_header;
                rhs_p_pkt->RemoveHeader(rhs_bp_header);
                if (lhs_source_ip.IsEqual(rhs_bp_header.get_source_ip())
                        && lhs_destination_ip.IsEqual(rhs_bp_header.get_destination_ip())
                        && lhs_seqno_value == rhs_bp_header.get_source_seqno()) {
                    NS_LOG_DEBUG(LogPrefixMacro << "NOTE: ANTI remove bundle");
                    break;
                } else {
                    assert(rhs_bp_header.get_bundle_type() == BundleType::BundlePacket);
                    rhs_p_pkt->AddHeader(rhs_bp_header);
                    daemon_bundle_queue_->Enqueue(Packet2Queueit(rhs_p_pkt));//Enqueue(rhs_p_pkt);
                }
            }
            p_anti_pkt->AddHeader(lhs_bp_header);
            NS_LOG_LOGIC(LogPrefixMacro << "Out of " << "RemoveBundleFromAntiDetail()");
        }

        /* refine 
         * check whether one packet is already in queue
         * true is duplicated
         */
        bool DtnApp::IsDuplicatedDetail(BPHeader& bp_header) {
            NS_LOG_INFO(LogPrefixMacro << "enter IsDuplicatedDetail()");
            if (bp_header.get_bundle_type() == BundleType::BundlePacket) {
                int number = daemon_bundle_queue_->GetNPackets();
                for (int i = 0; i < number; ++i) {
                    Ptr<Packet> lhs_p_pkt = daemon_bundle_queue_->Dequeue()->GetPacket();
                    BPHeader lhs_bp_header;
                    lhs_p_pkt->RemoveHeader(lhs_bp_header);
                    if (lhs_bp_header.get_source_ip().IsEqual(bp_header.get_source_ip())
                            && lhs_bp_header.get_source_seqno() == bp_header.get_source_seqno()) {
                        lhs_p_pkt->AddHeader(lhs_bp_header);
                        daemon_bundle_queue_->Enqueue(Packet2Queueit(lhs_p_pkt));//Enqueue(lhs_p_pkt);
                        return true;
                    }
                    lhs_p_pkt->AddHeader(lhs_bp_header);
                    daemon_bundle_queue_->Enqueue(Packet2Queueit(lhs_p_pkt));//Enqueue(lhs_p_pkt);
                }
            } else if (bp_header.get_bundle_type() == BundleType::AntiPacket) {
                int number = daemon_antipacket_queue_->GetNPackets();
                for (int i = 0; i < number; ++i) {
                    Ptr<Packet> lhs_p_pkt = daemon_antipacket_queue_->Dequeue()->GetPacket();
                    BPHeader lhs_bp_header;
                    lhs_p_pkt->RemoveHeader(lhs_bp_header);
                    if (lhs_bp_header.get_source_ip().IsEqual(bp_header.get_source_ip())
                            && lhs_bp_header.get_source_seqno() == bp_header.get_source_seqno()) {
                        lhs_p_pkt->AddHeader(lhs_bp_header);
                        daemon_antipacket_queue_->Enqueue(Packet2Queueit(lhs_p_pkt));//Enqueue(lhs_p_pkt);
                        return true;
                    }
                    lhs_p_pkt->AddHeader(lhs_bp_header);
                    daemon_antipacket_queue_->Enqueue(Packet2Queueit(lhs_p_pkt));//Enqueue(lhs_p_pkt);
                }
            } else {
                NS_LOG_ERROR(LogPrefixMacro << "ERROR: can't be, must wrong");
            }
            return false;
            NS_LOG_LOGIC(LogPrefixMacro << "Out of " << "IsDuplicatedDetail()");
        }

        /*
         * */
        void DtnApp::Report(std::ostream& os) {
            os << "node-" << node_->GetId() << "\nbundle_send_count_" << "=" << bundle_send_count_
                << "\nanti_send_count_=" << anti_send_count_ << endl;
        }

        /* refine 
         * ack, anti, bundle would go through this function call
         * should handle more condition
         * this call handles only link layer sending, so it's called trans_addr not dst_addr
         * LOG some
         */
        bool DtnApp::SocketSendDetail(Ptr<Packet> p_pkt, uint32_t flags, InetSocketAddress trans_addr) {
            NS_LOG_LOGIC(LogPrefixMacro << "enter SocketSendDetail()");
            {
                // LOG
                BPHeader bp_header;
                p_pkt->RemoveHeader(bp_header);
                NS_LOG_LOGIC(LogPrefixMacro << "SocketSendDetail() : bpheader :" << bp_header);
                SprayGoodDetail(bp_header, 1);
                if (p_pkt->GetSize() == 0) {
                    NS_LOG_ERROR(LogPrefixMacro << "ERROR:pkt size =" << p_pkt->GetSize() << ";bundle type=" << bp_header.get_bundle_type());
                    std::abort();
                } else {
                    if (bp_header.get_bundle_type() == BundleType::BundlePacket) {
                        bundle_send_count_++;
                    } else if (bp_header.get_bundle_type() == BundleType::AntiPacket) {
                        anti_send_count_++;
                    } else if (bp_header.get_bundle_type() == BundleType::TransmissionAck) {
                        ack_send_count_++;
                    } else {
                        NS_LOG_ERROR(LogPrefixMacro << "ERROR : bundletype=" << bp_header.get_bundle_type());
                        std::abort();
                    }
                }
                p_pkt->AddHeader(bp_header);
            }
            if (daemon_socket_handle_) {
                int result = daemon_socket_handle_->SendTo(p_pkt, flags, trans_addr);
                return result != -1 ? true : false;
            } else {
                NS_LOG_ERROR("socket_handle not initialized");
                std::abort();
            }
            NS_LOG_LOGIC(LogPrefixMacro << "Out of " << "SocketSendDetail()");
        }

        /* refine
        */
        void DtnApp::CreateSocketDetail() {
            NS_LOG_DEBUG(LogPrefixMacro << "enter CreateSocketDetail()");
            daemon_socket_handle_ = Socket::CreateSocket(node_, TypeId::LookupByName("ns3::UdpSocketFactory"));
            Ptr<Ipv4> ipv4 = node_->GetObject<Ipv4>();
            Ipv4Address ipip = (ipv4->GetAddress(1, 0)).GetLocal();
            NS_LOG_DEBUG("create bundle send socket,ip=" << ipip << ";port=" << NS3DTNBIT_PORT_NUMBER);
            InetSocketAddress local = InetSocketAddress(ipip, NS3DTNBIT_PORT_NUMBER);
            daemon_socket_handle_->Bind(local);
            NS_LOG_LOGIC(LogPrefixMacro << "Out of " << "CreateSocketDetail()");
        }

        /* NOTE : This function is not used
         * Aim :
         * This function is used to balanced the pkt priority in the queue
         * Detail :
         * using daemon_reorder_buffer_queue_
         * Important! such a ugly sort algorithm, refine it
         */
        void DtnApp::PeriodReorderDaemonBundleQueueDetail() {
            NS_LOG_INFO(LogPrefixMacro << "enter PeriodReorderDaemonBundleQueueDetail()");
            int bundle_number = daemon_bundle_queue_->GetNPackets();
            Ptr<Packet> p_pkt;
            BPHeader bp_header;
            for (int i = 0; i < bundle_number; ++i) {
                p_pkt = daemon_bundle_queue_->Dequeue()->GetPacket();
                daemon_reorder_buffer_queue_->Enqueue(Packet2Queueit(p_pkt));//Enqueue(p_pkt);
            }
            // reorder 
            for (int m = 0; m < bundle_number; m++) {
                int min_trans_count = 1000;
                dtn_seqno_t min_trans_seqno;
                for (int n = 0; n < daemon_reorder_buffer_queue_->GetNPackets(); n++) {
                    int trans_count = 0, index = 0;
                    p_pkt = daemon_reorder_buffer_queue_->Dequeue()->GetPacket();
                    p_pkt->RemoveHeader(bp_header);
                    // find trans_min_count
                    for (; index < transmit_assister_.daemon_transmission_bh_info_vec_.size(); index++) {
                        if (transmit_assister_.daemon_transmission_bh_info_vec_[index].info_source_seqno_ == bp_header.get_source_seqno()) {
                            trans_count++;
                        }
                    }
                    if (trans_count < min_trans_count) {
                        min_trans_count = min_trans_count < trans_count ? min_trans_count : trans_count;
                        min_trans_seqno = bp_header.get_source_seqno();
                    }
                    p_pkt->AddHeader(bp_header);
                    daemon_reorder_buffer_queue_->Enqueue(Packet2Queueit(p_pkt));//Enqueue(p_pkt);
                    // put the trans_min_count one into daemon_bundle_queue_
                    bool found = false;
                    while (!found) {
                        p_pkt = daemon_reorder_buffer_queue_->Dequeue()->GetPacket();
                        BPHeader tmp_bp_header;
                        p_pkt->RemoveHeader(tmp_bp_header);
                        if (tmp_bp_header.get_source_seqno() == min_trans_seqno) {
                            found = true;
                            p_pkt->AddHeader(tmp_bp_header);
                            daemon_bundle_queue_->Enqueue(Packet2Queueit(p_pkt));//Enqueue(p_pkt);
                        } else {
                            p_pkt->AddHeader(tmp_bp_header);
                            daemon_reorder_buffer_queue_->Enqueue(Packet2Queueit(p_pkt));//Enqueue(p_pkt);
                        }
                    }
                }
            }
            NS_LOG_LOGIC(LogPrefixMacro << "Out of " << "PeriodReorderDaemonBundleQueueDetail()");
        }

        /* refine
         * create and fill up then send
         */
        void DtnApp::CreateHelloBundleAndSendDetail(string msg_str, Ptr<Socket> broad_cast_skt) {
            NS_LOG_LOGIC(LogPrefixMacro << "enter CreateHelloBundleAndSendDetail()" << ";hello_bpheader_payload=" << msg_str.size());
            Ptr<Packet> p_pkt = Create<Packet>(msg_str.c_str(), msg_str.size());
            BPHeader bp_header;
            {
                // fill up bp_header
                SemiFillBPHeaderDetail(&bp_header);
                bp_header.set_bundle_type(BundleType::HelloPacket);
                bp_header.set_source_ip(Ipv4Address("255.255.255.255"));
                bp_header.set_source_seqno(p_pkt->GetUid());
                bp_header.set_payload_size(msg_str.size());
                bp_header.set_offset(msg_str.size());
            }
            p_pkt->AddHeader(bp_header);
            //p_pkt->AddPacketTag(QosTag(6));
            broad_cast_skt->Send(p_pkt);
            NS_LOG_LOGIC(LogPrefixMacro << "Out of " << "CreateHelloBundleAndSendDetail()");
        }

        /* refine 
        */
        void DtnApp::RemoveExpiredBAQDetail() {
            NS_LOG_INFO(LogPrefixMacro << "enter RemoveExpiredBAQDetail()");
            uint32_t pkt_number = 0, n = 0;
            // remove expired bundle queue packets
            pkt_number = daemon_bundle_queue_->GetNPackets();
            for (int i = 0; i < pkt_number; ++i) {
                Ptr<Packet> p_pkt = daemon_bundle_queue_->Dequeue()->GetPacket();
                BPHeader bp_header;
                p_pkt->RemoveHeader(bp_header);
                if ((Simulator::Now().GetSeconds() - bp_header.get_src_time_stamp().GetSeconds()) < NS3DTNBIT_HYPOTHETIC_BUNDLE_EXPIRED_TIME) {
                    //if not expired
                    p_pkt->AddHeader(bp_header);
                    daemon_bundle_queue_->Enqueue(Packet2Queueit(p_pkt));//Enqueue(p_pkt);
                } else {
                    for (int j = 0; j < neighbor_info_vec_.size(); j++) {
                        bool found_expired = false;
                        int m = 0;
                        for (; m < neighbor_info_vec_[j].info_sent_bp_seqno_vec_.size(); m++) {
                            if (bp_header.get_source_seqno() == neighbor_info_vec_[j].info_sent_bp_seqno_vec_[m]) { found_expired = true; break;} 
                        }
                        if (found_expired) {
                            NS_LOG_WARN(LogPrefixMacro << "WARN:expired, may happen");
                            UpdateNeighborInfoDetail(1, j, m);
                        }
                    }
                }
            }
            // remove expired antipacket queue packets
            pkt_number = daemon_antipacket_queue_->GetNPackets();
            for (int i = 0; i < pkt_number; ++i) {
                Ptr<Packet> p_pkt = daemon_antipacket_queue_->Dequeue()->GetPacket();
                BPHeader bp_header;
                p_pkt->RemoveHeader(bp_header);
                // if not expired
                if ((Simulator::Now().GetSeconds() - bp_header.get_src_time_stamp().GetSeconds() < NS3DTNBIT_ANTIPACKET_EXPIRED_TIME)) {
                    p_pkt->AddHeader(bp_header);
                    daemon_antipacket_queue_->Enqueue(Packet2Queueit(p_pkt));//Enqueue(p_pkt);
                } else {
                    for (int j = 0; j < neighbor_info_vec_.size(); j++) {
                        bool found_expired = false;
                        int k = 0;
                        for (; k < neighbor_info_vec_[j].info_sent_ap_seqno_vec_.size(); k++) {
                            if (bp_header.get_source_seqno() == neighbor_info_vec_[j].info_sent_ap_seqno_vec_[k]) { found_expired = true; break; }
                        }
                        if (found_expired) {
                            NS_LOG_WARN(LogPrefixMacro << "WARN:expired, may happen");
                            UpdateNeighborInfoDetail(2, j, k);
                            UpdateNeighborInfoDetail(3, j, k);
                        }
                    }
                }
            }
            NS_LOG_LOGIC(LogPrefixMacro << "Out of " << "RemoveExpiredBAQDetail()");
        }

        /* refine
         * you should fill following fields your self * 'bundle type' * 'dstip' * 'seqno' * 'payload size'
         */
        void DtnApp::SemiFillBPHeaderDetail(BPHeader* p_bp_header) {
            NS_LOG_LOGIC(LogPrefixMacro << "enter SemiFillBPHeaderDetail()");
            char srcstring[1024] = "";
            sprintf(srcstring, "10.0.0.%d", (node_->GetId() + 1));
            p_bp_header->set_source_ip(srcstring);
            p_bp_header->set_hop_count(0);
            if (routing_assister_.get_rm() == RoutingMethod::TimeExpanded) {
                // because TimeExpanded routing is accurate, so, every node need only one transmition for every bundle-pkt
                p_bp_header->set_spray(1);
            } else if (routing_assister_.get_rm() == RoutingMethod::SprayAndWait) {
                p_bp_header->set_spray(4);
            } else if (routing_assister_.get_rm() == RoutingMethod::Other) {
                p_bp_header->set_spray(3);
            } else if (routing_assister_.get_rm() == RoutingMethod::CGR) {
                p_bp_header->set_spray(1);
            } else {
                NS_LOG_ERROR(LogPrefixMacro << "ERROR: can't find routing method!");
                std::abort();
            }
            p_bp_header->set_retransmission_count(0);
            p_bp_header->set_src_time_stamp(Simulator::Now());
            p_bp_header->set_hop_time_stamp(Simulator::Now());
            p_bp_header->set_hop_ip(NodeNo2Ipv4(node_->GetId()));
            NS_LOG_LOGIC(LogPrefixMacro << "Out of " << "SemiFillBPHeaderDetail()");
        }

        /* refine
         * Aim: handy wrpper
         */
        void DtnApp::ScheduleTx (Time tNext, uint32_t dstnode, uint32_t payload_size) {
            NS_LOG_DEBUG(LogPrefixMacro << "enter ScheduleTx(), time-" << tNext << ",size=" << payload_size << ", to node-" << dstnode);
            Simulator::Schedule(tNext, &DtnApp::ToSendBundle, this, dstnode, payload_size);
        }

        int DtnApp::DtnAppRoutingAssister::RouteIt(int s, int d) {
            return p_rm_in_->DoRoute(s, d);
        }

        RoutingMethodInterface::RoutingMethodInterface(DtnApp& dp) : out_app_(dp) {}
        RoutingMethodInterface::~RoutingMethodInterface() {}
        void RoutingMethodInterface::GetInfo(int destination_id, int from_id, std::vector<int> vec_of_current_neighbor, int own_id, dtn_time_t expired_time, int bundle_size, int networkconfigurationflag, map<int, vector<int>> id2cur_exclude_vec_of_id) {}
        Adob RoutingMethodInterface::get_adob() { return out_app_.routing_assister_.vec_[0]; }

    } /* ns3dtnbit */ 
} /*ns3*/

namespace ns3 {
    namespace ns3dtnbit {
        void Adob::AdobDo_01(std::map<int, vector<vector<int>>> t_2_adjacent_array, int node_number) {
            // boost code
            using namespace boost;
            node_number_ = node_number;
            int t = -1;
            for (auto t2 : t_2_adjacent_array) {
                t++;
                auto time_index = get<0>(t2);
                auto t3 = get<1>(t2);
                Graph my_g;
                vector<VeDe> vec_vertex_des;
                unordered_map<int, VeDe> tmp_m;
                // add node & node NameProperties
                for (int i = 0; i < node_number; i++) {
                    std::stringstream ss;
                    ss << "node-" << i;
                    auto tmp_vd = add_vertex(VertexProperties(ss.str()), my_g);
                    vec_vertex_des.push_back(tmp_vd);
                    tmp_m[i] = tmp_vd;
                }
                //auto vec_edge_des = std::vector<vector<EdDe>>(node_number_, std::vector<EdDe>(node_number_, EdDe()));
                for (int i = 0; i < node_number; ++i) {
                    for (int j = i; j < node_number; ++j) {
                        if (i == j) {continue;}
                        add_edge(vec_vertex_des[i], vec_vertex_des[j], EdgeProperties(t3[i][j], 1), my_g);
                        add_edge(vec_vertex_des[j], vec_vertex_des[i], EdgeProperties(t3[i][j], 1), my_g);
                    }
                }
                assert(boost::num_edges(my_g)>0);
                assert(boost::num_vertices(my_g)>0);
                // load it
                t_vec_.push_back(time_index);
                g_vec_.push_back(my_g);
                g_vede_m_.push_back(tmp_m);
            }
        }

        void Adob::AdobDo_02(int node_number, int teg_layer_n, int max_range) {
            max_range_ = max_range;
            // add all node to it node-n-t-i
            using namespace boost;
            for (int n = 0; n < node_number; n++) {
                for (int t = 0; t < teg_layer_n; t++) {
                    stringstream ss;
                    ss << "node-" << n << "-" << t;
                    VeDe tmp_d = add_vertex(VertexProperties(ss.str()), teg_);
                    name2vd_map[ss.str()] = tmp_d;
                }
            }
            // add temporal link
            // g_vec_ is vector of static graph
            const int hypothetic_distance_of_temporal_link = max_range / 20;    // privent that message keep on one node all the time
            for (int t = 0; t < teg_layer_n - 1; t++) {
                for (int n = 0; n < node_number; n++) {
                    stringstream ss;
                    ss << "node-" << n << "-" << t;
                    auto name_0 = ss.str();
                    ss.str("");
                    ss << "node-" << n << "-" << t + 1;
                    auto name_1 = ss.str();
                    auto vd_0 = name2vd_map[name_0];
                    auto vd_1 = name2vd_map[name_1];
                    auto tmp_ed = add_edge(vd_0, vd_1, EdgeProperties(hypothetic_distance_of_temporal_link, 0), teg_);
                }
            }
            // for each layer add transmit link if distance_ of link 'a' of static upper layer graph is under max_range 
            // and the distance of link 'b' of static lower layer graph is also under max_range
            assert(g_vec_.size() >= teg_layer_n);
            // assume that g_vede_m_ == teg_layer_n 
            for (int t = 0; t < teg_layer_n - 1; t++) {
                Graph& tmp_g = g_vec_[t];
                Graph& tmp_g_other = g_vec_[t + 1];
                for (int i = 0; i < node_number; i++) {
                    for (int j = i; j < node_number; j++) {
                        if (i == j) {continue;}
                        auto i_d = g_vede_m_[t][i];
                        auto j_d = g_vede_m_[t][j];
                        auto e_p = edge(i_d, j_d, tmp_g);
                        auto i_d_other = g_vede_m_[t + 1][i];
                        auto j_d_other = g_vede_m_[t + 1][j];
                        auto e_p_other = edge(i_d_other, j_d_other, tmp_g_other);
                        if (e_p.second && e_p_other.second) {
                            auto ed = e_p.first;
                            auto ed_other = e_p_other.first;
                            if (tmp_g[ed].distance_ < max_range && tmp_g_other[ed_other].distance_ < max_range) {
                                string ix_name = "node-" + to_string(i) + "-" + to_string(t);
                                string jx_name = "node-" + to_string(j) + "-" + to_string(t);
                                string iy_name = "node-" + to_string(i) + "-" + to_string(t + 1);
                                string jy_name = "node-" + to_string(j) + "-" + to_string(t + 1);
                                auto tmp_id_of_g = name2vd_map[ix_name];
                                auto tmp_jd_of_g = name2vd_map[jx_name];
                                auto tmp_id_of_g_other = name2vd_map[iy_name];
                                auto tmp_jd_of_g_other = name2vd_map[jy_name];

                                auto tmp_edge_of_g = add_edge(tmp_id_of_g, tmp_jd_of_g_other, EdgeProperties(
                                            (tmp_g[ed].distance_ / 2) + (tmp_g_other[ed_other].distance_ / 2), 1), teg_);
                                auto tmp_edge_of_g_other = add_edge(tmp_jd_of_g, tmp_id_of_g_other, EdgeProperties(
                                            (tmp_g[ed].distance_ / 2) + (tmp_g_other[ed_other].distance_ / 2), 1), teg_);
                            }
                        } else {
                            std::cerr << "Error: can't acess edge" << " : line " << __LINE__ 
                                << " t=" << t << " i =" << i << " j =" << j 
                                << "size of g_vec_ =" << g_vec_.size()
                                << "size of g_vede_m_= " << g_vede_m_.size()
                                << std::endl;
                            std::abort();
                        }
                    }
                }
            }
        }

        void Adob::AdobDo_03() {
            assert(get_teg_size() > get_g_vec_size() * get_node_number());
            string teg_viz_filename = root_path + "/box/dtn_simulation_result/teg_viz.txt";
            // get round time 
            int rounded_time = Simulator::Now().GetSeconds();
            int t_max = g_vec_.size();
            //DelayMap delay_map_;

            std::cout << "NOTE:in AdobDo_03, before initialize delay_map_" << std::endl;
            // Initialize delay_map_
            const int c = hypo_c; // assume every data transmit would cost one unit of time
            for (int t = t_max - 2; t >= 0; t--) {
                for (int i = 0; i < get_node_number(); i++) {
                    for (int j = 0; j < get_node_number(); j++) {
                        string v_i_t_name = "node-" + to_string(i) + "-" + to_string(t);
                        string v_j_t_plus_c_name = "node-" + to_string(j) + "-" + to_string(t + c);
                        auto ep = edge(name2vd_map[v_i_t_name], name2vd_map[v_j_t_plus_c_name], teg_);
                        if (ep.second) {
                            int edge_delay_color = teg_[ep.first].message_color_;
                            DelayIndex tmp_dl = make_tuple(i, j, t, edge_delay_color);
                            delay_map_[tmp_dl] = edge_delay_color;
                        } else {
                            DelayIndex tmp_dl = make_tuple(i, j, t + 1, c);
                            DelayIndex tmp_dl_x = make_tuple(i, j, t, c);
                            auto found = delay_map_.find(tmp_dl);
                            if (found != delay_map_.end()) {
                                if (delay_map_[tmp_dl] < NS3DTNBIT_HYPOTHETIC_INFINITE_DELAY) {
                                    delay_map_[tmp_dl_x] = delay_map_[tmp_dl] + 1;
                                } else {
                                    delay_map_[tmp_dl_x] = NS3DTNBIT_HYPOTHETIC_INFINITE_DELAY;
                                }
                            } else {
                                delay_map_[tmp_dl_x] = NS3DTNBIT_HYPOTHETIC_INFINITE_DELAY;
                            }
                        }
                    }
                }
            }
            std::cout << "NOTE:in AdobDo_03, before shortest delay path" << std::endl;
            // shortest delay path
            for (int k = 0; k < get_node_number(); k++) {
                for (int i = 0; i < get_node_number(); i++) {
                    for (int j = 0; j < get_node_number(); j++) {
                        for (int t = 0; t < t_max - 2; t++) { // it's not meaning to set delay_map_ for t_max - 1, this is one difference from that paper
                            DelayIndex di_cur = make_tuple(i, j, t, c), di_to_k = make_tuple(i, k, t, c);
                            int delay_cur = delay_map_[di_cur];
                            int delay_to_k = delay_map_[di_to_k];
                            DelayIndex di_from_k = make_tuple(k, j, t + delay_to_k, c);
                            auto found = delay_map_.find(di_from_k);
                            if (found != delay_map_.end()) {
                                int delay_from_k = delay_map_[di_from_k];
                                int sum = delay_to_k + delay_from_k;
                                if (sum < delay_cur) {
                                    delay_map_[di_cur] = sum;
                                    auto tmp_tuple = make_tuple(i, j, t);
                                    teg_routing_table_[tmp_tuple] = k;
                                }
                            } else {
                                // maybe t + delay_to_k > T_max
                            }
                        }
                    }
                }
            }
            std::cout << "NOTE: write viz for teg" << std::endl;
            ofstream dot(teg_viz_filename);
            using EdgeProperties = ns3::ns3dtnbit::Adob::EdgeProperties;
            using VertexProperties = ns3::ns3dtnbit::Adob::VertexProperties;
            boost::write_graphviz(dot, teg_, 
                    boost::make_label_writer(boost::get(&VertexProperties::name_, teg_)),
                    boost::make_edge_writer(boost::get(&EdgeProperties::distance_, teg_), boost::get(&EdgeProperties::message_color_, teg_)));
            std::cout << "NOTE:in AdobDo_03, after shortest delay path" << std::endl;
        }

        /*
         * */ 
        void Adob::AdobDo_04() {
            // init node_id2cgr_xmit_vec_map_ TODO
            cout << "DEBUG_CGR " << "in AdobDo_04" << endl;
            int node_number = get_node_number();
            int time_max = get_g_vec_size();
            for (int d = 0; d < node_number; ++d) {
                vector<CgrXmit> vec_of_xmits;
                for (int s = 0; s < node_number; ++s) {
                    if (d == s) {continue;}
                    bool no_more_xmits = false;
                    int time_cur = 0;
                    while (!no_more_xmits) {
                        dtn_time_t contact_start_time, contact_end_time;
                        int node_id_of_to = d, node_id_of_from = s;
                        double data_transmission_rate = 80000;
                        bool found_xmit = false;
                        {
                            // init -> 0, when find available one, -> 1, when available one become not available -> 2
                            int link_state = 0;
                            for (int t = time_cur; t < time_max && link_state != 2; ++t) {
                                if (t == time_max - 1) {no_more_xmits = true;}
                                auto g_t = g_vec_[t];
                                auto g_vd_m = g_vede_m_[t];
                                auto s_descriptor = g_vd_m[s];
                                auto d_descriptor = g_vd_m[d];
                                auto ep = edge(s_descriptor, d_descriptor, g_t);
                                if (ep.second) {
                                    int dist = g_t[ep.first].distance_;
                                    if (dist < max_range_ && link_state == 0) {
                                        contact_start_time = t;
                                        link_state = 1;
                                    } else if ((dist > max_range_ || t == (time_max - 1)) && link_state == 1) {
                                        contact_end_time = t;
                                        link_state = 2;
                                        found_xmit = true;
                                        time_cur = t;
                                        break;
                                    }
                                } else {
                                    cout << "Error:" << __LINE__ << " can't be, s =" << s << ";d =" << d << ";t=" << t << endl;
                                    std::abort();
                                }
                            }
                        }
                        if (found_xmit) {
                            CgrXmit cgr_xmit_obj = {
                                contact_start_time,
                                contact_end_time,
                                node_id_of_from,
                                node_id_of_to,
                                data_transmission_rate
                            };
                            vec_of_xmits.push_back(cgr_xmit_obj);
                        }
                    }
                }
                if (vec_of_xmits.size() == 0) { cout << "DEBUG_CGR_WARN : node have no xmit" << ";node-" << d << endl; }
                if (vec_of_xmits.size() > 0) { cout << "DEBUG_CGR_DEBUG : node have " << vec_of_xmits.size() << " xmits" << ";node-" << d << endl; }
                node_id2cgr_xmit_vec_map_[d] = vec_of_xmits;
            }
        }

        Adob::Adob() {}
        Adob::~Adob() {}

        int Adob::get_node_number() { return node_number_; }
        int Adob::get_teg_size() { return num_edges(teg_); }
        int Adob::get_g_vec_size() { return g_vec_.size(); }

        Adob::Graph Adob::get_graph_for_now() const {
            for (int i = t_vec_.size() - 1; i >= 0 ; i--) {
                if (Simulator::Now().GetSeconds() >= t_vec_[i]) {
                    auto g_re = g_vec_[i];
                    return g_re;
                }
            }
            std::cout << "Error:" <<__LINE__ << "can't be, Seconds =" << Simulator::Now().GetSeconds() << "t_vec_" << t_vec_[0] << std::endl;
            std::abort();
        }
    } /* ns3dtnbit */ 
} /* ns3  */ 
