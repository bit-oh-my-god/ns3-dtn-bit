/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3dtn-bit.h"
#define LogPrefixMacro LogPrefix()<<"line-"<<__LINE__<<"]"

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
            return result;
        }

        int Ipv42NodeNo(Ipv4Address ip) {
            auto ip_base = Ipv4Address("10.0.0.1");
            auto ip_base_v = ip_base.Get();
            auto ip_v = ip.Get();
            return ip_v - ip_base_v;
        }

        InetSocketAddress GetInAddrFromSocket(Ptr<Socket>& socket) {
            Address own_addr;
            socket->GetSockName(own_addr);
            return InetSocketAddress::ConvertFrom(own_addr);
        }

        InetSocketAddress Ip2Addr(Ipv4Address ip) {
            return InetSocketAddress(ip, NS3DTNBIT_PORT_NUMBER);
        }

        Ipv4Address Addr2Ip(InetSocketAddress addr) {
            return addr.GetIpv4();
        }
    } /* ns3dtnbit */ 
} /*ns3*/

namespace ns3 {
    namespace ns3dtnbit {
        /*
         * Aim : inherited method
         * */
        void DtnApp::StartApplication() {
            NS_LOG_INFO(LogPrefixMacro << "NOTE:Enter startapplication()");
            try {
                if (1 != node_->GetNDevices()) {
                    std::stringstream ss;
                    ss << "GetNDevices" << node_->GetNDevices();
                    throw std::runtime_error(ss.str());
                }
            } catch (const std::exception& r_e) {
                NS_LOG_INFO(LogPrefixMacro << "NOTE:" << r_e.what()); 
            }
            Ptr<WifiNetDevice> wifi_d = DynamicCast<WifiNetDevice> (node_->GetDevice(0));
            wifi_ph_p = wifi_d->GetPhy();
            CheckBuffer(CheckState::State_2);
            StateCheckDetail();
            std::srand(std::time(0)); // use current time as seed for random generator
            NS_LOG_LOGIC(LogPrefixMacro << "Out of startapplication()");
        }

        /* refine
         * Aim : inherited method
         */
        void DtnApp::SetUp(Ptr<Node> node) {
            node_ = node;
            own_ip_ = NodeNo2Ipv4(node_->GetId());

            daemon_antipacket_queue_ = CreateObject<DropTailQueue>();
            daemon_bundle_queue_ = CreateObject<DropTailQueue>();
            daemon_consume_bundle_queue_ = CreateObject<DropTailQueue>();

            Ptr<UniformRandomVariable> y = CreateObject<UniformRandomVariable>();

            daemon_antipacket_queue_->SetAttribute("MaxPackets", UintegerValue(1000));
            daemon_bundle_queue_->SetAttribute("MaxPackets", UintegerValue(1000));
            daemon_consume_bundle_queue_->SetAttribute("MaxPackets", UintegerValue(1000));
        }

    } /* ns3dtnbit */ 
} /*ns3*/

namespace ns3 {
    namespace ns3dtnbit {

        bool DtnApp::NeighborInfo::IsLastSeen() {
            return Simulator::Now().GetSeconds() - info_last_seen_time_ < NS3DTNBIT_HELLO_BUNDLE_INTERVAL_TIME;
        }

        string DtnApp::DtnAppNeighborKeeper::HelloContent() {
            std::stringstream msg;
            {
                // prepare 'msg' to write into hello bundle
                // tmp_msg_00 ---------- Available bytes

                // tmp_msg_00
                char tmp_msg_00[1024] = "";
                int32_t tmp_number = ((dtn_seqno_t)(out_app_.daemon_baq_pkts_max_ * NS3DTNBIT_HYPOTHETIC_CACHE_FACTOR) - (out_app_.daemon_bundle_queue_->GetNBytes() + out_app_.daemon_antipacket_queue_->GetNBytes()));
                if (tmp_number <= 0) {
                    sprintf(tmp_msg_00, "%d ", 0);
                } else {
                    sprintf(tmp_msg_00, "%d ", tmp_number);
                }
                msg << tmp_msg_00;
                if (msg.str().length() > NS3DTNBIT_HELLO_BUNDLE_SIZE_MAX) {
                    // ERROR LOG
                    NS_LOG_ERROR("too big hello");
                    std::abort(); 
                }
            }
            return msg.str();
        }

        void DtnApp::DtnAppNeighborKeeper::SendHelloDetail(Ptr<Socket> socket) {
            Ptr<Packet> p_pkt; 
            BPHeader bp_header;
            {
                // this call would use msg.str() as the 'hello content'
                string msg_str = HelloContent();
                Ptr<Packet> p_pkt = Create<Packet>(msg_str.c_str(), msg_str.size());
                BPHeader bp_header;
                {
                    // fill up bp_header
                    out_app_.SemiFillBPHeaderDetail(&bp_header);
                    bp_header.set_bundle_type(BundleType::HelloPacket);
                    bp_header.set_source_ip(Ipv4Address("255.255.255.255"));
                    bp_header.set_source_seqno(p_pkt->GetUid());
                    bp_header.set_payload_size(msg_str.size());
                    bp_header.set_offset(msg_str.size());
                }
                p_pkt->AddHeader(bp_header);
                socket->Send(p_pkt);
            }
        }

        pair<bool, Ipv4Address> DtnApp::DtnAppNeighborKeeper::HasNewNeighbor() {
            bool ret = false;
            Ipv4Address newip;
            for (auto nei : neighbor_info_map_) {
                auto nei_ip = get<0>(nei);
                if (get<1>(nei).IsLastSeen()) {
                    if (cur_neighbor_.count(nei_ip)) {

                    } else {
                        cur_neighbor_.insert(nei_ip);
                        ret = true;
                        newip = nei_ip;
                    }
                } else {
                    if (cur_neighbor_.count(nei_ip)) {
                        cur_neighbor_.erase(cur_neighbor_.find(nei_ip));
                    } else {

                    }
                }
            }
            return {ret, newip};
        }

