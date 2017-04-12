/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3dtn-bit.h"

using std::string;
using std::endl;
#ifdef DEBUG
/*
 * by default, get the function name called the logfunc which called this
 */
string GetCallStack(int i = 2) {
    int nptrs;
    void *buffer[200];
    char **cstrings;
    char* return_str = new char[200];

    nptrs = backtrace(buffer, 200);
    sprintf(return_str, "backtrace() returned %d addresses\n", nptrs);
    /* The call backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO)
     *               would produce similar output to the following: */

    cstrings = backtrace_symbols(buffer, nptrs);
    if (cstrings == NULL || nptrs < 3) {
        perror("backtrace_symbols");
        exit(EXIT_FAILURE);
    }
    sprintf(return_str, "%s\n", cstrings[i]);
    free(cstrings);
    return return_str;
}

string FilePrint(string str) {
    std::stringstream ss;
    char* cs = new char[200];
    std::sprintf(cs, "file : %s, line : %d,", __FILE__, __LINE__);
    ss << "====== FilePrint ===== " << cs << "--->>" << str << endl;
    return ss.str();
}

string GetLogStr(string str) {
    std::stringstream ss;
    string caller = GetCallStack();
    ss << "==== Caller ====" << caller << "\n---->>" << str << endl;
    string Filep = FilePrint(ss.str());
    return Filep;
}

