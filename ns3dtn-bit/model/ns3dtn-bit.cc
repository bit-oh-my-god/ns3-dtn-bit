/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3dtn-bit.h"


namespace ns3 {
    // https://groups.google.com/forum/#!topic/ns-3-users/NyrxsLGzBxw
#define NS_LOG_APPEND_CONTEXT \
    Ptr<Node> node = GetObject<Node> (); \
    if ( node  )      { std::clog << Simulator::Now ().GetSeconds () << " [node " << node->GetId () << "] "; \
    } \
    else { std::clog << Simulator::Now ().GetSeconds () << " [node -] ";  }

    NS_LOG_COMPONENT_DEFINE ("DtnRunningLog");
    namespace ns3dtnbit {

        /* we define this would rewrite ( you can say shade ) the dufault CourseChange
         * and this rewrite is supposed in tutorial
         */
        static void CourseChanged(std::ostream *myos, std::string somethingImusthavetobecallableinthistracefunctionwhichisnotuseful_fuck_, Ptr<const MobilityModel> mobility)
        {
            Ptr<Node> node = mobility->GetObject<Node> ();
            Vector pos = mobility->GetPosition (); // Get position
            Vector vel = mobility->GetVelocity (); // Get velocity

            std::cout.precision(5);
            *myos << "Simulation Time : " << Simulator::Now () 
                << "; NODE: " << node->GetId() 
                << "; POS: x=" << pos.x << ", y=" << pos.y << ", z=" << pos.z 
                << "; VEL: x=" << vel.x << ", y=" << vel.y << ", z=" << vel.z 
                << std::endl;
        }

        /*
         * Can't use CreateObject<>, so do it myself
         */
        static Ptr<QueueItem> Packet2Queueit(Ptr<Packet> p_pkt) {
            QueueItem* p = new QueueItem(p_pkt);
            return Ptr<QueueItem>(p);
        }
#ifdef DEBUG
        // Important !! use std::cerr std::cout for app, example and script
        // NS_LOG is not easy to use.
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

        DtnApp::DtnApp() {

        }

        DtnApp::~DtnApp() {

        }

        DtnExampleInterface::DtnExampleInterface() {
            // add some default value for settings in Configure() call
            random_seed_ = 214127;
            node_number_ = 20 ;
            simulation_duration_ = 600;
            pcap_boolean_ = false;
            print_route_boolean_ = false;
            // TODO, make trace_file_ to be relative path
            trace_file_ = "/home/dtn-012345/ns-3_build/ns3-dtn-bit/box/current_trace/current_trace.tcl";
            log_file_ = "~/ns-3_build/ns3-dtn-bit/box/dtn_simulation_result/dtn_trace_log.txt";
        }

        DtnExampleInterface::DtnExampleInterface(DtnExampleInterface&& rh) {

        }

        

        DtnExample::DtnExample() {
            // add some default value for settings in Configure() call
            random_seed_ = 214127;
            node_number_ = 20 ;
            simulation_duration_ = 600;
            pcap_boolean_ = false;
            print_route_boolean_ = false;
            // TODO, make trace_file_ to be relative path
            trace_file_ = "/home/dtn-012345/ns-3_build/ns3-dtn-bit/box/current_trace/current_trace.tcl";
            log_file_ = "~/ns-3_build/ns3-dtn-bit/box/dtn_simulation_result/dtn_trace_log.txt";
        }