        void DtnApp::DtnAppTransmitSessionAssister::ReceiveBundleDetail(Ptr<Socket>& socket) {
            NS_LOG_INFO(LogPrefixMacro << "ReceiveHelloDetail");
            int loop_count = 0;
            while (socket->GetRxAvailable() > 0) {
                NS_LOG_INFO(LogPrefixMacro << "ReceiveBundle(), loop_count =" << loop_count++
                        << ";socket->GetRxAvailable =" << socket->GetRxAvailable());
                Address from_addr;
                BPHeader bp_header;
                Ptr<Packet> p_pkt = socket->RecvFrom(from_addr);
                InetSocketAddress from_s_addr = InetSocketAddress::ConvertFrom(from_addr);
                Ipv4Address from_ip = Addr2Ip(from_s_addr);
                p_pkt->RemoveHeader(bp_header);
                if (p_pkt->GetSize() == 0) {
                    NS_LOG_ERROR(LogPrefixMacro << "ERROR: can't be size = 0, bp_header.get_payload_size =" << bp_header.get_payload_size() << ";bundle_type=" << bp_header.get_bundle_type());
                    std::abort();
                }
                NS_LOG_INFO(LogPrefixMacro << "receive bundle_type_ = " << bp_header.get_bundle_type());
                if (bp_header.get_bundle_type() == BundleType::TransmissionAck) {
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
                        seqno_was_acked
                    };
                    NS_LOG_LOGIC(LogPrefixMacro << "tmp_bh_info - seqno=" << tmp_bh_info.info_source_seqno_);
                    if (NS3DTNBIT_NO_FRAGMENT) {
                        NS_LOG_DEBUG(LogPrefixMacro << "well! we know the bundle has been accept, this transmit-session can be close");
                        int last = daemon_transmission_info_map_[tmp_bh_info].info_transmission_bundle_last_sent_bytes_;
                        daemon_transmission_info_map_[tmp_bh_info].info_transmission_current_sent_acked_bytes_ += last;
                        daemon_transmission_info_map_[tmp_bh_info].info_transmission_bundle_last_sent_bytes_ = 0;
                        if (daemon_transmission_info_map_.count(tmp_bh_info)) { // if duplicated ack, would duplicated erase
                            //daemon_transmission_info_map_.erase(daemon_transmission_info_map_.find(tmp_bh_info)); don't erase yet, TODO
                        }
                    } else {
                        int total = daemon_transmission_info_map_[tmp_bh_info].info_transmission_total_send_bytes_;
                        int current = daemon_transmission_info_map_[tmp_bh_info].info_transmission_current_sent_acked_bytes_;
                        int last = daemon_transmission_info_map_[tmp_bh_info].info_transmission_bundle_last_sent_bytes_;
                        if (total ==  current + last) {
                            NS_LOG_DEBUG(LogPrefixMacro << "well! we know the bundle has been accept, this transmit-session can be close");
                            daemon_transmission_info_map_[tmp_bh_info].info_transmission_current_sent_acked_bytes_ += last;
                            daemon_transmission_info_map_[tmp_bh_info].info_transmission_bundle_last_sent_bytes_ = 0;
                            //daemon_transmission_info_map_.erase(daemon_transmission_info_map_.find(tmp_bh_info)); don't erase yet, TODO
                        } else if (total > current + last){
                            daemon_transmission_info_map_[tmp_bh_info].info_transmission_current_sent_acked_bytes_ += last;
                            NS_LOG_INFO(LogPrefixMacro << "here, before ToTransmit(), to transmit more"
                                    << "\ntotal =" << total << "\ncurrent_sent=" 
                                    << current << "\nlast_sent=" << last);
                            ToTransmit(tmp_bh_info);
                        } else {
                            NS_LOG_WARN(LogPrefixMacro << "WARN:can be true when transmit-fail-retransmit and original transmit both success, following code would handle this, total, cur, last =" << total << " " << current << " " << last 
                                    << "\n" 
                                    << "\n" << "from_ip = " << tmp_bh_info.info_transmit_addr_);
                            NS_LOG_ERROR("not fatal error: we don't have handling code, please change NS3DTNBIT_RETRANSMISSION_INTERVAL to bigger one, if max_range is 4,000 interval of value 1.7 would be enough");
                        }
                        return;
                    }
                } else if (bp_header.get_bundle_type() == BundleType::StorageinfoMaintainPkt) {
                    // CGRQM TODO
                    // send ack back and cope with StorageinfoMaintainInterface("receive neighbor storageinfo"), 
                    map<int, pair<int, int>> parsed_storageinfo_from_neighbor;
                    map<int, pair<int, int>> empty01;
                    map<int, int> empty02;
                    vector<int> empty03;
                    {
                        // parse 
                        std::stringstream ss;
                        int sizeofstorageinfo;
                        p_pkt->CopyData(&ss, bp_header.get_payload_size());
                        ss >> sizeofstorageinfo;
                        for (int i = 0; i < sizeofstorageinfo; i++) {
                            int nodeid, belivevalue, cachvalue;
                            ss >> nodeid >> belivevalue >> cachvalue;
                            parsed_storageinfo_from_neighbor[nodeid] = {belivevalue, cachvalue};
                        }
                    }
                    out_app_.routing_assister_.StorageinfoMaintainInterface("receive neighbor Storageinfo", parsed_storageinfo_from_neighbor, empty01, empty02, empty03);
                } else if (bp_header.get_bundle_type() == BundleType::TransmissionAck) {
                    // if not, send 'transmission ack' first
                    NS_LOG_INFO(LogPrefixMacro << "here, received anti or bundle, send ack back first, before ToSendAck" << "; ip=" << from_ip 
                            << "; bp_header=" << bp_header);
                    NS_LOG_DEBUG(LogPrefixMacro << "receive bundle from node-" << Ipv42NodeNo(from_ip) << " to node-" << Ipv42NodeNo(out_app_.own_ip_));
                    ToSendAckDetail(bp_header, from_ip);
                    out_app_.seqno2fromid_map_[bp_header.get_source_seqno()] = Ipv42NodeNo(from_ip);
                    {
                        // process bundle or anti-pkt
                        NS_LOG_INFO(LogPrefixMacro << "here; after send ack back, process recept bp_header: " << bp_header);
                        if (bp_header.get_bundle_type() == BundleType::AntiPacket) {
                            std::abort();
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
                            DaemonBundleHeaderInfo tmp_header_info = {
                                from_ip,
                                bp_header.get_source_seqno(),
                            };
                            daemon_reception_info_map_[tmp_header_info] = tmp_recept_info;
                            p_pkt->AddHeader(bp_header);
                            daemon_reception_info_map_[tmp_header_info].info_fragment_pkt_pointer_vec_.push_back(p_pkt);
                            BundleReceptionTailWorkDetail(tmp_header_info);
                        } else {
                            NS_LOG_ERROR(LogPrefixMacro << "ERROR:error! no possible");
                            std::abort();
                        } // later usage
                        NS_LOG_INFO(LogPrefixMacro << "out of recervebundle");
                    }
                } else {
                    // UnKnow
                    std::abort();
                }
            }
            NS_LOG_INFO(LogPrefixMacro << "ReceiveHelloDetail");
        }

