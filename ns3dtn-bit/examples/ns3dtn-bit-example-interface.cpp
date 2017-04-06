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

        vector<DtnApp::Adob> DtnExampleInterface::CreateAdjacentList() {
            vector<DtnApp::Adob> result;
            std::map<int, std::map<int, vector<int>>> ntpos_map;
            {
                // regular expression code TODO
                std::ifstream infile(teg_file_);
                string line;
                while (std::getline(infile, line)) {
                    std::istringstream iss(line);
                    string str_tmp;
                    int node_v, time_v, x_v, y_v, z_v;
                    iss >> str_tmp >> node_v >> str_tmp >> time_v >> str_tmp >> x_v >> y_v >> z_v;
                    vector<int> ntpos = {node_v, time_v, x_v, y_v, z_v};
                    ntpos_map[time_v][node_v] = ntpos;
                }
            }
            assert(ntpos_map.size() == node_number_);
            std::map<int, vector<vector<int>>> t_2_adjacent_array;
            {
                // calculate code
                auto distance_func = [](std::map<int, vector<int>>& m, int i, int j) -> int {
                    long reint = 0;
                    auto pos1 = m[i];
                    auto pos2 = m[j];
                    reint = (pos1[0] - pos2[0]) * (pos1[0] - pos2[0]) 
                        + (pos1[1] - pos2[1]) * (pos1[1] - pos2[1])
                        + (pos1[2] - pos2[2]) *(pos1[2] - pos2[2]);
                    reint = sqrt(reint);
                    return reint;
                };
                for (auto& m1 : ntpos_map) {
                    auto m2 = get<1>(m1);
                    int time = get<0>(m1);
                    auto tmp_adjacent_array = vector<vector<int>>(node_number_, vector<int>(node_number_, -1));
                    for (int i = 0; i < node_number_; i++) {
                        for (int j = 0; j < node_number_; ++j) {
                            tmp_adjacent_array[i][j] = distance_func(m2, i, j);
                        }
                    }
                    t_2_adjacent_array[time] = tmp_adjacent_array;
                }
            }
            DtnApp::Adob adob_ob = DtnApp::Adob(t_2_adjacent_array, node_number_);
            result.push_back(adob_ob);
            return result;
        }

        void DtnExampleInterface::InstallApplications() {
            TypeId udp_tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
            // create adob for app
            std::cout << "=============== CreateAdjacentList ===============" << endl;
            auto adob = CreateAdjacentList();
            std::cout << "=============== End of create ===============" << endl;
            Ptr<DtnApp> app[node_number_];
            for (uint32_t i = 0; i < node_number_; ++i) { 
                // create app and set
                app[i] = CreateObject<DtnApp>();
                app[i]->SetUp(nodes_container_.Get(i));
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
                source->Connect(remote); Time tmpt = Seconds(0.1 + 0.01*i); app[i]->ToSendHello(source, simulation_duration_, tmpt, false); 
                // set hello listen socket 
                Ptr<Socket> recvSink = Socket::CreateSocket(nodes_container_.Get(i), udp_tid);
                InetSocketAddress local(Ipv4Address::GetAny(), NS3DTNBIT_HELLO_PORT_NUMBER);
                recvSink->Bind(local);
                recvSink->SetRecvCallback(MakeCallback(&DtnApp::ReceiveHello, app[i]));
                // load adob to each app, and load RoutingMethodInterface
                if (ex_rm_ == DtnApp::RoutingMethod::Other) {
                    std::unique_ptr<RoutingMethodInterface> p_rm_in = CreateRouting(*app[i]);
                    app[i]->InvokeMeWhenInstallAppToSetupDtnAppRoutingAssister(ex_rm_, std::move(p_rm_in), adob);
                } else if (ex_rm_ == DtnApp::RoutingMethod::SprayAndWait) {
                    app[i]->InvokeMeWhenInstallAppToSetupDtnAppRoutingAssister(ex_rm_, adob);
                } else {
                    std::abort();
                }
            }
            // bundle send 
            Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
            unsigned int pkts_total = 3;
            double xinterval = simulation_duration_ / (2 * node_number_);
            for (uint32_t i = 0; i < node_number_; ++i) { 
                for (uint32_t j = 0; j < pkts_total; ++j) { 
                    uint32_t dstnode = i;
                    while (dstnode == i) {
                        dstnode = x->GetInteger(0, node_number_-1);
                    }
                    dtn_time_t sch_time = xinterval * j + x->GetValue(0.0, simulation_duration_ / node_number_);
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
            // TODO, make files to be relative path
            trace_file_ = "/home/dtn-012345/ns-3_build/ns3-dtn-bit/box/current_trace/current_trace.tcl";
            teg_file_ = "/home/dtn-012345/ns-3_build/ns3-dtn-bit/box/current_trace/teg.txt";
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
            std::cout << "Using Boost "     
                << BOOST_VERSION / 100000     << "."  // major version
                << BOOST_VERSION / 100 % 1000 << "."  // minor version
                << BOOST_VERSION % 100                // patch level
                << std::endl;
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

    } /* ns3dtnbit */ 

} /* ns3 */ 