        /* refine
        */
        void DtnApp::SetUp(Ptr<Node> node) {
            node_ = node;
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
        */
        void DtnApp::ToSendHello(Ptr<Socket> socket, dtn_time_t simulation_end_time, Time hello_interval, bool hello_right_now_boolean) {
            Ptr<Packet> p_pkt; 
            BPHeader bp_header;
            if (hello_right_now_boolean) {
                if (Simulator::Now().GetSeconds() > simulation_end_time) {
                    daemon_socket_handle_->Close();
                } else {
                    std::stringstream msg;
                    do {
                        // prepare 'msg' to write into hello bundle
                        char tmp_msg_00[1024] = "";
                        if (congestion_control_method_ == CongestionControlMethod::DynamicControl) {
                            if ((drops_count_ == 0) && (congestion_control_parameter_ < 0.9)) {
                                congestion_control_parameter_ += 0.01;
                            } else {
                                if ((drops_count_ > 0) && (congestion_control_parameter_ > 0.5)) {
                                    congestion_control_parameter_ *= 0.8;
                                } 
                                drops_count_ = 0;
                            }
                        }
                        int32_t tmp_number = ((daemon_bundle_queue_->GetNBytes() + daemon_antipacket_queue_->GetNBytes()) - (dtn_seqno_t)(congestion_control_parameter_ * daemon_baq_bytes_max_));
                        if (tmp_number <= 0) {
                            sprintf(tmp_msg_00, "%d", 0);
                        } else {
                            sprintf(tmp_msg_00, "%d", tmp_number);
                        }
                        msg << tmp_msg_00;
                        PeriodReorderDaemonBundleQueueDetail();
                        char tmp_msg_01[1024] = "";
                        int pkts = daemon_bundle_queue_->GetNPackets();
                        int anti_pkts = daemon_antipacket_queue_->GetNPackets();
                        sprintf(tmp_msg_01, "%d", pkts);
                        for (int i = 0; i < pkts; i++) {
                            p_pkt = daemon_bundle_queue_->Dequeue()->GetPacket();
                            if (msg.str().length() < NS3DTNBIT_HELLO_BUNDLE_SIZE_MAX) {
                                BPHeader bp_header;
                                p_pkt->RemoveHeader(bp_header);
                                dtn_seqno_t tmp_seqno = bp_header.get_source_seqno();
                                char tmp_msg_02[1024] = "";
                                sprintf(tmp_msg_02, "%d", tmp_seqno);
                                msg << tmp_msg_02;
                                p_pkt->AddHeader(bp_header);
                            } else {
                                // ERROR LOG
                                // 'at time, too big hello bundle'
                                std::cerr << GetLogStr("ERROR") << " ====== too big hello, time: " << Simulator::Now().GetSeconds() << endl;
                            }
                            daemon_bundle_queue_->Enqueue(Packet2Queueit(p_pkt));
                        }
                        for (int i = 0; i < anti_pkts; i++) {
                            p_pkt = daemon_antipacket_queue_->Dequeue()->GetPacket();
                            if (msg.str().length() < NS3DTNBIT_HELLO_BUNDLE_SIZE_MAX) {
                                p_pkt->RemoveHeader(bp_header);
                                dtn_seqno_t tmp_seqno = bp_header.get_source_seqno();
                                char tmp_msg_03[1024] = "";
                                sprintf(tmp_msg_03, "%d", tmp_seqno);
                                msg << tmp_msg_03;
                                p_pkt->AddHeader(bp_header);
                            } else {
                                // ERROR LOG
                                std::cerr << GetLogStr("ERROR") << " ====== too big hello, time: " << Simulator::Now().GetSeconds() << endl;
                            }
                            daemon_antipacket_queue_->Enqueue(Packet2Queueit(p_pkt));
                        }
                    } while (0);
                    // this call would use msg.str() as the 'hello content'
                    CreateHelloBundleAndSendDetail(msg.str(), socket);
                    Simulator::Schedule(Seconds(NS3DTNBIT_HELLO_BUNDLE_INTERVAL_TIME), &DtnApp::ToSendHello, this, socket, simulation_end_time, Seconds(NS3DTNBIT_HELLO_BUNDLE_INTERVAL_TIME), true);
                }
            } else {
                Simulator::Schedule(hello_interval, &DtnApp::ToSendHello, this, socket, simulation_end_time, hello_interval, true);
            }
        }

        /* refine 
        */
        void DtnApp::ReceiveHello(Ptr<Socket> socket_handle) {
            Ptr<Packet> p_pkt;
            BPHeader bp_header;
            Address from_addr;
            while (p_pkt = socket_handle->RecvFrom(from_addr)) {
                InetSocketAddress addr = InetSocketAddress::ConvertFrom(from_addr);
                bool neighbor_found = false;
                int j = 0;
                for (; j < neighbor_info_vec_.size(); j++) {
                    neighbor_found = neighbor_info_vec_[j].info_address_ == addr ? true : false;
                }
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
                neighbor_info_vec_[j].info_baq_seqno_vec_ = vector<dtn_seqno_t>(NS3DTNBIT_HYPOTHETIC_NEIGHBOR_BAQ_NUMBER_MAX, 0);
                std::stringstream pkt_str_stream;
                p_pkt->RemoveHeader(bp_header);
                p_pkt->CopyData(&pkt_str_stream, bp_header.get_payload_size());
                do {
                    // parse raw content of pkt and update 'neighbor_info_vec_'
                    pkt_str_stream >> neighbor_info_vec_[j].info_daemon_baq_available_bytes_;
                    int pkt_seqno_number;
                    // the rest seqno is the seqno for antipacket
                    pkt_str_stream >> pkt_seqno_number;
                    int m = 0;
                    for (; m < pkt_seqno_number; m++) {
                        pkt_str_stream >> neighbor_info_vec_[j].info_baq_seqno_vec_[m];
                    }
                    // check whether 'stringstream' has sth to read
                    while (pkt_str_stream.rdbuf()->in_avail() != 0) {
                        dtn_seqno_t tmp_antipacket_seqno;
                        pkt_str_stream >> tmp_antipacket_seqno;
                        // we need negative to indicate this seqno is from antipacket
                        neighbor_info_vec_[j].info_baq_seqno_vec_[m++] = tmp_antipacket_seqno;
                        bool anti_found = false;
                        for (int k = 0; ((k < NS3DTNBIT_HYPOTHETIC_NEIGHBOR_BAQ_NUMBER_MAX) && (anti_found)); k++) {
                            anti_found = neighbor_info_vec_[j].info_sent_ap_seqno_vec_[k] == tmp_antipacket_seqno ? true : false;
                            if (anti_found) {
                                UpdateNeighborInfoDetail(3, j, k);
                                UpdateNeighborInfoDetail(2, j, k);
                            }
                        }
                    }
                } while (0);
            }
        }

        /* refine 
        */
        void DtnApp::ToSendAck(BPHeader& ref_bp_header, Ipv4Address response_ip) {
            std::string tmp_payload_str;
            do {
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
            } while (0);
            Ptr<Packet> p_pkt = Create<Packet>(tmp_payload_str.c_str(), tmp_payload_str.size());
            BPHeader bp_header;
            do {
                // fill up bp_header
                SemiFillBPHeaderDetail(&bp_header);
                bp_header.set_bundle_type(TransmissionAck);
                bp_header.set_destination_ip(response_ip);
                bp_header.set_source_seqno(p_pkt->GetUid());
                bp_header.set_payload_size(tmp_payload_str.size());
                bp_header.set_offset(tmp_payload_str.size());
            } while (0);
            p_pkt->AddHeader(bp_header);
            InetSocketAddress response_addr = InetSocketAddress(response_ip, NS3DTNBIT_PORT_NUMBER);
            SocketSendDetail(p_pkt, 0, response_addr);
        };

        /* refine 
         * 
         */
        void DtnApp::ToSendBundle(uint32_t dstnode_number, uint32_t payload_size) {
            std::string tmp_payload_str;
            do {
                // fill up payload 
                tmp_payload_str = "just_one_content_no_meaning";
            } while (0);
            Ptr<Packet> p_pkt = Create<Packet>(tmp_payload_str.c_str(), tmp_payload_str.size());
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
                bp_header.set_offset(tmp_payload_str.size());
            } while (0);
            p_pkt->AddHeader(bp_header);
            if ((daemon_antipacket_queue_->GetNBytes() + daemon_bundle_queue_->GetNBytes() + p_pkt->GetSize() <= daemon_baq_bytes_max_)) {
                daemon_bundle_queue_->Enqueue(Packet2Queueit(p_pkt));
                // NORMAL LOG
                std::cout << GetLogStr("LOG") << "====== to send bundle" << std::endl;
            } else {
                // ERROR LOG
                std::cerr << GetLogStr("ERROR") << "======  bundle to be sent is too bit" << std::endl;
            }
        }

        /* refine 
         * create and fill up antipacket-bundle then enqueue
         */
        void DtnApp::ToSendAntipacketBundle(BPHeader& ref_bp_header) {
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
                msg << ref_bp_header.get_source_seqno();
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
                bp_header.set_offset(anti_packet_payload_str.size());
            } while (0);
            p_pkt->AddHeader(bp_header);
            daemon_antipacket_queue_->Enqueue(Packet2Queueit(p_pkt));
            do {
                // LOG 
                std::cout << GetLogStr("LOG") << "====== send anti pckt bundle";
            } while (0);
        }