        void DtnApp::DtnAppTransmitSessionAssister::ToSendAckDetail(BPHeader& ref_bp_header, Ipv4Address response_ip) {
            std::string tmp_payload_str;
            {
                // fill up payload
                std::stringstream tmp_sstream;
                tmp_sstream << out_app_.own_ip_; 
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
                out_app_.SemiFillBPHeaderDetail(&bp_header);
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
        }

        bool DtnApp::DtnAppTransmitSessionAssister::SocketSendDetail(Ptr<Packet> p_pkt, uint32_t flags, InetSocketAddress trans_addr) {
            BPHeader bp_header;
            {
                p_pkt->RemoveHeader(bp_header);
                if (p_pkt->GetSize() == 0) { NS_LOG_ERROR(LogPrefixMacro << "ERROR:pkt size =" << p_pkt->GetSize() << ";bundle type=" << bp_header.get_bundle_type()); std::abort(); }
                bp_header.set_hop_time_stamp(Simulator::Now());
                bp_header.set_hop_ip(NodeNo2Ipv4(out_app_.node_->GetId()));
                p_pkt->AddHeader(bp_header);
                NS_LOG_INFO(LogPrefixMacro << "one bundle trans, header is " << bp_header);
            }
            if (out_app_.daemon_socket_handle_) {
                NS_LOG_INFO(LogPrefixMacro << "one bundle trans, trans_addr is " << Addr2Ip(trans_addr) << " " << trans_addr.GetPort());
                NS_LOG_DEBUG(LogPrefixMacro << "PKTTRACE: one bundle trans, from node-" << Ipv42NodeNo(out_app_.own_ip_) << " to node-" << Ipv42NodeNo(Addr2Ip(trans_addr)) << " seq : " << bp_header.get_source_seqno());
                int result = out_app_.daemon_socket_handle_->SendTo(p_pkt, flags, trans_addr);
                return result != -1 ? true : false;
            } else {
                NS_LOG_ERROR("socket_handle not initialized");
                std::abort();
            }
        }

        vector<Ipv4Address> DtnApp::DtnAppNeighborKeeper::PackageStillNeighborAvailableDetail(BPHeader& ref_bp_header) {
            vector<Ipv4Address> result;
            for (auto nk : neighbor_info_map_) {
                auto& neighbor_info_this_ = get<1>(nk);
                Ipv4Address nei_ip = get<0>(nk);
                bool nei_last_seen_bool = neighbor_info_this_.IsLastSeen();
                bool nei_have_space = neighbor_info_this_.info_daemon_baq_available_bytes_ > ref_bp_header.get_payload_size() + ref_bp_header.GetSerializedSize();
                bool nei_is_not_source = !nei_ip.IsEqual(ref_bp_header.get_source_ip());
                bool nei_is_not_from = out_app_.seqno2fromid_map_[ref_bp_header.get_source_seqno()] != Ipv42NodeNo(nei_ip);
                bool pre = nei_last_seen_bool && nei_have_space && nei_is_not_source && nei_is_not_from;

                if (pre) {
                    result.push_back(get<0>(nk));
                } else {
                    NS_LOG_INFO(LogPrefixMacro<<" not yet");
                }
            }
            return result;
        }

        void DtnApp::DtnAppTransmitSessionAssister::BundleReceptionTailWorkDetail(DaemonBundleHeaderInfo tmp_header_info) {
            BPHeader bp_header;
            Ptr<Packet> tmp_p_pkt;
            tmp_p_pkt = daemon_reception_info_map_[tmp_header_info].info_fragment_pkt_pointer_vec_[0]->Copy();
            tmp_p_pkt->RemoveHeader(bp_header);
            if (bp_header.get_payload_size() == tmp_p_pkt->GetSize()) {
                // this bundle is non-fragment or a already reassemble one
                bool is_dupli_check = out_app_.IsDuplicatedDetail(bp_header);
                if (is_dupli_check) {
                    // check Duplicates here
                    NS_LOG_WARN(LogPrefixMacro << "WARN: receive a duplicated bundle, or a bundle has been store in this node once. This may happen");
                    return;
                } else {
                    ThisIsNotDup();
                }
                if (bp_header.get_destination_ip().IsEqual(out_app_.own_ip_)) {
                    NS_LOG_DEBUG(LogPrefixMacro << "NOTE:BundleTrace:Great! one bundle arrive destination! bp_header=" << bp_header);
                    NS_LOG_DEBUG(LogPrefixMacro << "PKTTRACE:BundleTrace:Great! one bundle arrive destination! bp_header=" << bp_header << " seq : " << bp_header.get_source_seqno());
                    //ToSendAntipacketBundle(bp_header);
                    tmp_p_pkt->AddHeader(bp_header);
                    out_app_.daemon_consume_bundle_queue_->Enqueue(Packet2Queueit(tmp_p_pkt));
                    out_app_.before_receive_seqno_set_.insert(bp_header.get_source_seqno());
                } else {
                    if (bp_header.get_bundle_type() == BundleType::BundlePacket) {
                        NS_LOG_DEBUG(LogPrefixMacro << "NOTE:BundleTrace:good! one bundle recept, it's one hop! bp_header=" << bp_header);
                        NS_LOG_DEBUG(LogPrefixMacro << "PKTTRACE: receive one," << " from node-" << Ipv42NodeNo(tmp_header_info.info_transmit_addr_) << " to node-" << Ipv42NodeNo(out_app_.own_ip_) << " seq : " << bp_header.get_source_seqno());
                    } else {
                        NS_LOG_ERROR(LogPrefixMacro << "ERROR: can't be");
                    }
                    tmp_p_pkt->AddHeader(bp_header);
                    out_app_.daemon_bundle_queue_->Enqueue(Packet2Queueit(tmp_p_pkt));
                    out_app_.before_receive_seqno_set_.insert(bp_header.get_source_seqno());
                }
            } else {
                NS_LOG_ERROR(LogPrefixMacro << "fragment not solved!");
                std::abort();
            }

            if (NS3DTNBIT_NO_FRAGMENT) {
                NS_LOG_INFO(LogPrefixMacro << "erase reception info");
                daemon_reception_info_map_.erase(daemon_reception_info_map_.find(tmp_header_info));
            }
        }

        void DtnApp::DtnAppTransmitSessionAssister::TransmitSessionFailCheck(DaemonBundleHeaderInfo bh_info, int last_time_current) {
            bool last_trans_was_acked = false;
            if (size_t(last_time_current) < daemon_transmission_info_map_[bh_info].info_transmission_current_sent_acked_bytes_) {
                last_trans_was_acked = true;
            }
            if (last_trans_was_acked) {
                NS_LOG_INFO(LogPrefixMacro << "seqno=" << bh_info.info_source_seqno_ << "; this is acked, successed! ");
            } else {
                NS_LOG_INFO(LogPrefixMacro << "seqno=" << bh_info.info_source_seqno_ << "; this is not acked, retransmit!");
                ToTransmit(bh_info);
            }
        }

        bool DtnApp::DtnAppRoutingAssister::ShallWait() {
            if (get_rm() == RoutingMethod::Other) {
                return false;
            } else {
                return true;
            }
        }

        // CGRQM TODO
        // add all arg in one method
        void DtnApp::DtnAppRoutingAssister::StorageinfoMaintainInterface(string s
                ,map<int, pair<int, int>> parsed_storageinfo_from_neighbor
                ,map<int, pair<int, int>>& move_storageinfo_to_this
                ,map<int, int> storagemax
                ,vector<int> path_of_route
                , pair<int, int> update
                ) {
            if (rm_ == RoutingMethod::QM) {
                p_rm_in_->StorageinfoMaintainInterface(s, parsed_storageinfo_from_neighbor, move_storageinfo_to_this, storagemax, path_of_route, update);
            } else {
                NS_LOG_WARN("do nothing, fix me!");
            }
        }

        int DtnApp::DtnAppRoutingAssister::FindTheNeighborThisBPHeaderTo(BPHeader& ref_bp_header) {
            int s, d, result;
            {
                // init s, d
                auto ip_s = ref_bp_header.get_source_ip();
                auto ip_d = ref_bp_header.get_destination_ip();
                if (ip_d == out_app_.own_ip_) {NS_LOG_INFO(LogPrefixMacro << "this is destination, would return false and continue."); return -1;} 
                s = Ipv42NodeNo(ip_s);
                d = Ipv42NodeNo(ip_d);
            }
            vector<Ipv4Address> available_ip = out_app_.neighbor_keeper_.PackageStillNeighborAvailableDetail(ref_bp_header);
            vector<int> available;
            for (auto ip : available_ip) { available.push_back(Ipv42NodeNo(ip)); }
            if (available.empty()) {NS_LOG_INFO(LogPrefixMacro << "available is none, return false"); return -1;}

            // check the routing method and invoke by their way
            if (IsSet()) {
                dtn_seqno_t that_seqno = ref_bp_header.get_source_seqno();
                if (get_rm() == RoutingMethod::SprayAndWait) {
                    // TODO get a random 'A' 
                    int random_A = std::rand();
                    NS_LOG_INFO(LogPrefixMacro << "Available has " << available.size() << " random_A is " << random_A);
                    if (available.size() > 1) {
                        NS_LOG_DEBUG(LogPrefixMacro << "FUCK!!!! Available has " << available.size() << " random_A is " << random_A);
                        for (auto a : available) { NS_LOG_DEBUG(LogPrefixMacro << a); }
                    }
                    result = available[random_A % available.size()];
                } else if (get_rm() == RoutingMethod::DirectForward) {
                    // TODO get a random 'A' 
                    int random_A = std::rand();
                    NS_LOG_INFO(LogPrefixMacro << "Available has " << available.size());
                    result = available[random_A % available.size()];
                } else if (get_rm() == RoutingMethod::Other) {
                    p_rm_in_->GetInfo(-1, -1, vector<int>(), -1, -1.1, -1, -1, out_app_.id2cur_exclude_vec_of_id_, -1.1, that_seqno);
                    result = RouteIt(out_app_.node_->GetId(), d);
                } else if (get_rm() == RoutingMethod::TimeExpanded) {
                    p_rm_in_->GetInfo(-1, -1, vector<int>(), -1, -1.1, -1, -1, out_app_.id2cur_exclude_vec_of_id_, -1.1, that_seqno);
                    result = RouteIt(out_app_.node_->GetId(), d);
                } else if (get_rm() == RoutingMethod::CGR) {
                    int destination_id = Ipv42NodeNo(ref_bp_header.get_destination_ip());
                    int from_id = -1;
                    auto found = out_app_.seqno2fromid_map_.find(ref_bp_header.get_source_seqno());
                    if (found != out_app_.seqno2fromid_map_.end()) {
                        from_id = out_app_.seqno2fromid_map_[ref_bp_header.get_source_seqno()];
                    } else {
                        from_id = out_app_.node_->GetId();
                    }
                    vector<int> vec_of_current_neighbor;
                    for (auto nei : out_app_.neighbor_keeper_.neighbor_info_map_) {
                        if (get<1>(nei).IsLastSeen()) {
                            vec_of_current_neighbor.push_back(Ipv42NodeNo(get<0>(nei)));
                        }
                    }
                    int own_id = out_app_.node_->GetId();
                    dtn_time_t expired_time = ref_bp_header.get_src_time_stamp().GetSeconds() + NS3DTNBIT_HYPOTHETIC_BUNDLE_EXPIRED_TIME;
                    int bundle_size = ref_bp_header.get_payload_size();
                    int flag = 0;
                    dtn_time_t current_time = Simulator::Now().GetSeconds();

                    // -------------- dividing ----------
                    p_rm_in_->GetInfo(destination_id, from_id, vec_of_current_neighbor, own_id, expired_time, 
                            bundle_size, flag, out_app_.id2cur_exclude_vec_of_id_, current_time, that_seqno);
                    result = RouteIt(out_app_.node_->GetId(), d);
                } else if (get_rm() == RoutingMethod::QM) {
                    int destination_id = Ipv42NodeNo(ref_bp_header.get_destination_ip());
                    int from_id = -1;
                    auto found = out_app_.seqno2fromid_map_.find(ref_bp_header.get_source_seqno());
                    if (found != out_app_.seqno2fromid_map_.end()) {
                        from_id = out_app_.seqno2fromid_map_[ref_bp_header.get_source_seqno()];
                    } else {
                        from_id = out_app_.node_->GetId();
                    }
                    vector<int> vec_of_current_neighbor;
                    for (auto nei : out_app_.neighbor_keeper_.neighbor_info_map_) {
                        if (get<1>(nei).IsLastSeen()) {
                            vec_of_current_neighbor.push_back(Ipv42NodeNo(get<0>(nei)));
                        }
                    }
                    int own_id = out_app_.node_->GetId();
                    dtn_time_t expired_time = ref_bp_header.get_src_time_stamp().GetSeconds() + NS3DTNBIT_HYPOTHETIC_BUNDLE_EXPIRED_TIME;
                    int bundle_size = ref_bp_header.get_payload_size();
                    int flag = 0;
                    dtn_time_t current_time = Simulator::Now().GetSeconds();

                    // -------------- dividing ----------
                    p_rm_in_->GetInfo(destination_id, from_id, vec_of_current_neighbor, own_id, expired_time, 
                            bundle_size, flag, out_app_.id2cur_exclude_vec_of_id_, current_time, that_seqno);
                    result = RouteIt(out_app_.node_->GetId(), d);
                } else {
                    std::abort();
                }
                if (result == int(out_app_.node_->GetId())) {NS_LOG_WARN(LogPrefixMacro << "WARN: routing self!  " << ";d=" << d << ";result = " << result);}
                if (result != -1) {
                    auto ipkey = NodeNo2Ipv4(result);
                    if (out_app_.neighbor_keeper_.neighbor_info_map_.count(ipkey) 
                            && out_app_.neighbor_keeper_.neighbor_info_map_[ipkey].IsLastSeen()) {
                        return result;
                    }
                    NS_LOG_INFO(LogPrefixMacro << "routing decision is not in available, or have be sent; we would wait and abond this" << " all available is: ");
                    for (auto v : available) { NS_LOG_INFO("v = " << v << "."); }
                    if (ShallWait()) {
                        return -1;
                    } else {
                        int random_A = std::rand();
                        return available[random_A % available.size()];
                    }
                } else {
                    return -1;
                }
            } else {
                NS_LOG_ERROR("can't find the routing method or method not assigned, routing_assister_ is set=" << IsSet());
                std::abort();
            }
        }

        bool DtnApp::IsDuplicatedDetail(BPHeader& bp_header) {
            /*
               auto found_in_before_receive_seqno_set = before_receive_seqno_set_.find(bp_header.get_source_seqno());
               if (found_in_before_receive_seqno_set != before_receive_seqno_set_.end()) {
               return true;
               } else {
               return false;
               }
               */
            return before_receive_seqno_set_.count(bp_header.get_source_seqno());
        }

        void DtnApp::DtnAppTransmitSessionAssister::InitTransmission(Ipv4Address nei_ip, BPHeader bp_header, bool& is_exist) {
            DaemonBundleHeaderInfo tmp_header_info = {
                nei_ip,
                bp_header.get_source_seqno(),
            };
            DaemonTransmissionInfo tmp_transmission_info = {
                bp_header.get_payload_size() + bp_header.GetSerializedSize(),
                0,
                Simulator::Now().GetSeconds(),
                Simulator::Now().GetSeconds(),
                0,
                {},
                bp_header.get_retransmission_count(),
            };
            bool transmit_session_already = daemon_transmission_info_map_.count(tmp_header_info);
            assert(transmit_session_already == (daemon_transmission_info_map_.find(tmp_header_info) != daemon_transmission_info_map_.end()));
            if (transmit_session_already) {
                NS_LOG_WARN(LogPrefixMacro << "WARN:transmit-session already exist, head = " << bp_header 
                        << " \n headinfo=" << tmp_header_info.info_transmit_addr_ << " " << tmp_header_info.info_source_seqno_);
#ifdef UGLY_DEBUG
                NS_LOG_INFO(LogPrefixMacro << "Print daemon_transmission_info_map_\n");
                for (auto& dti : daemon_transmission_info_map_) {
                    NS_LOG_INFO(LogPrefixMacro << " ----- nei_ip=" << get<0>(dti).info_transmit_addr_ << " --- seqno=" << get<0>(dti).info_source_seqno_);
                }
#endif /* ifndef  */
                is_exist = true;
            } else {
                NS_LOG_DEBUG(LogPrefixMacro << "PKTTRACE:transmission session Enqueue" << " from node-" << Ipv42NodeNo(out_app_.own_ip_) << " to node-" << Ipv42NodeNo(nei_ip) << " seq " << bp_header.get_source_seqno() << " seq : " << bp_header.get_source_seqno());
                daemon_transmission_info_map_[tmp_header_info] = tmp_transmission_info;
                out_app_.ReplicationGoodDetail(bp_header, 1);
            }
        }

        void DtnApp::DtnAppTransmitSessionAssister::ToTransmit(DaemonBundleHeaderInfo bh_info) {
            bool neighbor_nearby = false;
            bool this_session_not_done = false;
            bool this_session_not_ove_retx_time = false;
            auto ip_to = bh_info.info_transmit_addr_;
            BPHeader tran_bp_header;
            int offset_value;
            {
                // check state, cancel transmission if condition
                if (0 == daemon_transmission_info_map_.count(bh_info) || 
                        daemon_transmission_info_map_[bh_info].info_transmission_total_send_bytes_ == daemon_transmission_info_map_[bh_info].info_transmission_current_sent_acked_bytes_) {
                    NS_LOG_INFO(LogPrefixMacro << "this transmit-session has been done! we would return and drop this transmit." 
                            << "transmit-to-ip=" << bh_info.info_transmit_addr_
                            << ";seqno=" << bh_info.info_source_seqno_
                            << "; if this transmit-to-ip is equal to last, the robin-round schedule may not work");
                    return;
                } else {
                    this_session_not_done = true;
                }
                if (daemon_transmission_info_map_[bh_info].info_retransmission_count_ > NS3DTNBIT_MAX_RETRANSMISSION) {
                    NS_LOG_INFO(LogPrefixMacro << "this transmit-session is over max-retranmission time , would drop this transmit." 
                            << "transmit-to-ip=" << bh_info.info_transmit_addr_
                            << ";seqno=" << bh_info.info_source_seqno_);
                    return;
                } else {
                    this_session_not_ove_retx_time = true;
                }
                if (out_app_.neighbor_keeper_.neighbor_info_map_.count(ip_to)) {
                    if (out_app_.neighbor_keeper_.neighbor_info_map_[ip_to].IsLastSeen()) {
                        neighbor_nearby = true;
                    } else {
                        Simulator::Schedule(Seconds(NS3DTNBIT_HELLO_BUNDLE_INTERVAL_TIME * 3), 
                                &::ns3::ns3dtnbit::DtnApp::DtnAppTransmitSessionAssister::ToTransmit, this, bh_info);
                        NS_LOG_WARN(LogPrefixMacro 
                                << "WARN:to transmit: can't find neighbor or neighbor not recently seen. This might happen when ToTransmit() with is_retransmit = true, we would cancel this try, and retry next time." 
                                << ",last seen time=" << (double)out_app_.neighbor_keeper_.neighbor_info_map_[ip_to].info_last_seen_time_ << ";base time=" << Simulator::Now().GetSeconds() - (NS3DTNBIT_HELLO_BUNDLE_INTERVAL_TIME));
                        return;
                    }
                }
            }

            if (this_session_not_done && this_session_not_ove_retx_time && neighbor_nearby) {
                Ptr<Packet> tran_p_pkt;
                if (NS3DTNBIT_NO_FRAGMENT) {
                    tran_p_pkt = daemon_transmission_info_map_[bh_info].info_transmission_pck_buffer_[0];
                }
                if (daemon_transmission_info_map_[bh_info].info_retransmission_count_ > 0) {
                    NS_LOG_INFO("---- seqno=" << bh_info.info_source_seqno_ << ";retransmit_count  = " << daemon_transmission_info_map_[bh_info].info_retransmission_count_);
                }
                tran_p_pkt->RemoveHeader(tran_bp_header);
                offset_value = tran_p_pkt->GetSize();
                assert(offset_value == tran_bp_header.get_payload_size());
                if (offset_value == 0) {
                    NS_LOG_ERROR(LogPrefixMacro << "tran_p_pkt.size() = 0" << " bp_header :" << tran_bp_header);
                    std::abort();
                }
                assert(tran_p_pkt->GetSize()!=0);
                tran_bp_header.set_offset(offset_value);
                tran_p_pkt->AddHeader(tran_bp_header);
                {
                    // update state
                    auto& target = daemon_transmission_info_map_[bh_info];
                    target.info_transmission_bundle_last_sent_time_ = Simulator::Now().GetSeconds();
                    target.info_transmission_bundle_last_sent_bytes_ = tran_p_pkt->GetSize();
                }
                if (!NS3DTNBIT_NO_ROBUST_TRANSMIT){
                    // fail check
                    int last_time_current = daemon_transmission_info_map_[bh_info].info_transmission_current_sent_acked_bytes_;
                    Simulator::Schedule(Seconds(NS3DTNBIT_RETRANSMISSION_INTERVAL), 
                            &::ns3::ns3dtnbit::DtnApp::DtnAppTransmitSessionAssister::TransmitSessionFailCheck, this, bh_info, last_time_current);
                }
                if (!SocketSendDetail(tran_p_pkt, 0, InetSocketAddress(ip_to, NS3DTNBIT_PORT_NUMBER))) {
                    NS_LOG_ERROR("SocketSendDetail fail");
                    std::abort();
                }
            }
        }

        void DtnApp::DtnAppNeighborKeeper::ReceiveHelloDetail(Ptr<Socket>& socket_handle) {
            Ptr<Packet> p_pkt;
            string avli_s;
            BPHeader bp_header;
            Address from_addr;
            while ((p_pkt = socket_handle->RecvFrom(from_addr))) {
                InetSocketAddress addr = InetSocketAddress::ConvertFrom(from_addr);
                Ipv4Address ip_from = Addr2Ip(addr);
                if (!neighbor_info_map_.count(ip_from)) {
                    NeighborInfo tmp_neighbor_info = {
                        0,
                        0,
                    };
                    neighbor_info_map_[ip_from] = tmp_neighbor_info;
                }
                std::stringstream pkt_str_stream;
                p_pkt->RemoveHeader(bp_header);
                p_pkt->CopyData(&pkt_str_stream, bp_header.get_payload_size());
                // parse raw content of pkt and update 'neighbor_info_vec_'
                pkt_str_stream >> avli_s;
                neighbor_info_map_[ip_from].info_daemon_baq_available_bytes_ = stoi(avli_s);
                // CGRQM TODO
                map<int, pair<int, int>> empty01;
                map<int, pair<int, int>> empty02;
                map<int, int> empty03;
                vector<int> empty04;
                out_app_.routing_assister_.StorageinfoMaintainInterface("update storage info from hello"
                        ,empty01
                        ,empty02
                        ,empty03
                        ,empty04
                        ,{Ipv42NodeNo(ip_from), (stoi(avli_s) / NS3DTNBIT_HYPOTHETIC_CACHE_FACTOR)});
                neighbor_info_map_[ip_from].info_last_seen_time_ = Simulator::Now().GetSeconds();
            }
        }

        int DtnApp::DtnAppRoutingAssister::RouteIt(int s, int d) {
            return p_rm_in_->DoRoute(s, d);
        }

        RoutingMethodInterface::RoutingMethodInterface(DtnApp& dp) : out_app_(dp) {}
        RoutingMethodInterface::~RoutingMethodInterface() {}
        void RoutingMethodInterface::GetInfo(int destination_id, int from_id, std::vector<int> vec_of_current_neighbor, int own_id, dtn_time_t expired_time, int bundle_size, int networkconfigurationflag, map<int, vector<int>> id2cur_exclude_vec_of_id, dtn_time_t local_time, dtn_seqno_t that_seqno) {
            // nothing
        }

        // CGRQM TODO
        void RoutingMethodInterface::StorageinfoMaintainInterface(string s
                ,map<int, pair<int, int>> parsed_storageinfo_from_neighbor
                ,map<int, pair<int, int>>& move_storageinfo_to_this
                ,map<int, int> storagemax
                ,vector<int> path_of_route
                , pair<int, int> update
                ) {
            // nothing
        }
        Adob RoutingMethodInterface::get_adob() { return out_app_.routing_assister_.vec_[0]; }

    } /* ns3dtnbit */ 
}

