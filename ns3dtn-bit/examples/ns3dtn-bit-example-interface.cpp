/*
*/
#include "ns3dtn-bit-example-interface.h"

namespace ns3 {
    namespace ns3dtnbit {
        DtnExampleInterface::DtnExampleInterface(DtnExampleInterface&& rh) {

        }
        /*
         * 
         * copy from ns3 wifi-adhoc example
         */
        void DtnExampleInterface::CreateDevices() {
            WifiHelper wifi;
            std::string phyMode("DsssRate1Mbps");
            double rss = -80;  // -dBm
            if (print_log_boolean_) {
                // wifi.EnableLogComponents();  // Turn on all Wifi logging
            }
            wifi.SetStandard(WIFI_PHY_STANDARD_80211b);
            YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default();
            // This is one parameter that matters when using FixedRssLossModel
            // set it to zero; otherwise, gain will be added
            wifiPhy.Set("RxGain", DoubleValue(0)); 
            // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
            wifiPhy.SetPcapDataLinkType(YansWifiPhyHelper::DLT_IEEE802_11_RADIO); 
            YansWifiChannelHelper wifiChannel;
            wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
            // The below FixedRssLossModel will cause the rss to be fixed regardless
            // of the distance between the two stations, and the transmit power
            wifiChannel.AddPropagationLoss("ns3::FixedRssLossModel", "Rss", DoubleValue(rss));
            wifiPhy.SetChannel(wifiChannel.Create());
            // Add a mac and disable rate control
            WifiMacHelper wifiMac;
            wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                    "DataMode",StringValue(phyMode),
                    "ControlMode",StringValue(phyMode));
            // Set it to adhoc mode
            wifiMac.SetType("ns3::AdhocWifiMac");
            net_devices_container_ = wifi.Install(wifiPhy, wifiMac, nodes_container_);
        }

        void DtnExampleInterface::InstallApplications() {
            TypeId udp_tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
            Ptr<DtnApp> app[node_number_];
            for (uint32_t i = 0; i < node_number_; ++i) { 
                // create app and set
                app[i] = CreateObject<DtnApp>();
                app[i]->SetUp(nodes_container_.Get(i));
                app[i]->InvokeMeWhenInstallAppToSetupDtnAppRoutingAssister(ex_rm_);
                nodes_container_.Get(i)->AddApplication(app[i]);
                app[i]->SetStartTime(Seconds(0.0));
                app[i]->SetStopTime(Seconds(5000.0));
                // set bundle receive socket
                Ptr<Socket> dst = Socket::CreateSocket(nodes_container_.Get(i), udp_tid);
                char dststring[1024]="";
                sprintf(dststring,"10.0.0.%d",(i + 1));
                InetSocketAddress dstlocaladdr(Ipv4Address(dststring), NS3DTNBIT_PORT_NUMBER);
                dst->Bind(dstlocaladdr);
                dst->SetRecvCallback(MakeCallback(&DtnApp::ReceiveBundle, app[i]));

                // set hello send socket
                Ptr<Socket> source = Socket::CreateSocket(nodes_container_.Get (i), udp_tid);
                InetSocketAddress remote(Ipv4Address("255.255.255.255"), NS3DTNBIT_HELLO_PORT_NUMBER);
                source->SetAllowBroadcast(true);
                source->Connect(remote);
                Time tmpt = Seconds(0.1 + 0.01*i);
                app[i]->ToSendHello(source, simulation_duration_, tmpt, false);
                // set hello listen socket
                Ptr<Socket> recvSink = Socket::CreateSocket(nodes_container_.Get(i), udp_tid);
                InetSocketAddress local(Ipv4Address::GetAny(), NS3DTNBIT_HELLO_PORT_NUMBER);
                recvSink->Bind(local);
                recvSink->SetRecvCallback(MakeCallback(&DtnApp::ReceiveHello, app[i]));
            }
            // bundle send 
            Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
            for (uint32_t i = 0; i < node_number_; ++i) { 
                for (uint32_t j = 0; j < 2; ++j) { 
                    uint32_t dstnode = i;
                    while (dstnode == i) {
                        dstnode = x->GetInteger(0, node_number_-1);
                    }
                    double xinterval = simulation_duration_ / (2 * node_number_);
                    dtn_time_t sch_time = xinterval * j + x->GetValue(0.0, 200.0);
                    // note that 'NS3DTNBIT_HYPOTHETIC_TRANS_SIZE_FRAGMENT_MAX == 1427'
                    int sch_size = 345;
                    std::cout << "bundle send schedule: time=" << sch_time << ";node-" << i << "send " << sch_size << " size-pkt to node-" << dstnode << std::endl;
                    app[i]->ScheduleTx(Seconds(sch_time), dstnode, sch_size);
                }
            }
            Simulator::Schedule(Seconds(5), &DtnExampleInterface::LogCounter, this, 5);
        }

        void DtnExampleInterface::InstallInternetStack() {
            InternetStackHelper internet;
            internet.Install(nodes_container_);
            Ipv4AddressHelper ipv4;
            NS_LOG_UNCOND("Assign IP Addresses.");
            ipv4.SetBase("10.0.0.0", "255.0.0.0");
            Ipv4InterfaceContainer i = ipv4.Assign(net_devices_container_);
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

        /* refine
         *
         */
        void DtnExampleInterface::ConfigureEx(int argc, char** argv) {
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

        /* TODO write Report method, this should write total report to one file
         * 
         */
        void DtnExample::Report(std::ostream& os) {
            os << "Here In DtnExample::Report" << endl;
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

        /*
         * 
         */
        void DtnExampleInterface::LogCounter(int n) {
            const int inter = 20;
            std::cout << "counter===> simulation time : " << n << "\n" << std::endl;
            if (n < simulation_duration_) {
                Simulator::Schedule(Seconds(inter), &DtnExampleInterface::LogCounter, this, n + inter);
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
                    app[i]->ScheduleTx(Seconds(xinterval * j + x->GetValue(0.0, 200.0)),
                            dstnode,
                            100*(x->GetInteger(3, 8)));
                }
            }
            Simulator::Schedule(Seconds(1), &DtnExample::LogCounter, this, 0);
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

        /* refine 
        */
        void DtnExampleInterface::CreateNodes() {
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
        void DtnExampleInterface::RunEx() {
            std::cout << "******************** create " << node_number_ << " nodes ******************"<< std::endl;
            CreateNodes();
            std::cout << "******************** create devices ***************" << std::endl;
            CreateDevices();
            std::cout << "******************** install stack ***************" << std::endl;
            InstallInternetStack();
            std::cout << "******************** install app ***************" << std::endl;
            InstallApplications();
            std::cout << "******************** simulation ***************" << std::endl;
            std::cout << "simulator began, nodes =" << node_number_ << " simulate time = " << simulation_duration_ << ", randmon seed = " << random_seed_ << std::endl;
            Simulator::Stop(Seconds(simulation_duration_));
            Simulator::Run();
            file_stream_.close();
            Simulator::Destroy();
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

    } /* ns3dtnbit */ 

} /* ns3 */ 