        /* refine 
         *     'ack bundle' : call 'ToTransmit' to transmit more(if have more) to the 'ack sender'
         *     'not ack bundle' : send a 'ack bundle', then get the information in that pkt
         *         'hello packet'
         *         'normal bundle'
         *         'antipacket bundle'
         * use daemon_reception_info_vec_ and daemon_reception_packet_buffer_vec_
         */
        void DtnApp::ReceiveBundle(Ptr<Socket> socket) {
            Address own_addr;
            socket->GetSockName(own_addr);
            own_ip_ = Ipv4Address::ConvertFrom(own_addr);
            while (socket->GetRxAvailable() > 0) {
                Address from_addr;
                Ptr<Packet> p_pkt = socket->RecvFrom(from_addr);
                BPHeader bp_header;
                p_pkt->RemoveHeader(bp_header);
                InetSocketAddress from_s_addr = InetSocketAddress::ConvertFrom(from_addr);
                Ipv4Address from_ip = Ipv4Address::ConvertFrom(from_addr);
                if (bp_header.get_bundle_type() == TransmissionAck) {
                    // if is 'ack' update state, get ack src-dst seqno, timestamp, bytes. then call 'ToTransmit' to transmit more
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
                    int k = 0;
                    for (; k < daemon_transmission_info_vec_.size(); k++) {
                        if (tmp_bh_info == daemon_transmission_bh_info_vec_[k]) { break; }
                    }
                    daemon_transmission_info_vec_[k].info_transmission_current_sent_acked_bytes_ += 
                        daemon_transmission_info_vec_[k].info_transmission_bundle_last_sent_bytes_;
                    ToTransmit(tmp_bh_info, false);
                } else {
                    // if not, send 'ack'
                    ToSendAck(bp_header, from_ip);
                }
                do {
                    // process the receiving pkt, then shrift state
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
                    if (tmp_recept_info.info_bundle_type_ == AntiPacket) {
                        // antipacket must not be fragment, it's safe to directly process
                        // keep antipacket and remove the bundle 'corresponded to'
                        BPHeader tmp_bp_header = bp_header;
                        p_pkt->AddHeader(bp_header);
                        if (!IsDuplicatedDetail(tmp_bp_header)) {
                            daemon_antipacket_queue_->Enqueue(Packet2Queueit(p_pkt->Copy()));
                            RemoveBundleFromAntiDetail(p_pkt);
                        }
                    } else if (tmp_recept_info.info_bundle_type_ == BundlePacket) {
                        bool reception_info_found = false;
                        int k = 0;
                        for (; k < daemon_reception_info_vec_.size(); k++) {
                            if (tmp_recept_info.info_bundle_source_ip_ == daemon_reception_info_vec_[k].info_bundle_source_ip_ 
                                    && tmp_recept_info.info_bundle_seqno_ == daemon_reception_info_vec_[k].info_bundle_seqno_
                                    && tmp_recept_info.info_trasmission_receive_from_ip_ == daemon_reception_info_vec_[k].info_trasmission_receive_from_ip_) {
                                reception_info_found = true;
                                break;
                            }
                        }
                        if (reception_info_found) {
                            // is recorded, keep receiving and check order of fragment, add new fragment to 'daemon_reception_packet_buffer_vec_' when all fragment received, parse this packet, deal with it
                            p_pkt->AddHeader(bp_header);
                            daemon_reception_info_vec_[k].info_fragment_pkt_pointer_vec_.push_back(p_pkt);
                            FragmentReassembleDetail(k);
                        } else {
                            // not recorded
                            daemon_reception_info_vec_.push_back(tmp_recept_info);
                            p_pkt->AddHeader(bp_header);
                            daemon_reception_packet_buffer_vec_.push_back(p_pkt);
                        }
                        BundleReceptionTailWorkDetail();
                    } else {} // later usage
                } while (0);
            }
        }