namespace ns3 {
    namespace ns3dtnbit {

        void DtnApp::ReactMain(string s) {
            if (s == "NewTransmitCheck") {
                int decision_neighbor = -1;
                BPHeader bp_header;
                if (wifi_ph_p->IsStateIdle()) {
                    NS_LOG_LOGIC(LogPrefixMacro << "is stateidle");
                    //NS_LOG_DEBUG(LogPrefixMacro << "we have NPackets = " << daemon_bundle_queue_->GetNPackets());
                    for (size_t n = 0; n < daemon_bundle_queue_->GetNPackets(); n++) {
                        Ptr<Packet> p_pkt = daemon_bundle_queue_->Dequeue()->GetPacket();
                        //int p_pkt_size = p_pkt->GetSize();
                        p_pkt->RemoveHeader(bp_header);
                        //assert(p_pkt_size == bp_header.get_payload_size() + bp_header.GetSerializedSize());
                        if (Simulator::Now().GetSeconds() - bp_header.get_src_time_stamp().GetSeconds() < NS3DTNBIT_HYPOTHETIC_BUNDLE_EXPIRED_TIME) {
                            bool bundle_check_good = (ReplicationGoodDetail(bp_header, 0));
                            bool bundle_lazy_transmit_check = Simulator::Now().GetSeconds() - bp_header.get_hop_time_stamp().GetSeconds() > NS3DTNBIT_HELLO_BUNDLE_INTERVAL_TIME;
                            bool bundle_dest_not_this_check = !bp_header.get_destination_ip().IsEqual(own_ip_);
                            if (bundle_check_good && bundle_dest_not_this_check && bundle_lazy_transmit_check) {
                                decision_neighbor = routing_assister_.FindTheNeighborThisBPHeaderTo(bp_header);
                                if (decision_neighbor == -1) { p_pkt->AddHeader(bp_header); daemon_bundle_queue_->Enqueue(Packet2Queueit(p_pkt)); continue; } else {
                                    NS_LOG_DEBUG(LogPrefixMacro << "PKTTRACE: One decision is made, " << "from node-" << Ipv42NodeNo(own_ip_) << " to node-" << decision_neighbor << " seq : " << bp_header.get_source_seqno());
                                }
                                auto nei_ip = NodeNo2Ipv4(decision_neighbor);
                                bool is_exist = false;
                                transmit_assister_.InitTransmission(nei_ip, bp_header, is_exist);
                                DaemonBundleHeaderInfo tmp_header_info = {
                                    nei_ip,
                                    bp_header.get_source_seqno()
                                };
                                p_pkt->AddHeader(bp_header);
                                daemon_bundle_queue_->Enqueue(Packet2Queueit(p_pkt));
                                if (!is_exist) {
                                    Ptr<Packet> p_pkt_copy = p_pkt->Copy();
                                    transmit_assister_.daemon_transmission_info_map_[tmp_header_info].info_transmission_pck_buffer_.push_back(p_pkt_copy);
                                    transmit_assister_.ToTransmit(tmp_header_info);
                                }
                            } else {
                                p_pkt->AddHeader(bp_header); daemon_bundle_queue_->Enqueue(Packet2Queueit(p_pkt)); continue; 
                            }
                        } else {
                            continue;   // would remove expired package
                        }
                    }
                }
            } else if (s == "ReceiveBundle") {
                NS_LOG_INFO(LogPrefixMacro << "React ReceiveBundle");
                // should I check every time I receive Hello?
                if (transmit_assister_.NotDuplicatedBundle()) {
                    ReactMain("NewTransmitCheck");
                }
            } else if (s == "ReceiveHello") {
                NS_LOG_INFO(LogPrefixMacro << "React ReceiveHello");
                auto t01 = neighbor_keeper_.HasNewNeighbor();
                auto t011 = t01.first;
                auto t012 = t01.second;
                if (neighbor_keeper_.CheckBufferTimePass() || t011) {  
                    if (t011) {
                        // CGRQM TODO send routing_assister_.StorageinfoMaintainInterface("to send storageinfo to neighbor")
                        Ipv4Address toneighbor_ip = t012;
                        map<int, pair<int, int>> empty01;
                        map<int, pair<int, int>> current_storageinfo;
                        map<int, int> empty02;
                        vector<int> empty03;
                        routing_assister_.StorageinfoMaintainInterface("to send storageinfo to neighbor", empty01, current_storageinfo, empty02,empty03);
                        {
                            // send storageinfo to 
                            std::string tmp_payload_str;
                            {
                                // fill up payload
                                std::stringstream tmp_sstream;
                                tmp_sstream << current_storageinfo.size();
                                for (auto ccp : current_storageinfo) {
                                    tmp_sstream << " ";
                                    tmp_sstream << ccp.first;
                                    tmp_sstream << " ";
                                    tmp_sstream << ccp.second.first;
                                    tmp_sstream << " ";
                                    tmp_sstream << ccp.second.second;
                                }
                                tmp_payload_str = tmp_sstream.str();
                            }
                            Ptr<Packet> p_pkt = Create<Packet>(tmp_payload_str.c_str(), tmp_payload_str.size());
                            BPHeader bp_header;
                            {
                                // fill up bp_header
                                SemiFillBPHeaderDetail(&bp_header);
                                bp_header.set_bundle_type(BundleType::StorageinfoMaintainPkt);
                                bp_header.set_destination_ip(toneighbor_ip);
                                bp_header.set_source_seqno(p_pkt->GetUid());
                                bp_header.set_payload_size(tmp_payload_str.size());
                                bp_header.set_offset(tmp_payload_str.size());
                            }
                            p_pkt->AddHeader(bp_header);
                            InetSocketAddress toneighbor_addr = InetSocketAddress(toneighbor_ip, NS3DTNBIT_PORT_NUMBER);
                            if (!transmit_assister_.SocketSendDetail(p_pkt, 0, toneighbor_addr)) {
                                NS_LOG_ERROR("SOCKET send error");
                                std::abort();
                            }
                        }
                    }
                    ReactMain("NewTransmitCheck");
                }
            } else if (s == "OneTransmited") {

            } else {
                std::abort();
            }
        }