#endif

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("DtnRunningLog");
    namespace ns3dtnbit {

        /*
         * Can't use CreateObject<>, so do it myself
         */
        static Ptr<QueueItem> Packet2Queueit(Ptr<Packet> p_pkt) {
            QueueItem* p = new QueueItem(p_pkt);
            return Ptr<QueueItem>(p);
        }

        static Ipv4Address NodeNo2Ipv4(int node_no) {
            auto ip_base = Ipv4Address("10.0.0.1");
            auto ip_v = ip_base.Get();
            ip_v += node_no;
            Ipv4Address result;
            result.Set(ip_v);
            std::cout << result << std::endl;
            return result;
        }

        static int Ipv42NodeNo(Ipv4Address ip) {
            auto ip_base = Ipv4Address("10.0.0.1");
            auto ip_base_v = ip_base.Get();
            auto ip_v = ip.Get();
            return ip_v - ip_base_v;
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
            NS_LOG_DEBUG(LogPrefixMacro << "NOTE:in startapplication()");
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
        }

        /* refine
         * Aim : inherited method
         */
        void DtnApp::SetUp(Ptr<Node> node) {
            node_ = node;
            congestion_control_parameter_ = 1;
            retransmission_interval_ = 15.0;
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

            daemon_baq_bytes_max_ = 1375000 + y->GetInteger(0, 1)*9625000;
        }

        /* refine 
         * Aim : send hello
         */
        void DtnApp::ToSendHello(Ptr<Socket> socket, dtn_time_t simulation_end_time, Time hello_interval, bool hello_right_now_boolean) {
            NS_LOG_LOGIC(LogPrefixMacro << "enter ToSendHello()");
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
                        int32_t tmp_number = ((dtn_seqno_t)(congestion_control_parameter_ * daemon_baq_bytes_max_) - (daemon_bundle_queue_->GetNBytes() + daemon_antipacket_queue_->GetNBytes()));
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
                for (; j < neighbor_info_vec_.size(); j++) {
                    auto ip_n = neighbor_info_vec_[j].info_address_.GetIpv4();
                    neighbor_found = (ip_n.IsEqual(ip_from)) ? true : false;
                    if (neighbor_found) {break;}
                }
                //NS_LOG_LOGIC(LogPrefixMacro << "here,j=" << j << ";ip_from=" << ip_from << ";found=" << neighbor_found);
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
                NS_LOG_INFO(LogPrefixMacro << "receive hello, node-" << Ipv42NodeNo(ip_from) << " to node-" << node_->GetId() << ", ip=" << ip_from << ";found =" << neighbor_found << ";j=" << j << ";pkt_seqno_number=" << pkt_seqno_number << ";Available_bytes_=" << neighbor_info_vec_[j].info_daemon_baq_available_bytes_ << ";avli_s=" << avli_s << ";size()=" << bp_header.get_payload_size());
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
            //NS_LOG_LOGIC(LogPrefixMacro << "out of receive hello");
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
            p_pkt->AddHeader(bp_header);
            if ((daemon_antipacket_queue_->GetNBytes() + daemon_bundle_queue_->GetNBytes() + p_pkt->GetSize() <= daemon_baq_bytes_max_)) {
                daemon_bundle_queue_->Enqueue(Packet2Queueit(p_pkt));
                // NORMAL LOG
                NS_LOG_DEBUG(LogPrefixMacro << "out of ToSendBundle()");
            } else {
                // ERROR LOG
                NS_LOG_ERROR("Error : bundle is too big or no space");
                std::abort();
            }
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
                    NS_LOG_LOGIC(LogPrefixMacro << "tmp_bh_info - seqno" << tmp_bh_info.info_source_seqno_);
                    int k = 0;
                    for (int kk = 0; kk < daemon_transmission_info_vec_.size(); kk++) {
                        if (tmp_bh_info == daemon_transmission_bh_info_vec_[kk]) { k = kk; break; }
                    }
                    int total = daemon_transmission_info_vec_[k].info_transmission_total_send_bytes_, current = daemon_transmission_info_vec_[k].info_transmission_current_sent_acked_bytes_, last =daemon_transmission_info_vec_[k].info_transmission_bundle_last_sent_bytes_;
                    if (total ==  current + last) {
                        NS_LOG_DEBUG(LogPrefixMacro << "good! we know the bundle has been accept, this transmit-session can be close");
                        daemon_transmission_info_vec_[k].info_transmission_current_sent_acked_bytes_ += last;
                    } else if (total > current + last){
                        daemon_transmission_info_vec_[k].info_transmission_current_sent_acked_bytes_ += last;
                        NS_LOG_DEBUG(LogPrefixMacro << "here, before ToTransmit(), to transmit more"
                                << "\ntotal =" << total << "\ncurrent_sent=" 
                                << current << "\nlast_sent=" << last);
                        ToTransmit(tmp_bh_info, false);
                    } else {
                        NS_LOG_ERROR(LogPrefixMacro << "can't be true, total, cur, last =" << total << " " << current << " " << last 
                                << "\n k =" << k  << " " << daemon_transmission_info_vec_.size() 
                                << " " << daemon_retransmission_packet_buffer_vec_.size() 
                                << " " << daemon_transmission_bh_info_vec_.size());
                        std::abort();
                    }
                    return;
                } else {
                    // if not, send 'transmission ack' first
                    NS_LOG_DEBUG(LogPrefixMacro << "here, received anti or bundle, send ack back first, before ToSendAck" << "; ip=" << from_ip 
                            << "; bp_header=" << bp_header);
                    ToSendAck(bp_header, from_ip);
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
                    NS_LOG_DEBUG(LogPrefixMacro << "NOTE:Great! one bundle arrive destination! seqno=" << bp_header.get_source_seqno());
                    ToSendAntipacketBundle(bp_header);
                    tmp_p_pkt->AddHeader(bp_header);
                    daemon_consume_bundle_queue_->Enqueue(Packet2Queueit(tmp_p_pkt->Copy()));
                    // this is a heuristic method to make hello, to let others know it already has it.
                    daemon_bundle_queue_->Enqueue(Packet2Queueit(tmp_p_pkt->Copy()));
                } else {
                    NS_LOG_DEBUG(LogPrefixMacro << "NOTE:good! one bundle recept, it's one hop! seqno=" << bp_header.get_source_seqno());
                    tmp_p_pkt->AddHeader(bp_header);
                    daemon_bundle_queue_->Enqueue(Packet2Queueit(tmp_p_pkt->Copy()));
                }
            } else {
                NS_LOG_ERROR(LogPrefixMacro << "fragment not solved!");
                std::abort();
            }
        }

        /* refine 
         * Aim : handle send bundle stuff after checkbuffer
         * Detail :
         * use bh_info as key get all msg, 
         * check whether it's able to call 'SocketSendDetail'
         * handle retransmit and transmit more
         * use daemon_retransmission_packet_buffer_vec_
         * note the packet size problem, size < NS3DTNBIT_HYPOTHETIC_TRANS_SIZE_FRAGMENT_MAX
         * update 'daemon_transmission_info_vec_'
         * anti-pkt or bundle-pkt
         */
        void DtnApp::ToTransmit(DaemonBundleHeaderInfo bh_info, bool is_retransmit) {
            NS_LOG_DEBUG(LogPrefixMacro << "enter ToTransmit()");
            bool real_send_boolean = false;
            int index = 0, j = 0;
            for (int ii = 0; ii < daemon_transmission_bh_info_vec_.size(); ii++) {
                // find the index of the 'bundle to transmit' to 'daemon_transmission_info_vec_' using 'bh_info'
                if (daemon_transmission_bh_info_vec_[ii] == bh_info) { index = ii; break; }
            }
            // check state, cancel transmission if condition
            if (daemon_transmission_info_vec_[index].info_transmission_total_send_bytes_ == daemon_transmission_info_vec_[index].info_transmission_current_sent_acked_bytes_) {
                NS_LOG_DEBUG(LogPrefixMacro << "this transmit-session has been done! we would return and drop this transmit." 
                        << "transmit-to-ip=" << bh_info.info_transmit_addr_.GetIpv4()
                        << ";seqno=" << bh_info.info_source_seqno_
                        << "; if this transmit-to-ip is equal to last, the robin-round schedule may not work");
                return;
            }
            for (int jj = 0; jj < neighbor_info_vec_.size(); jj++) {
                // find the neighbor should be transmit, if this neighbor was not recently seen, schedule 'ToTransmit' later, otherwise, set real_send_boolean
                auto ip_n = neighbor_info_vec_[jj].info_address_.GetIpv4();
                auto ip_to = bh_info.info_transmit_addr_.GetIpv4();
                if (ip_n.IsEqual(ip_to)) {
                    if (Simulator::Now().GetSeconds() - neighbor_info_vec_[jj].info_last_seen_time_  < NS3DTNBIT_HELLO_BUNDLE_INTERVAL_TIME * 3) {
                        real_send_boolean = true;
                        j = jj;
                        break;
                    } else {
                        Simulator::Schedule(Seconds(NS3DTNBIT_HELLO_BUNDLE_INTERVAL_TIME * 10), &DtnApp::ToTransmit, this, bh_info, false);
                        NS_LOG_WARN(LogPrefixMacro << "WARN:to transmit: can't find neighbor or neighbor not recently seen," << "j=" << jj << ",last seen time=" << (double)neighbor_info_vec_[jj].info_last_seen_time_ << ";base time=" << Simulator::Now().GetSeconds() - (NS3DTNBIT_HELLO_BUNDLE_INTERVAL_TIME * 3));
                        return;
                    }
                }
            }
            Ptr<Packet> tran_p_pkt;
            BPHeader tran_bp_header;
            int offset_value;
            if (real_send_boolean) {
                tran_p_pkt = daemon_retransmission_packet_buffer_vec_[index]->Copy(); 
                tran_p_pkt->RemoveHeader(tran_bp_header);
                if (daemon_transmission_info_vec_[index].info_transmission_total_send_bytes_ > NS3DTNBIT_HYPOTHETIC_TRANS_SIZE_FRAGMENT_MAX) {
                    NS_LOG_WARN(LogPrefixMacro << "WARN:fragment may have error");
                    {
                        // prepare bytes
                        int need_to_bytes = daemon_transmission_info_vec_[index].info_transmission_total_send_bytes_ - daemon_transmission_info_vec_[index].info_transmission_current_sent_acked_bytes_;
                        NS_LOG_INFO(LogPrefixMacro << "here" << ";daemon_reception_packet_buffer_vec_.size()" << daemon_reception_packet_buffer_vec_.size() << ";index" << index);
                        tran_p_pkt->RemoveAtStart(daemon_transmission_info_vec_[index].info_transmission_current_sent_acked_bytes_);
                        if (need_to_bytes > NS3DTNBIT_HYPOTHETIC_TRANS_SIZE_FRAGMENT_MAX) {
                            tran_p_pkt->RemoveAtEnd(need_to_bytes - NS3DTNBIT_HYPOTHETIC_TRANS_SIZE_FRAGMENT_MAX);
                        }
                        tran_bp_header.set_offset(daemon_transmission_info_vec_[index].info_transmission_current_sent_acked_bytes_ + tran_p_pkt->GetSize());
                        if (is_retransmit) {
                            tran_bp_header.set_retransmission_count(tran_bp_header.get_retransmission_count() + 1);
                        }
                        offset_value = tran_p_pkt->GetSize() + daemon_transmission_info_vec_[index].info_transmission_current_sent_acked_bytes_;
                    }
                } else {
                    offset_value = tran_p_pkt->GetSize();
                    assert(offset_value == tran_bp_header.get_payload_size());
                    if (offset_value == 0) {
                        NS_LOG_ERROR(LogPrefixMacro << "tran_p_pkt.size() = 0" << " bp_header :" << tran_bp_header);
                        std::abort();
                    }
                }
                assert(tran_p_pkt->GetSize()!=0);
                tran_bp_header.set_offset(offset_value);
                tran_p_pkt->AddHeader(tran_bp_header);
                {
                    // update state
                    daemon_transmission_info_vec_[index].info_transmission_bundle_last_sent_time_ = Simulator::Now().GetSeconds();
                    daemon_transmission_info_vec_[index].info_transmission_bundle_last_sent_bytes_ = tran_p_pkt->GetSize();
                    daemon_transmission_bh_info_vec_[index].info_retransmission_count_ = tran_bp_header.get_retransmission_count();
                    if (tran_bp_header.get_bundle_type() == BundleType::AntiPacket) {
                        neighbor_info_vec_[j].info_sent_ap_seqno_vec_.push_back(tran_bp_header.get_source_seqno());
                        neighbor_info_vec_[j].info_sent_ap_time_vec_.push_back(Simulator::Now().GetSeconds());
                    } else {
                        neighbor_info_vec_[j].info_sent_bp_seqno_vec_.push_back(tran_bp_header.get_source_seqno());
                    }
                }
                NS_LOG_DEBUG(LogPrefixMacro << "before SocketSendDetail,tran_p_pkt.size()=" << tran_p_pkt->GetSize() << ";transmit ip=" << neighbor_info_vec_[j].info_address_.GetIpv4() << ";tran_bp_header : " << tran_bp_header);
                if (!SocketSendDetail(tran_p_pkt, 0, neighbor_info_vec_[j].info_address_)) {
                    NS_LOG_ERROR("SocketSendDetail fail");
                    std::abort();
                }
            }
        }

        /*
         * Aim : A RoutingMethod switcher
         * TODO
         * define your decision method
         */
        bool DtnApp::FindTheNeighborThisBPHeaderTo(BPHeader& ref_bp_header, int& return_index_of_neighbor_you_dedicate, DtnApp::CheckState check_state) {
            NS_LOG_INFO(LogPrefixMacro << "enter FindTheNeighborThisBPHeaderTo()");
            if (routing_assister_.IsSet() && routing_assister_.get_rm() == RoutingMethod::SprayAndWait) {
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
                int s, d, indx = -1, result;
                {
                    // init s, d
                    auto ip_s = ref_bp_header.get_source_ip();
                    auto ip_d = ref_bp_header.get_destination_ip();
                    if (ip_d == own_ip_) {return false;}
                    s = Ipv42NodeNo(ip_s);
                    d = Ipv42NodeNo(ip_d);
                }
                NS_LOG_DEBUG(LogPrefixMacro << "NOTE: before YouRouting method");
                result = routing_assister_.RouteIt(s, d);
                if (result == node_->GetId()) {NS_LOG_ERROR(LogPrefixMacro << "Error: routing self! s=" << s << ";d=" << d << ";result = " << result);}
                NS_LOG_DEBUG(LogPrefixMacro << "NOTE: after YouRouting method, result =" << result);
                if (result != -1) {
                    auto ip_base = NodeNo2Ipv4(result);
                    for (int i = 0; i < neighbor_info_vec_.size(); i++) {
                        auto nip = neighbor_info_vec_[i].info_address_.GetIpv4();
                        if (nip.IsEqual(ip_base)) { indx = i; break; }
                    }
                    vector<int> available = BPHeaderBasedSendDecisionDetail(ref_bp_header, check_state);
                    for (auto v : available) {
                        if (v == indx) {
                            return_index_of_neighbor_you_dedicate = indx;
                            NS_LOG_DEBUG(LogPrefixMacro << "NOTE:your decision is made, to-node-id =" << result << "; index of neighbor_info_vec_ is = " << indx);
                            return true;
                        }
                    }
                    NS_LOG_INFO(LogPrefixMacro << "routing decision is not available, or have be sent;" << " to-node-id =" << result << "; index of neighbor_info_vec_ is = " << indx << "all available is: ");
                    if (available.size() == 0) {
                        NS_LOG_WARN(LogPrefixMacro << "WARN: available is none.");
                        return false;
                    }
                    return_index_of_neighbor_you_dedicate = available[0];
                    return true;
                } else {
                    return false;
                }
            } else {
                NS_LOG_ERROR("can't fine the routing method or method not assigned, routing_assister_ is set=" << routing_assister_.IsSet());
                std::abort();
            }
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
                    << "\ndaemon_retransmission_packet_buffer_vec_.size()=" <<daemon_retransmission_packet_buffer_vec_.size()
                    << "\ndaemon_transmission_info_vec_.size()=" <<daemon_transmission_info_vec_.size()
                    << "\ndaemon_transmission_bh_info_vec_.size()=" <<daemon_transmission_bh_info_vec_.size()
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
                    daemon_retransmission_packet_buffer_vec_.size() +
                    daemon_reception_info_vec_.size() +
                    neighbor_info_vec_.size() +
                    daemon_transmission_info_vec_.size() +
                    daemon_transmission_bh_info_vec_.size() < 400) {
            } else {
                NS_LOG_ERROR(LogPrefixMacro << "ERROR: queue and vecotr too big");
                std::abort();
            }
            //if (Simulator::Now().GetSeconds() > 150) { std::abort(); }
            Simulator::Schedule(Seconds(10), &DtnApp::StateCheckDetail, this);
        }

        /*
         *  real send flags == 1
         * */
        bool DtnApp::SprayGoodDetail(BPHeader bp_header, int flag) {
            int v = bp_header.get_source_seqno();
            if (flag == 1) {bp_header.set_hop_time_stamp(Simulator::Now());}
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
            NS_LOG_INFO(LogPrefixMacro << "enter check buffer()");
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
                NS_LOG_LOGIC(LogPrefix() << "is stateidle");
                if (check_state == CheckState::State_2) {
                    // go through daemon_antipacket_queue_ to find whether real_send_boolean should be set true
                    for (int n = 0; n < daemon_antipacket_queue_->GetNPackets() && !real_send_boolean; n++) {
                        p_pkt = daemon_antipacket_queue_->Dequeue()->GetPacket();
                        p_pkt->RemoveHeader(bp_header);
                        if ((SprayGoodDetail(bp_header, 0)) && (Simulator::Now().GetSeconds() - bp_header.get_hop_time_stamp().GetSeconds() > 2 * NS3DTNBIT_HELLO_BUNDLE_INTERVAL_TIME)) {
                            real_send_boolean = FindTheNeighborThisBPHeaderTo(bp_header, decision_neighbor, check_state);
                        }
                        p_pkt->AddHeader(bp_header);
                        Ptr<Packet> p_pkt_copy = p_pkt->Copy();
                        daemon_antipacket_queue_->Enqueue(Packet2Queueit(p_pkt_copy));
                    }
                } else if (check_state == CheckState::State_1){
                    // go though daemon_bundle_queue_ to find whether real_send_boolean should be set true
                    for (int n = 0; n < daemon_bundle_queue_->GetNPackets() && !real_send_boolean; n++) {
                        p_pkt = daemon_bundle_queue_->Dequeue()->GetPacket();
                        p_pkt->RemoveHeader(bp_header);
                        if (Simulator::Now().GetSeconds() - bp_header.get_src_time_stamp().GetSeconds() < NS3DTNBIT_HYPOTHETIC_BUNDLE_EXPIRED_TIME) {
                            if ((SprayGoodDetail(bp_header, 0)) && Simulator::Now().GetSeconds() - bp_header.get_hop_time_stamp().GetSeconds() > 2 * NS3DTNBIT_HELLO_BUNDLE_INTERVAL_TIME) {
                                real_send_boolean = FindTheNeighborThisBPHeaderTo(bp_header, decision_neighbor, check_state);
                            }
                        } else {
                            NS_LOG_WARN(LogPrefixMacro << "WARN:expired pkt, may happed, may error");
                            if (bp_header.get_retransmission_count() < NS3DTNBIT_MAX_TRANSMISSION_TIMES) {
                                // rearrange outdate packet, do nothing about real_send_boolean 
                                if (bp_header.get_hop_count() == 0) {
                                    // this bundle had not been sent out, so we can reset time for it and then re-enqueue for further retransmit
                                    bp_header.set_src_time_stamp(Simulator::Now());
                                    bp_header.set_retransmission_count(bp_header.get_retransmission_count() + 1);
                                    for (int j = 0; j < neighbor_info_vec_.size(); j++) {
                                        for (int m = 0; m < neighbor_info_vec_[j].info_sent_bp_seqno_vec_.size(); m++) {
                                            if (neighbor_info_vec_[j].info_sent_bp_seqno_vec_[m] == bp_header.get_source_seqno()) {
                                                // we need to rollback the record of 'send this bundle'
                                                UpdateNeighborInfoDetail(1, j, m);
                                                break;
                                            }
                                        }
                                    }
                                } else { 
                                    NS_LOG_ERROR(LogPrefixMacro << "ERROR:solve the droped expired bundle, not implement yet!"); std::abort();
                                }
                            } else { 
                                NS_LOG_ERROR(LogPrefixMacro << "ERROR:solve the droped expired bundle, not implement yet!"); std::abort(); 
                            }
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
                for (auto ele : daemon_transmission_bh_info_vec_) {
                    if (ele == tmp_header_info) {
                        transmist_session_already = true;
                    }
                }
                if (transmist_session_already) {
                    BPHeader bbh;
                    p_pkt->RemoveHeader(bbh);
                    NS_LOG_INFO(LogPrefixMacro << "transmit-session already exist, head = " << bbh);
                    p_pkt->AddHeader(bbh);
                } else {
                    NS_LOG_INFO(LogPrefixMacro << "transmission session Enqueue");
                    daemon_transmission_bh_info_vec_.push_back(tmp_header_info);
                    daemon_transmission_info_vec_.push_back(tmp_transmission_info);
                    daemon_retransmission_packet_buffer_vec_.push_back(p_pkt->Copy());
                }
                ToTransmit(tmp_header_info, false);
            }
            CheckBufferSwitchStateDetail(real_send_boolean, check_state);
        }

        /*
         * Aim : switch state of app
         * */
        void DtnApp::CheckBufferSwitchStateDetail(bool real_send_boolean, DtnApp::CheckState check_state) {
            NS_LOG_INFO(LogPrefixMacro << "enter CheckBufferSwitchStateDetail()");
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
                                                   Simulator::Schedule(Seconds(3 * NS3DTNBIT_BUFFER_CHECK_INTERVAL), &DtnApp::CheckBuffer, this, CheckState::State_2);
                                               }
                                               break;
                                           }
                case CheckState::State_2 : {
                                               if (real_send_boolean) {
                                                   Simulator::Schedule(Seconds(NS3DTNBIT_BUFFER_CHECK_INTERVAL), &DtnApp::CheckBuffer, this, CheckState::State_2);
                                               } else {
                                                   Simulator::Schedule(Seconds(3 * NS3DTNBIT_BUFFER_CHECK_INTERVAL), &DtnApp::CheckBuffer, this, CheckState::State_1);
                                               }
                                               break;
                                           }
                default : {break;}
            }
        }

        /* refine
         * Aim :
         * This is a routing method, just find all avilables.
         * Detail :
         * handle normal bundle and antipacket send decision
         * bundle_sent if logic included by transmit_session_already 
         */
        vector<int> DtnApp::BPHeaderBasedSendDecisionDetail(BPHeader& ref_bp_header, enum CheckState check_state) {
            NS_LOG_INFO(LogPrefixMacro << "enter BPHeaderBasedSendDecisionDetail()");
            vector<int> result;
            switch (ref_bp_header.get_bundle_type()) {
                case BundleType::BundlePacket : { 
                                                    Ipv4Address dst_ip = ref_bp_header.get_destination_ip();
                                                    for (int j = 0; j < neighbor_info_vec_.size(); j++) {
                                                        bool nei_last_seen_bool = Simulator::Now().GetSeconds() - neighbor_info_vec_[j].info_last_seen_time_ < NS3DTNBIT_HELLO_BUNDLE_INTERVAL_TIME * 2;
                                                        bool nei_have_space = neighbor_info_vec_[j].info_daemon_baq_available_bytes_ > ref_bp_header.get_payload_size() + ref_bp_header.GetSerializedSize();
                                                        //bool nei_is_dest = dst_ip.IsEqual(neighbor_info_vec_[j].info_address_.GetIpv4());
                                                        bool nei_is_source = neighbor_info_vec_[j].info_address_.GetIpv4().IsEqual(ref_bp_header.get_source_ip());
                                                        if (nei_is_source || !nei_have_space || !nei_last_seen_bool) { break; }
                                                        // 'tranmit_session_already' is true, means that totransmit() has been called and shouldn't init a now one.
                                                        // 'neighbor_has_bundle is false' maens that neighbor do not carry this bundle
                                                        // 'bundle_sent is false' means that you haven't send this bundle to this neighbor
                                                        bool transmit_session_already = false, neighbor_has_bundle = false, bundle_sent = false;
                                                        for (int i = 0; i < daemon_transmission_bh_info_vec_.size(); i++) {
                                                            auto no1 = daemon_transmission_bh_info_vec_[i].info_source_seqno_;
                                                            auto no2 = ref_bp_header.get_source_seqno();
                                                            auto ip1 = daemon_transmission_bh_info_vec_[i].info_transmit_addr_.GetIpv4();
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
                                                        for (int m = 0; (!transmit_session_already) && (!neighbor_has_bundle) && (m < neighbor_info_vec_[j].info_sent_bp_seqno_vec_.size()); m++) {
                                                            if (neighbor_info_vec_[j].info_sent_bp_seqno_vec_[m] == ref_bp_header.get_source_seqno()) {
                                                                bundle_sent = true;
                                                                break;
                                                            }
                                                        }
                                                        if ((!transmit_session_already) && (!neighbor_has_bundle) && (!bundle_sent)) {
                                                            NS_LOG_DEBUG(LogPrefixMacro << "NOTE: neighbor" << j << ", already,has,sent is :" << transmit_session_already << " " << neighbor_has_bundle << " " << bundle_sent);
                                                            result.push_back(j);
                                                        } else {
                                                            NS_LOG_INFO(LogPrefixMacro << "neighbor " << j << "didn't available" << ", already,has,sent is :" << transmit_session_already << " " << neighbor_has_bundle << " " << bundle_sent);
                                                        }
                                                    }
                                                    break;
                                                }
                case BundleType::AntiPacket : {
                                                  for (int j = 0; j < neighbor_info_vec_.size(); j++) {
                                                      // neighbor already has this antipacket or this antipacket has been sent to this neighbor else
                                                      bool neighbor_has_bundle = false, anti_pkt_sent = false; 
                                                      for (int m = 0; (!neighbor_has_bundle) && (m < neighbor_info_vec_[j].info_baq_seqno_vec_.size()); m++) {
                                                          if (neighbor_info_vec_[j].info_baq_seqno_vec_[m] == ref_bp_header.get_source_seqno()) {
                                                              neighbor_has_bundle = true;
                                                              break;
                                                          } 
                                                      }
                                                      for (int m = 0; (!anti_pkt_sent) && (!neighbor_has_bundle) && m < neighbor_info_vec_[j].info_sent_ap_seqno_vec_.size(); m++) {
                                                          if (neighbor_info_vec_[j].info_sent_ap_seqno_vec_[m] == ref_bp_header.get_source_seqno() && (Simulator::Now().GetSeconds() - neighbor_info_vec_[j].info_sent_ap_time_vec_[m] > NS3DTNBIT_BUFFER_CHECK_INTERVAL * 2)) {
                                                              anti_pkt_sent = true;
                                                              break;
                                                          }
                                                      }
                                                      if (neighbor_has_bundle == false && anti_pkt_sent == false) {
                                                          NS_LOG_DEBUG(LogPrefixMacro << "NOTE: anti decision made, has, sent = " << neighbor_has_bundle << " " << anti_pkt_sent);
                                                          result.push_back(j);
                                                      }
                                                  }
                                                  break;
                                              }
                default :   break;
            }
            return result;
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
        }

        /* refine 
         * this func would be invoked only in ReceiveBundle()
         * it would find the pkt in daemon_bundle_queue_, then dequeue it 
         * and update neighbor_sent_bp_seqno_vec_ & daemon_transmission_bh_info_vec_
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
                    for (; index < daemon_transmission_bh_info_vec_.size(); index++) {
                        if (daemon_transmission_bh_info_vec_[index].info_source_seqno_ == bp_header.get_source_seqno()) {
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
                if (((Simulator::Now().GetSeconds() - bp_header.get_src_time_stamp()) < NS3DTNBIT_HYPOTHETIC_BUNDLE_EXPIRED_TIME) || (bp_header.get_hop_count() == 0)) {
                    //if not expired
                    p_pkt->AddHeader(bp_header);
                    daemon_bundle_queue_->Enqueue(Packet2Queueit(p_pkt));//Enqueue(p_pkt);
                } else {
                    for (int j = 0; j < neighbor_info_vec_.size(); j++) {
                        int m = 0;
                        bool found_expired = false;
                        while (m < NS3DTNBIT_HYPOTHETIC_NEIGHBOR_BAQ_NUMBER_MAX && found_expired == 0) {
                            // hop count equals zero means that it came from a neighbor
                            if (bp_header.get_source_seqno() == neighbor_info_vec_[j].info_sent_bp_seqno_vec_[m]) {
                                found_expired = true;
                            } else {
                                m++;
                            }
                            if (found_expired) {
                                NS_LOG_WARN(LogPrefixMacro << "WARN:expired, may happen");
                                UpdateNeighborInfoDetail(1, n, j);
                            }
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
                        int k = 0;
                        bool found_expired = false;
                        while (k < NS3DTNBIT_HYPOTHETIC_NEIGHBOR_BAQ_NUMBER_MAX && found_expired == false) {
                            if (bp_header.get_source_seqno() == - neighbor_info_vec_[j].info_sent_ap_seqno_vec_[k]) {
                                found_expired = true;
                                k++;
                            }
                            if (found_expired) {
                                NS_LOG_WARN(LogPrefixMacro << "WARN:expired, may happen");
                                UpdateNeighborInfoDetail(2, j, k);
                                UpdateNeighborInfoDetail(3, j, k);
                            }
                        }
                    }
                }
            }
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
            p_bp_header->set_spray(2);
            p_bp_header->set_retransmission_count(0);
            p_bp_header->set_src_time_stamp(Simulator::Now());
            p_bp_header->set_hop_time_stamp(Simulator::Now());
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
    } /* ns3dtnbit */ 
    /* ... */
}