        /* refine 
         * check whether all fragment received
         * use AddAtEnd
         * set hop time
         */
        void DtnApp::FragmentReassembleDetail(int k) {
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
         * update state 
         */
        void DtnApp::BundleReceptionTailWorkDetail() {
            auto it = daemon_reception_packet_buffer_vec_.rbegin();
            Ptr<Packet> tmp_p_pkt = (*it)->Copy();
            BPHeader bp_header;
            tmp_p_pkt->RemoveHeader(bp_header);
            if (bp_header.get_payload_size() == tmp_p_pkt->GetSize()) {
                // this bundle is non-fragment or reassemble one
                if (bp_header.get_destination_ip() == own_ip_) {
                    ToSendAntipacketBundle(bp_header);
                    tmp_p_pkt->AddHeader(bp_header);
                    daemon_consume_bundle_queue_->Enqueue(Packet2Queueit(tmp_p_pkt->Copy()));
                } else {
                    tmp_p_pkt->AddHeader(bp_header);
                    daemon_bundle_queue_->Enqueue(Packet2Queueit(tmp_p_pkt->Copy()));
                }
            }
        }

        /* refine 
         * use bh_info as key get all msg, 
         * check whether it's able to call 'SocketSendDetail'
         * handle retransmit and transmit more
         * use daemon_retransmission_packet_buffer_vec_
         * note the packet size problem, size < NS3DTNBIT_HYPOTHETIC_TRANS_SIZE_FRAGMENT_MAX
         * update 'daemon_transmission_info_vec_'
         */
        void DtnApp::ToTransmit(DaemonBundleHeaderInfo bh_info, bool is_retransmit) {
            bool real_send_boolean = false;
            int index = 0, j = 0;
            for (; index < daemon_transmission_bh_info_vec_.size(); index++) {
                // find the index of the 'bundle to transmit' to 'daemon_transmission_info_vec_' using 'bh_info'
                if (daemon_transmission_bh_info_vec_[index] == bh_info) {
                    break;
                }
            }
            do {
                // check state, cancel transmission if condition
                if (daemon_transmission_info_vec_[index].info_transmission_total_send_bytes_ == daemon_transmission_info_vec_[index].info_transmission_current_sent_acked_bytes_) {
                    return;
                }
            } while (0);
            for (; j < neighbor_info_vec_.size(); j++) {
                // find the neighbor should be transmit, if this neighbor was not recently seen, schedule 'ToTransmit' later, otherwise, set real_send_boolean
                if (neighbor_info_vec_[j].info_address_ == bh_info.info_transmit_addr_) {
                    if (neighbor_info_vec_[j].info_last_seen_time_ > Simulator::Now().GetSeconds() - (NS3DTNBIT_HELLO_BUNDLE_INTERVAL_TIME * 3)) {
                        real_send_boolean = true;
                    } else {
                        Simulator::Schedule(Seconds(NS3DTNBIT_HELLO_BUNDLE_INTERVAL_TIME), &DtnApp::ToTransmit, this, bh_info, false);
                        return;
                    }
                    break;
                }
            }
            Ptr<Packet> tran_p_pkt;
            BPHeader tran_bp_header;
            int offset_value;
            if (real_send_boolean) {
                do {
                    // prepare bytes
                    int need_to_bytes = daemon_transmission_info_vec_[index].info_transmission_total_send_bytes_ - daemon_transmission_info_vec_[index].info_transmission_current_sent_acked_bytes_;
                    tran_p_pkt = daemon_retransmission_packet_buffer_vec_[index]->Copy(); 
                    tran_p_pkt->RemoveHeader(tran_bp_header);
                    tran_p_pkt->RemoveAtStart(daemon_transmission_info_vec_[index].info_transmission_current_sent_acked_bytes_);
                    if (need_to_bytes > NS3DTNBIT_HYPOTHETIC_TRANS_SIZE_FRAGMENT_MAX) {
                        tran_p_pkt->RemoveAtEnd(need_to_bytes - NS3DTNBIT_HYPOTHETIC_TRANS_SIZE_FRAGMENT_MAX);
                    }
                    tran_bp_header.set_offset(daemon_transmission_info_vec_[index].info_transmission_current_sent_acked_bytes_ + tran_p_pkt->GetSize());
                    if (is_retransmit) {
                        tran_bp_header.set_retransmission_count(tran_bp_header.get_retransmission_count() + 1);
                    }
                    offset_value = tran_p_pkt->GetSize() + daemon_transmission_info_vec_[index].info_transmission_current_sent_acked_bytes_;
                } while (0);
                do {
                    // update state
                    daemon_transmission_info_vec_[index].info_transmission_bundle_last_sent_time_ = Simulator::Now().GetSeconds();
                    daemon_transmission_info_vec_[index].info_transmission_bundle_last_sent_bytes_ = tran_p_pkt->GetSize();
                    daemon_transmission_bh_info_vec_[index].info_retransmission_count_ = tran_bp_header.get_retransmission_count();
                    if (tran_bp_header.get_bundle_type() == AntiPacket) {
                        neighbor_info_vec_[j].info_sent_ap_seqno_vec_.push_back(tran_bp_header.get_source_seqno());
                        neighbor_info_vec_[j].info_sent_ap_time_vec_.push_back(Simulator::Now().GetSeconds());
                    } else {
                        neighbor_info_vec_[j].info_sent_bp_seqno_vec_.push_back(tran_bp_header.get_source_seqno());
                    }
                } while (0);
                tran_bp_header.set_offset(offset_value);
                tran_p_pkt->AddHeader(tran_bp_header);
                SocketSendDetail(tran_p_pkt, 0, neighbor_info_vec_[j].info_address_);
                // Simulator::Schedule(Seconds(retransmission_interval_), &DtnApp::ToTransmit, this, bh_info, true);
            }
        }

        /* refine 
         * check your 'bundle queue' buffer and other related buffer periodly
         * make code refactory to this 
         * do check_state == CheckState::State_2 means that it was an Antipacket? and != 2 means that it was bundlepacket? yes, you can say.
         */
        void DtnApp::CheckBuffer(CheckState check_state) {
            // one time one pkt would be sent
            bool real_send_boolean = false;
            int decision_neighbor = 0;
            std::pair<int, int> real_send_info;
            Ptr<Packet> p_pkt;
            BPHeader bp_header;
            // remove expired antipackets and bundles
            if (check_state == CheckState::State_2) {
                RemoveExpiredBAQDetail();
            }
            // prepare and set real_send_boolean
            // being less than 2 means, wifi mac layer is not busy
            if (daemon_mac_queue_->GetNBytes() < 2) {
                if (check_state == CheckState::State_2) {
                    // go through daemon_antipacket_queue_ to find whether real_send_boolean should be set true
                    int pkt_number = daemon_antipacket_queue_->GetNPackets(), n = 0;
                    while ((n++ < pkt_number) && (!real_send_boolean)) {
                        p_pkt = daemon_antipacket_queue_->Dequeue()->GetPacket();
                        p_pkt->RemoveHeader(bp_header);
                        if ((Simulator::Now().GetSeconds() - bp_header.get_hop_time_stamp().GetSeconds() > 0.2)) {
                            real_send_boolean = BPHeaderBasedSendDecisionDetail(bp_header, decision_neighbor, check_state);
                        }
                        p_pkt->AddHeader(bp_header);
                        Ptr<Packet> p_pkt_copy = p_pkt->Copy();
                        daemon_antipacket_queue_->Enqueue(Packet2Queueit(p_pkt_copy));
                    }
                } else {
                    // go though daemon_bundle_queue_ to find whether real_send_boolean should be set true
                    int pkt_number = daemon_bundle_queue_->GetNPackets(), n = 0;
                    while ((n++ < pkt_number) && (!real_send_boolean)) {
                        p_pkt = daemon_bundle_queue_->Dequeue()->GetPacket();
                        p_pkt->RemoveHeader(bp_header);
                        if (Simulator::Now().GetSeconds() - bp_header.get_src_time_stamp().GetSeconds() < NS3DTNBIT_HYPOTHETIC_BUNDLE_EXPIRED_TIME) {
                            if (Simulator::Now().GetSeconds() - bp_header.get_src_time_stamp().GetSeconds() > 0.2) {
                                real_send_boolean = BPHeaderBasedSendDecisionDetail(bp_header, decision_neighbor, check_state);
                            }
                            p_pkt->AddHeader(bp_header);
                            Ptr<Packet> p_pkt_copy = p_pkt->Copy();
                            daemon_bundle_queue_->Enqueue(Packet2Queueit(p_pkt_copy));
                        } else {
                            // rearrange outdate packet, do nothing about real_send_boolean 
                            if (bp_header.get_retransmission_count() < NS3DTNBIT_MAX_TRANSMISSION_TIMES) {
                                if (bp_header.get_hop_count() == 0) {
                                    // this bundle had not been sent out, so we can reset time for it and then re-enqueue for further retransmit
                                    bp_header.set_src_time_stamp(Simulator::Now());
                                } else {}
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
                                p_pkt->AddHeader(bp_header);
                                daemon_bundle_queue_->Enqueue(Packet2Queueit(p_pkt));
                            }
                        }
                    }
                }
            } else {
                check_state = CheckState::State_0;
            }
            // do real send stuff
            if (real_send_boolean) {
                if (!daemon_socket_handle_) {
                    CreateSocketDetail();
                }
                int j = decision_neighbor;
                DaemonBundleHeaderInfo tmp_header_info = {
                    neighbor_info_vec_[j].info_address_,
                    bp_header.get_source_seqno(),
                    bp_header.get_retransmission_count()
                };
                daemon_transmission_bh_info_vec_.push_back(tmp_header_info);
                DaemonTransmissionInfo tmp_transmission_info = {
                    p_pkt->GetSize(),
                    std::min((uint32_t)NS3DTNBIT_HYPOTHETIC_TRANS_SIZE_FRAGMENT_MAX, p_pkt->GetSize()),
                    Simulator::Now().GetSeconds(),
                    Simulator::Now().GetSeconds(),
                    std::min((uint32_t)NS3DTNBIT_HYPOTHETIC_TRANS_SIZE_FRAGMENT_MAX, p_pkt->GetSize())
                };
                daemon_transmission_info_vec_.push_back(tmp_transmission_info);
                if (check_state != CheckState::State_2) {
                    daemon_flow_count_++;
                    //p_pkt->AddPacketTag(FlowIdTag(bp_header.get_source_seqno()));
                    //p_pkt->AddPacketTag(QosTag(bp_header.get_retransmission_count()));
                } else {
                    // to author's intention every sequence number of antipacket would be negative
                    //p_pkt->AddPacketTag(FlowIdTag(- (bp_header.get_source_seqno())));
                    //p_pkt->AddPacketTag(QosTag(4));
                }
                daemon_retransmission_packet_buffer_vec_.push_back(p_pkt->Copy());
                ToTransmit(tmp_header_info, false);
            }
            // switch check_state and reschedule
            switch (check_state) {
                case CheckState::State_0 : {
                                               if (real_send_boolean == 0) {Simulator::Schedule(Seconds(0.01), &DtnApp::CheckBuffer, this, CheckState::State_2);}
                                               else {Simulator::Schedule(Seconds(0.001), &DtnApp::CheckBuffer, this, CheckState::State_2);}
                                               break;
                                           }
                case CheckState::State_1 : {
                                               if (real_send_boolean == 0) {CheckBuffer(CheckState::State_0);}
                                               else {Simulator::Schedule(Seconds(0.001), &DtnApp::CheckBuffer, this, CheckState::State_2);}
                                               break;
                                           }
                case CheckState::State_2 : {
                                               if (real_send_boolean == 0) {CheckBuffer(CheckState::State_1);}
                                               else {Simulator::Schedule(Seconds(0.001), &DtnApp::CheckBuffer, this, CheckState::State_2);}
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
        bool DtnApp::BPHeaderBasedSendDecisionDetail(BPHeader& ref_bp_header, int& return_index_of_neighbor, enum CheckState check_state) {
            bool real_send_boolean = false;
            switch (ref_bp_header.get_bundle_type()) {
                case BundleType::BundlePacket : { 
                                                    Ipv4Address dst_ip = ref_bp_header.get_destination_ip();
                                                    uint32_t spray_value = ref_bp_header.get_spray();
                                                    for (int j = 0; (j < neighbor_info_vec_.size()) && (!real_send_boolean); j++) {
                                                        if (
                                                                (((check_state == CheckState::State_0) && (spray_value > 0) && ((congestion_control_method_ == CongestionControlMethod::NoControl) || (neighbor_info_vec_[j].info_daemon_baq_available_bytes_ > ref_bp_header.get_payload_size() + ref_bp_header.GetSerializedSize()))) || (dst_ip != neighbor_info_vec_[j].info_address_.GetIpv4()))
                                                                && ((Simulator::Now().GetSeconds() - neighbor_info_vec_[j].info_last_seen_time_) < 0.1) 
                                                                && (neighbor_info_vec_[j].info_address_.GetIpv4() != ref_bp_header.get_source_ip()) 
                                                           ) {
                                                            // 'neighbor_has_bundle is false' maens that neighbor do not carry this bundle
                                                            // 'bundle_sent is false' means that you haven't send this bundle to this neighbor
                                                            bool neighbor_has_bundle = false, bundle_sent = false;
                                                            for (int m = 0; (!neighbor_has_bundle) && (neighbor_info_vec_[j].info_baq_seqno_vec_.size() > m); m++) {
                                                                if (neighbor_info_vec_[j].info_baq_seqno_vec_[m] == ref_bp_header.get_source_seqno()) {
                                                                    neighbor_has_bundle = true;
                                                                }
                                                            }
                                                            for (int m = 0; (!neighbor_has_bundle) && (!bundle_sent) && (m < neighbor_info_vec_[j].info_sent_bp_seqno_vec_.size()); m++) {
                                                                if (neighbor_info_vec_[j].info_sent_bp_seqno_vec_[m] == ref_bp_header.get_source_seqno()) {
                                                                    bundle_sent = true;
                                                                }
                                                            }
                                                            if ((!neighbor_has_bundle) && (!bundle_sent)) {
                                                                real_send_boolean = true;
                                                                return_index_of_neighbor = j;
                                                                if (check_state == CheckState::State_0) {
                                                                    if (routing_method_ == RoutingMethod::SprayAndWait) {
                                                                        ref_bp_header.set_spray(ref_bp_header.get_spray() / 2);
                                                                    }
                                                                    if (congestion_control_method_ != CongestionControlMethod::NoControl) {
                                                                        if (ref_bp_header.get_payload_size() + ref_bp_header.GetSerializedSize()
                                                                                >= neighbor_info_vec_[j].info_daemon_baq_available_bytes_) {
                                                                            neighbor_info_vec_[j].info_daemon_baq_available_bytes_ = 0;
                                                                        } else {
                                                                            neighbor_info_vec_[j].info_daemon_baq_available_bytes_ -=
                                                                                ref_bp_header.get_payload_size() + ref_bp_header.GetSerializedSize();
                                                                        }
                                                                    }
                                                                } else {
                                                                    ref_bp_header.set_hop_time_stamp(Simulator::Now() + Seconds(5.0));
                                                                }
                                                            }
                                                        }
                                                    }
                                                    break;
                                                }
                case BundleType::AntiPacket : {
                                                  for (int j = 0; (j < neighbor_info_vec_.size()) && (real_send_boolean == false); j++) {
                                                      // neighbor already has this antipacket or this antipacket has been sent to this neighbor else
                                                      bool neighbor_has_bundle = false, anti_pkt_sent = false; 
                                                      for (int m = 0; (!neighbor_has_bundle) && (neighbor_info_vec_[j].info_baq_seqno_vec_.size() > m) && (m < 2 * NS3DTNBIT_HYPOTHETIC_NEIGHBOR_BAQ_NUMBER_MAX); m++) {
                                                          if (neighbor_info_vec_[j].info_baq_seqno_vec_[m] == -(dtn_seqno_t)ref_bp_header.get_source_seqno()) {
                                                              neighbor_has_bundle = true;
                                                          } 
                                                      }
                                                      for (int m = 0; (!anti_pkt_sent) && (!neighbor_has_bundle) && (neighbor_info_vec_[j].info_sent_ap_seqno_vec_.size() > m) && m < (NS3DTNBIT_HYPOTHETIC_NEIGHBOR_BAQ_NUMBER_MAX); m++) {
                                                          if (neighbor_info_vec_[j].info_sent_ap_seqno_vec_[m] == ref_bp_header.get_source_seqno() && (Simulator::Now().GetSeconds() - neighbor_info_vec_[j].info_sent_ap_time_vec_[m] < 1.5)) {
                                                              anti_pkt_sent = true;
                                                          }
                                                      }
                                                      if (neighbor_has_bundle == false && anti_pkt_sent == false) {
                                                          real_send_boolean = true;
                                                          return_index_of_neighbor = j;
                                                      }
                                                  }
                                                  break;
                                              }
                default :   break;
            }
            return real_send_boolean;
        }

        /* refine 
         * roll back
         */
        void DtnApp::UpdateNeighborInfoDetail(int which_info, int which_neighbor, int which_pkt_index) {
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
            BPHeader lhs_bp_header;
            p_anti_pkt->RemoveHeader(lhs_bp_header);
            int number = daemon_bundle_queue_->GetNPackets();
            std::stringstream lhs_ss;
            p_anti_pkt->CopyData(dynamic_cast<std::ostream *>(&lhs_ss), lhs_bp_header.get_payload_size());
            std::string source_ip, destination_ip;
            dtn_seqno_t lhs_seqno_value;
            lhs_ss >> source_ip >> destination_ip >> lhs_seqno_value;
            Ipv4Address lhs_source_ip(source_ip.c_str()), lhs_destination_ip(destination_ip.c_str());
            while (number--) {
                Ptr<Packet> rhs_p_pkt = daemon_bundle_queue_->Dequeue()->GetPacket();
                BPHeader rhs_bp_header;
                rhs_p_pkt->RemoveHeader(rhs_bp_header);
                if (lhs_source_ip == rhs_bp_header.get_source_ip() 
                        && lhs_destination_ip == rhs_bp_header.get_destination_ip() 
                        && lhs_seqno_value == rhs_bp_header.get_source_seqno()) {
                    break;
                } else {
                    rhs_p_pkt->AddHeader(rhs_bp_header);
                    daemon_bundle_queue_->Enqueue(Packet2Queueit(rhs_p_pkt));//Enqueue(rhs_p_pkt);
                }
            }
            p_anti_pkt->AddHeader(lhs_bp_header);
        }

        /* refine 
         * check whether one packet is already in queue
         */
        bool DtnApp::IsDuplicatedDetail(BPHeader& bp_header) {
            if (bp_header.get_bundle_type() == BundlePacket) {
                int number = daemon_bundle_queue_->GetNPackets();
                for (int i = 0; i < number; ++i) {
                    Ptr<Packet> lhs_p_pkt = daemon_bundle_queue_->Dequeue()->GetPacket();
                    BPHeader lhs_bp_header;
                    lhs_p_pkt->RemoveHeader(lhs_bp_header);
                    if (lhs_bp_header.get_source_ip() == bp_header.get_source_ip()
                            && lhs_bp_header.get_source_seqno() == bp_header.get_source_seqno()) {
                        lhs_p_pkt->AddHeader(lhs_bp_header);
                        daemon_bundle_queue_->Enqueue(Packet2Queueit(lhs_p_pkt));//Enqueue(lhs_p_pkt);
                        return true;
                    }
                    lhs_p_pkt->AddHeader(lhs_bp_header);
                    daemon_bundle_queue_->Enqueue(Packet2Queueit(lhs_p_pkt));//Enqueue(lhs_p_pkt);
                }
            } else {
                int number = daemon_antipacket_queue_->GetNPackets();
                for (int i = 0; i < number; ++i) {
                    Ptr<Packet> lhs_p_pkt = daemon_antipacket_queue_->Dequeue()->GetPacket();
                    BPHeader lhs_bp_header;
                    lhs_p_pkt->RemoveHeader(lhs_bp_header);
                    if (lhs_bp_header.get_source_ip() == bp_header.get_source_ip()
                            && lhs_bp_header.get_source_seqno() == bp_header.get_source_seqno()) {
                        lhs_p_pkt->AddHeader(lhs_bp_header);
                        daemon_bundle_queue_->Enqueue(Packet2Queueit(lhs_p_pkt));//Enqueue(lhs_p_pkt);
                        return true;
                    }
                    lhs_p_pkt->AddHeader(lhs_bp_header);
                    daemon_antipacket_queue_->Enqueue(Packet2Queueit(lhs_p_pkt));//Enqueue(lhs_p_pkt);
                }
            }
            return false;
        }

        /* refine 
         * should handle more condition
         * this call handles only link layer sending, so it's called trans_addr not dst_addr
         * LOG some
         */
        bool DtnApp::SocketSendDetail(Ptr<Packet> p_pkt, uint32_t flags, InetSocketAddress trans_addr) {
            // LOG
            std::cout << "Simulation time " << Simulator::Now().GetSeconds() << ", in SocketSendDetail" << std::endl;
            int result = daemon_socket_handle_->SendTo(p_pkt, flags, trans_addr);
            return result != -1 ? true : false;
        }

        /* refine
        */
        void DtnApp::CreateSocketDetail() {
            daemon_socket_handle_ = Socket::CreateSocket(node_, TypeId::LookupByName("ns3::UdpSocketFactory"));
            Ptr<Ipv4> ipv4 = node_->GetObject<Ipv4>();
            Ipv4Address ipip = (ipv4->GetAddress(1, 0)).GetLocal();
            InetSocketAddress local = InetSocketAddress(ipip, NS3DTNBIT_PORT_NUMBER);
            daemon_socket_handle_->Bind(local);
        }

        /* refine
         * using daemon_reorder_buffer_queue_
         * Important! such a ugly sort algorithm, refine it
         */
        void DtnApp::PeriodReorderDaemonBundleQueueDetail() {
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
            Ptr<Packet> p_pkt = Create<Packet>(msg_str.c_str(), msg_str.size());
            BPHeader bp_header;
            do {
                // fill up bp_header
                SemiFillBPHeaderDetail(&bp_header);
                bp_header.set_bundle_type(HelloPacket);
                bp_header.set_source_ip(Ipv4Address("255.255.255.255"));
                bp_header.set_source_seqno(p_pkt->GetUid());
                bp_header.set_payload_size(msg_str.size());
                bp_header.set_offset(msg_str.size());
            } while (0);
            p_pkt->AddHeader(bp_header);
            //p_pkt->AddPacketTag(QosTag(6));
            broad_cast_skt->Send(p_pkt);
        }

        /* refine 
        */
        void DtnApp::RemoveExpiredBAQDetail() {
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
                        int m = 0, found_expired = 0;
                        while (m < NS3DTNBIT_HYPOTHETIC_NEIGHBOR_BAQ_NUMBER_MAX && found_expired == 0) {
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
                Ptr<Packet> p_pkt = daemon_antipacket_queue_->Dequeue()->GetPacket();
                BPHeader bp_header;
                p_pkt->RemoveHeader(bp_header);
                // if not expired
                if ((Simulator::Now().GetSeconds() - bp_header.get_src_time_stamp().GetSeconds() < NS3DTNBIT_ANTIPACKET_EXPIRED_TIME)) {
                    p_pkt->AddHeader(bp_header);
                    daemon_antipacket_queue_->Enqueue(Packet2Queueit(p_pkt));//Enqueue(p_pkt);
                } else {
                    for (int j = 0; j < neighbor_info_vec_.size(); j++) {
                        int k = 0, found_expired = 0;
                        while (k < NS3DTNBIT_HYPOTHETIC_NEIGHBOR_BAQ_NUMBER_MAX && found_expired == 0) {
                            if (bp_header.get_source_seqno() == - neighbor_info_vec_[j].info_sent_ap_seqno_vec_[k]) {
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
         * you should fill following fields your self * 'bundle type' * 'dstip' * 'seqno' * 'payload size'
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

        /* refine
         *
         */
        void DtnExampleInterface::Configure(int argc, char** argv) {
            CommandLine cmdl_parser;
            cmdl_parser.AddValue("randmon_seed", "help:just random", random_seed_);
            cmdl_parser.AddValue("node_number", "nothing help", node_number_);
            cmdl_parser.AddValue("simulation_duration", "nothing help", simulation_duration_);
            cmdl_parser.AddValue("pcap_boolean", "nothing help", pcap_boolean_);
            cmdl_parser.AddValue("print_route_boolean", "nothing help", print_route_boolean_);
            cmdl_parser.AddValue("trace_file", "nothing help", trace_file_);
            if(trace_file_.empty()) {
                std::cout << "traceFile is empty!!!! Usage of " 
                    << "xxxx --traceFile=/path/to/tracefile\n"
                    << "or"
                    << "Run ns-3 ./waf --run scratch/ns3movement --traceFile=/path/to/example/example.ns_movements"
                    << std::endl;
            }
            cmdl_parser.AddValue("log_file", "nothing help", log_file_);
            cmdl_parser.Parse(argc, argv);
            SeedManager::SetSeed(random_seed_);
            file_stream_.open(log_file_.c_str());
        }


        /* refine
         *
         * this would be invoked first when program was running
         */
        void DtnExample::Configure(int argc, char** argv) {
            CommandLine cmdl_parser;
            cmdl_parser.AddValue("randmon_seed", "help:just random", random_seed_);
            cmdl_parser.AddValue("node_number", "nothing help", node_number_);
            cmdl_parser.AddValue("simulation_duration", "nothing help", simulation_duration_);
            cmdl_parser.AddValue("pcap_boolean", "nothing help", pcap_boolean_);
            cmdl_parser.AddValue("print_route_boolean", "nothing help", print_route_boolean_);
            cmdl_parser.AddValue("trace_file", "nothing help", trace_file_);
            if(trace_file_.empty()) {
                std::cout << "traceFile is empty!!!! Usage of " 
                    << "xxxx --traceFile=/path/to/tracefile\n"
                    << "or"
                    << "Run ns-3 ./waf --run scratch/ns3movement --traceFile=/path/to/example/example.ns_movements"
                    << std::endl;
            }
            cmdl_parser.AddValue("log_file", "nothing help", log_file_);
            cmdl_parser.Parse(argc, argv);
            SeedManager::SetSeed(random_seed_);
            file_stream_.open(log_file_.c_str());
        }

        /* refine
        */
        void DtnExampleInterface::Run() {
            Config::SetDefault("ns3::ArpCache::WaitReplyTimeout", StringValue("100000000ns"));
            // default 1.0 s, newset 0.1 s
            Config::SetDefault("ns3::ArpCache::MaxRetries", UintegerValue(10));
            // default 3, newset 10
            Config::SetDefault("ns3::ArpCache::AliveTimeout", StringValue("5000000000000ns"));
            // default 120 s, newset 5000s
            std::cout << "******************** create nodes ******************" << std::endl;
            CreateNodes();
            std::cout << "******************** create devices ******************" << std::endl;
            CreateDevices();
            std::cout << "******************** install stack ******************" << std::endl;
            InstallInternetStack();
            std::cout << "******************** install app ******************" << std::endl;
            InstallApplications();
            std::cout << "********************* Simulate **************" << std::endl;
            std::cout << "simulator began, simulate time = " << simulation_duration_ << ", randmon seed = " << random_seed_ << std::endl;
            Simulator::Stop(Seconds(simulation_duration_));
            Simulator::Run();
            file_stream_.close();
            Simulator::Destroy();
        }


        /* refine
        */
        void DtnExample::Run() {
            Config::SetDefault("ns3::ArpCache::WaitReplyTimeout", StringValue("100000000ns"));
            // default 1.0 s, newset 0.1 s
            Config::SetDefault("ns3::ArpCache::MaxRetries", UintegerValue(10));
            // default 3, newset 10
            Config::SetDefault("ns3::ArpCache::AliveTimeout", StringValue("5000000000000ns"));
            // default 120 s, newset 5000s
            std::cout << "******************** create nodes ******************" << std::endl;
            CreateNodes();
            std::cout << "******************** create devices ******************" << std::endl;
            CreateDevices();
            std::cout << "******************** install stack ******************" << std::endl;
            InstallInternetStack();
            std::cout << "******************** install app ******************" << std::endl;
            InstallApplications();
            // TODO do I really need arpchche populated???
            //std::cout << "******************** populate arpcache ******************" << std::endl;
            //PopulateArpCache();
            std::cout << "********************* Simulate **************" << std::endl;
            std::cout << "simulator began, simulate time = " << simulation_duration_ << ", randmon seed = " << random_seed_ << std::endl;
            Simulator::Stop(Seconds(simulation_duration_));
            Simulator::Run();
            file_stream_.close();
            Simulator::Destroy();
        }

        /* refine 
        */
        void DtnExample::CreateNodes() {
            std::string full_path_str = trace_file_;
            Ns2MobilityHelper ns2_mobi = Ns2MobilityHelper(full_path_str);
            std::cout << "Create " << node_number_ << "nodes." << std::endl;
            nodes_container_.Create(node_number_);
            // name nodes
            for (int i = 0; i < node_number_; ++i) {
                std::stringstream ss;
                ss << "node-" << i;
                Names::Add(ss.str(), nodes_container_.Get(i));
            }
            ns2_mobi.Install();
            // what does Config::Connect("",) means nothing ,just how API works
            Config::Connect("/NodeList/*/$ns3::MobilityModel/CourseChange", 
                    MakeBoundCallback(&CourseChanged, &file_stream_));
        }

        /* refactory, API Change
         * the API is different, check is and refactory
         */
        void DtnExample::CreateDevices() {
            Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue("ErpOfdmRate6Mbps"));
            Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("0"));
            WifiHelper wifi_helper;
            wifi_helper.SetStandard(WIFI_PHY_STANDARD_80211g);
            YansWifiPhyHelper yans_phy_wifi_helper = YansWifiPhyHelper::Default();
            YansWifiChannelHelper yans_wifi_chnnel_helper;
            //YansWifiChannel yans_wifi_chnnel;
            yans_wifi_chnnel_helper.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
            yans_wifi_chnnel_helper.AddPropagationLoss("ns3::FriisPropagationLossModel");
            yans_phy_wifi_helper.SetChannel(yans_wifi_chnnel_helper.Create());
            QosWifiMacHelper wifi_mac = QosWifiMacHelper::Default();
            wifi_helper.SetRemoteStationManager("ns3::IdealWifiManager");
            yans_phy_wifi_helper.Set("TxPowerLevels", UintegerValue(1));    // default 1
            yans_phy_wifi_helper.Set("TxPowerStart", DoubleValue(12.5));    // default 16.0206
            yans_phy_wifi_helper.Set("TxPowerEnd", DoubleValue(12.5));    // default 16.0206
            yans_phy_wifi_helper.Set("EnergyDetectionThreshold", DoubleValue(-74.5));    // default -96
            yans_phy_wifi_helper.Set("CcaMode1Threshold", DoubleValue(-77.5));    // default -99
            yans_phy_wifi_helper.Set("RxNoiseFigure", DoubleValue(7));    // default 7
            yans_phy_wifi_helper.Set("TxGain", DoubleValue(1.0));    // default 1.0
            yans_phy_wifi_helper.Set("RxGain", DoubleValue(1.0));    // default 1.0
            wifi_mac.SetType("ns3::AdhocWifiMac");

            net_devices_container_ = wifi_helper.Install(yans_phy_wifi_helper, wifi_mac, nodes_container_);
            if (pcap_boolean_) {
                yans_phy_wifi_helper.EnablePcapAll(std::string("rtprot"));
            }
        }

        /* refine
        */
        void DtnExample::InstallInternetStack() {
            Ipv4StaticRoutingHelper rtprot;
            InternetStackHelper stack_helper;
            stack_helper.SetRoutingHelper(rtprot);
            stack_helper.Install(nodes_container_);
            Ipv4AddressHelper ip_helper;
            ip_helper.SetBase("10.0.0.0", "255.0.0.0");
            ip_interface_container_ = ip_helper.Assign(net_devices_container_);
            if (print_route_boolean_) {
                Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("rtprot.routes", std::ios::out);
                rtprot.PrintRoutingTableAllAt(Seconds(8), routingStream);
            }
        }

        /* refine
        */
        void DtnExample::InstallApplications() {
            //std::cout << GetLogStr("In InstallApplication") << std::endl;
            TypeId udp_tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
            Ptr<DtnApp> app[node_number_];
            for (uint32_t i = 0; i < node_number_; ++i) { 
                app[i] = CreateObject<DtnApp>();
                app[i]->SetUp(nodes_container_.Get(i));
                nodes_container_.Get(i)->AddApplication(app[i]);
                //app[i]->SetStartTime (Seconds (0.5 + 0.00001*i));
                //app[i]->SetStopTime (Seconds (5000.0));

                Ptr<Socket> dst = Socket::CreateSocket(nodes_container_.Get(i), udp_tid);
                char dststring[1024]="";
                sprintf(dststring,"10.0.0.%d",(i + 1));
                InetSocketAddress dstlocaladdr (Ipv4Address(dststring), 50000);
                dst->Bind(dstlocaladdr);
                dst->SetRecvCallback(MakeCallback(&DtnApp::ReceiveBundle, app[i]));

                Ptr<Socket> source = Socket::CreateSocket(nodes_container_.Get (i), udp_tid);
                InetSocketAddress remote(Ipv4Address("255.255.255.255"), 80);
                source->SetAllowBroadcast(true);
                source->Connect(remote);
                Time tmpt = Seconds(0.1 + 0.01*i);
                app[i]->ToSendHello(source, simulation_duration_, tmpt, false);

                Ptr<Socket> recvSink = Socket::CreateSocket(nodes_container_.Get(i), udp_tid);
                InetSocketAddress local(Ipv4Address::GetAny(), 80);
                recvSink->Bind(local);
                recvSink->SetRecvCallback(MakeCallback(&DtnApp::ReceiveHello, app[i]));
            }
            Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
            for (uint32_t i = 0; i < node_number_; ++i) { 
                for (uint32_t j = 0; j < 4; ++j) { 
                    uint32_t dstnode = i;
                    while (dstnode == i) {
                        dstnode = x->GetInteger(0, node_number_-1);
                    }
                    double xinterval = simulation_duration_ / (2 * node_number_);
                    app[i]->ScheduleTx(dstnode, Seconds(xinterval * j + x->GetValue(0.0, 200.0)), 200*(x->GetInteger(1, 10)));
                }
            }
            Simulator::Schedule(Seconds(1), &DtnExample::LogCounter, this, 0);
        }

        /*
         * 
         */
        void DtnExample::LogCounter(int n) {
            std::cout << "counter===> simulation time : " << n << "\n" << std::endl;
            if (n < simulation_duration_) {
                Simulator::Schedule(Seconds(1), &DtnExample::LogCounter, this, n + 1);
            }
        }

        /* TODO write Report method, this should write total report to one file
         * 
         */
        void DtnExampleInterface::Report(std::ostream& os) {
            os << "Here In DtnExampleInterface::Report" << endl;
        }

        /* TODO write Report method, this should write total report to one file
         * 
         */
        void DtnExample::Report(std::ostream& os) {
            os << "Here In DtnExample::Report" << endl;
        }

        /* refactory
        */
        void DtnExample::PopulateArpCache() { 
            std::cout << GetLogStr("In PopulateArpCache") << std::endl;
            // create a long last arp cache
            Ptr<ArpCache> arp = CreateObject<ArpCache>(); 
            arp->SetAliveTimeout(Seconds(3600 * 24 * 365)); 
            // Populates ARP Cache with information from all nodes
            for (NodeList::Iterator i = NodeList::Begin(); i != NodeList::End(); ++i) { 
                Ptr<Ipv4L3Protocol> ip = (*i)->GetObject<Ipv4L3Protocol> (); 
                NS_ASSERT(ip !=0); 
                ObjectVectorValue interfaces; 
                // in one ipv4 protocol of one node, there might be multiply ip interfaces(ip address and mask)
                ip->GetAttribute("InterfaceList", interfaces); 
                for (uint32_t j = 0; j != ip->GetNInterfaces(); j ++) {
                    Ptr<Ipv4Interface> ipIface = ip->GetInterface(j);
                    NS_ASSERT(ipIface != 0); 
                    Ptr<NetDevice> device = ipIface->GetDevice(); 
                    NS_ASSERT(device != 0); 
                    Mac48Address addr = Mac48Address::ConvertFrom(device->GetAddress()); 
                    for (uint32_t k = 0; k < ipIface->GetNAddresses(); k ++) { 
                        Ipv4Address ipAddr = ipIface->GetAddress(k).GetLocal(); 
                        // GetLocal() get the local address, which is used only in a private network, and the routers won't
                        // routes the traffic, you can routes whatever you want.
                        if(ipAddr == Ipv4Address::GetLoopback()) {
                            continue; 
                        }
                        ArpCache::Entry* entry = arp->Add(ipAddr); 
                        // TODO bug fix, for now, we don't need to fix it because that we don't use it anymore.
                        // this would cause compile error
                        //entry->MarkWaitReply(0); 
                        entry->MarkWaitReply(ns3::ArpCache::Ipv4PayloadHeaderPair()); 
                        entry->MarkAlive(addr); 
                    } 
                } 
            } 
            for (NodeList::Iterator i = NodeList::Begin(); i != NodeList::End(); ++i) { 
                Ptr<Ipv4L3Protocol> ip = (*i)->GetObject<Ipv4L3Protocol>(); 
                NS_ASSERT(ip !=0); 
                ObjectVectorValue interfaces; 
                ip->GetAttribute("InterfaceList", interfaces);
                for (uint32_t j = 0; j != ip->GetNInterfaces(); j ++) {
                    Ptr<Ipv4Interface> ipIface = ip->GetInterface (j);
                    ipIface->SetAttribute("ArpCache", PointerValue(arp)); 
                } 
            } 
        }

        /* refine
        */
        void DtnApp::ScheduleTx (uint32_t dstnode, Time tNext, uint32_t payload_size)
        {
            send_event_id_ = Simulator::Schedule (tNext, &DtnApp::ToSendBundle, this, dstnode, payload_size);
        }
    } /* ns3dtnbit */ 
    /* ... */
}