        void DtnApp::CheckBuffer(CheckState check_state) {
            if (!daemon_socket_handle_) {
                CreateSocketDetail();
                if (daemon_socket_handle_) {
                    NS_LOG_WARN(LogPrefixMacro << "NOTE:deamon_socket_handle not init, init now");
                } else {
                    NS_LOG_ERROR(LogPrefixMacro << "ERROR: can't init socket");
                }
            }
        }

        void DtnApp::ReceiveBundle(Ptr<Socket> socket) {
            transmit_assister_.DuplicatedBundleGuard(); 
            transmit_assister_.ReceiveBundleDetail(socket);
            ReactMain("ReceiveBundle");
        }

        void DtnApp::ReceiveHello(Ptr<Socket> socket_handle) {
            neighbor_keeper_.ReceiveHelloDetail(socket_handle);
            ReactMain("ReceiveHello");
        }

        void DtnApp::ToSendHello(Ptr<Socket> socket, dtn_time_t simulation_end_time, Time first_time) {
            if (hello_schedule_flag_) {
                Simulator::Schedule(Seconds(NS3DTNBIT_HELLO_BUNDLE_INTERVAL_TIME), 
                        &DtnApp::ToSendHello, this, socket, simulation_end_time, first_time);
                neighbor_keeper_.SendHelloDetail(socket);
            } else {
                Simulator::Schedule(first_time, 
                        &DtnApp::ToSendHello, this, socket, simulation_end_time, first_time);
                hello_schedule_flag_ = true;
            }
        }

        void DtnApp::ToSendBundle(uint32_t dstnode_number, uint32_t payload_size) {
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
                before_receive_seqno_set_.insert(bp_header.get_source_seqno());
                // NORMAL LOG
                NS_LOG_INFO(LogPrefixMacro << "normal out of ToSendBundle()");
            } else {
                // ERROR LOG
                NS_LOG_ERROR("Error : bundle is too big or no space");
                std::abort();
            }
        }

        void DtnApp::StateCheckDetail() {
            Simulator::Schedule(Seconds(10), 
                    &DtnApp::StateCheckDetail, this);
            NS_LOG_LOGIC(LogPrefixMacro << "Out of " << "StateCheckDetail()");
        }

        /*
         *  real send flags == 1
         *  
         * */
        bool DtnApp::ReplicationGoodDetail(BPHeader& bp_header, int flag) {
            NS_LOG_INFO(LogPrefixMacro << "in ReplicationGoodDetail");
            int v = bp_header.get_source_seqno();
            auto found = spray_map_.find(v);
            if (found == spray_map_.end()) {
                if (routing_assister_.get_rm() == RoutingMethod::SprayAndWait && bp_header.get_src_time_stamp().GetSeconds() + NS3DTNBIT_SPRAY_PHASE_TWO_TIME < Simulator::Now().GetSeconds()) {
                    // time is over, SprayAndWait enter the phase two of it's routing
                    spray_map_[v] = 1;
                } else {
                    spray_map_[v] = bp_header.get_spray();
                }
                return true;
            } else {
                if (spray_map_[v] > 0) {
                    if (spray_map_[v] == NS3DTNBIT_SPRAY_ARGUMENT - 1 && flag == 1) {NS_LOG_DEBUG(LogPrefixMacro << "PKTTRACE: FUCK!!! Package has been replicated, SprayAndWait!!");}
                    spray_map_[v] -= flag;
                    return true;
                } else if (spray_map_[v] == 0){
                    return false;
                } else {
                    std::abort();
                }
            }
        }

        void DtnApp::Report(std::ostream& os) {

        }

        void DtnApp::CreateSocketDetail() {
            daemon_socket_handle_ = Socket::CreateSocket(node_, TypeId::LookupByName("ns3::UdpSocketFactory"));
            Ptr<Ipv4> ipv4 = node_->GetObject<Ipv4>();
            Ipv4Address ipip = (ipv4->GetAddress(1, 0)).GetLocal();
            NS_LOG_INFO("create bundle send socket,ip=" << ipip << ";port=" << NS3DTNBIT_PORT_NUMBER);
            InetSocketAddress local = InetSocketAddress(ipip, NS3DTNBIT_PORT_NUMBER);
            daemon_socket_handle_->Bind(local);
        }

        void DtnApp::SemiFillBPHeaderDetail(BPHeader* p_bp_header) {
            char srcstring[1024] = "";
            sprintf(srcstring, "10.0.0.%d", (node_->GetId() + 1));
            p_bp_header->set_source_ip(srcstring);
            p_bp_header->set_hop_count(0);
            if (routing_assister_.get_rm() == RoutingMethod::TimeExpanded) {
                // because TimeExpanded routing is accurate, so, every node need only one transmition for every bundle-pkt
                p_bp_header->set_spray(1);
            } else if (routing_assister_.get_rm() == RoutingMethod::SprayAndWait) {
                p_bp_header->set_spray(NS3DTNBIT_SPRAY_ARGUMENT);
            } else if (routing_assister_.get_rm() == RoutingMethod::DirectForward) {
                p_bp_header->set_spray(1);
            } else if (routing_assister_.get_rm() == RoutingMethod::Other) {
                p_bp_header->set_spray(1);
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
        }

        void DtnApp::ScheduleTx (Time tNext, uint32_t dstnode, uint32_t payload_size) {
            NS_LOG_DEBUG(LogPrefixMacro << "enter ScheduleTx(), time-" << tNext << ",size=" << payload_size << ", to node-" << dstnode);
            Simulator::Schedule(tNext, 
                    &DtnApp::ToSendBundle, this, dstnode, payload_size);
        }

        std::string DtnApp::LogPrefix() {
            std::stringstream ss;
            ss << "[time-" << Simulator::Now().GetSeconds() 
                << ";node-" << node_->GetId() << ";";
            return ss.str();
        }

    } /* ns3dtnbit */ 
} /*ns3*/

