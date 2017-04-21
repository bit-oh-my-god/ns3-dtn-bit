/*
*/
#include "ns3dtn-bit-example-interface.h"
#include "../config.txt"
extern std::string root_path;

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
            if (print_wifi_log_) {
                wifi.EnableLogComponents();  // Turn on all Wifi logging
            }
            wifi.SetStandard(WIFI_PHY_STANDARD_80211b);
            YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default();
            wifiPhy.Set("RxGain", DoubleValue(0)); 
            wifiPhy.SetPcapDataLinkType(YansWifiPhyHelper::DLT_IEEE802_11_RADIO); 
            YansWifiChannelHelper wifiChannel;
            wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
            wifiChannel.AddPropagationLoss ("ns3::RangePropagationLossModel",  
                    "MaxRange", DoubleValue (4000.0));
            wifiPhy.SetChannel(wifiChannel.Create());
            WifiMacHelper wifiMac;

            wifi.SetRemoteStationManager("ns3::IdealWifiManager");
            // Set it to adhoc mode
            wifiMac.SetType("ns3::AdhocWifiMac");
            net_devices_container_ = wifi.Install(wifiPhy, wifiMac, nodes_container_);
        }

        // use vector for further modify
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
                    vector<int> ntpos = {x_v, y_v, z_v, node_v, time_v};
                    ntpos_map[time_v][node_v] = ntpos;
                }
            }
            assert(ntpos_map[0].size() == node_number_);
            std::map<int, vector<vector<int>>> t_2_adjacent_array;
            {
                // calculate code
                auto distance_func = [](std::map<int, vector<int>>& m, int i, int j) -> int {
                    double reint = 0.0;
                    auto posi = m[i];
                    auto posj = m[j];
                    reint = ((double)(posi[0] - posj[0]) * (double)(posi[0] - posj[0]))
                        + ((double)(posi[1] - posj[1]) * (double)(posi[1] - posj[1]))
                        + ((double)(posi[2] - posj[2]) * (double)(posi[2] - posj[2]));
                    reint = sqrt(reint);
                    if (reint == 0) {
                        std::cout << "DEBUG time=" << posi[4] << ",i =" << i << ", pos=" << posi[0] << "-" << posi[1] << "-" << posi[2] 
                            << ", j = " << j << ", pos=" << posj[0] << "-" << posj[1] << "-" << posj[2] << std::endl;
                    }
                    return reint;
                };
                for (auto& m1 : ntpos_map) {
                    auto m2 = get<1>(m1);
                    int time = get<0>(m1);
                    auto tmp_adjacent_array = vector<vector<int>>(node_number_, vector<int>(node_number_, -1));
                    for (int i = 0; i < node_number_; i++) {
                        for (int j = 0; j < node_number_; ++j) {
                            if (i == j) {continue;}
                            tmp_adjacent_array[i][j] = distance_func(m2, i, j);
                        }
                    }
                    t_2_adjacent_array[time] = tmp_adjacent_array;
                }
            }
            std::cout << "DEBUG:outof608" << std::endl;
            for (int a = 0; a < node_number_; ++a) {
                for (int b = 0; b < node_number_; ++b) {
                    std::cout << "v-" << a << "-" << b << "=" << t_2_adjacent_array[608][a][b] << std::endl;
                }
            }
            DtnApp::Adob adob_ob = DtnApp::Adob();
            adob_ob.AdobDo_01(t_2_adjacent_array, node_number_);
            // default, I recommand to use set teg_layer_n to be time_duration_ / 2
            adob_ob.AdobDo_02(node_number_, simulation_duration_, 4000);
            std::cout << "NOTE: after AdobDo_02()" <<std::endl;
            adob_ob.AdobDo_03();
            std::cout << "NOTE: after AdobDo_03()" <<std::endl;
            result.emplace_back(std::move(adob_ob));
            return result;
        }

        void DtnExampleInterface::InstallApplications() {
            TypeId udp_tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
            // create adob for app
            std::cout << "=============== CreateAdjacentList ===============" << endl;
            auto adob = CreateAdjacentList();
            std::cout << "=============== End of create ===============" << endl;
            //Ptr<DtnApp> app[node_number_];
            for (uint32_t i = 0; i < node_number_; ++i) { 
                // create app and set
                apps_.push_back(CreateObject<DtnApp>());
                apps_[i]->SetUp(nodes_container_.Get(i));
                nodes_container_.Get(i)->AddApplication(apps_[i]);
                apps_[i]->SetStartTime(Seconds(0.0));
                apps_[i]->SetStopTime(Seconds(5000.0));
                // set bundle receive socket
                Ptr<Socket> dst = Socket::CreateSocket(nodes_container_.Get(i), udp_tid);
                char dststring[1024]="";
                sprintf(dststring,"10.0.0.%d",(i + 1));
                InetSocketAddress dstlocaladdr(Ipv4Address(dststring), NS3DTNBIT_PORT_NUMBER);
                dst->Bind(dstlocaladdr);
                dst->SetRecvCallback(MakeCallback(&DtnApp::ReceiveBundle, apps_[i]));
                // set hello send socket
                Ptr<Socket> source = Socket::CreateSocket(nodes_container_.Get (i), udp_tid);
                InetSocketAddress remote(Ipv4Address("255.255.255.255"), NS3DTNBIT_HELLO_PORT_NUMBER);
                source->SetAllowBroadcast(true);
                source->Connect(remote); 
                Time tmpt = Seconds(0.1 + 0.01*i); 
                apps_[i]->ToSendHello(source, simulation_duration_, tmpt, false); 
                // set hello listen socket 
                Ptr<Socket> recvSink = Socket::CreateSocket(nodes_container_.Get(i), udp_tid);
                InetSocketAddress local(Ipv4Address::GetAny(), NS3DTNBIT_HELLO_PORT_NUMBER);
                recvSink->Bind(local);
                recvSink->SetRecvCallback(MakeCallback(&DtnApp::ReceiveHello, apps_[i]));
                // load adob to each app, and load RoutingMethodInterface
                if (ex_rm_ == DtnApp::RoutingMethod::Other || ex_rm_ == DtnApp::RoutingMethod::TimeExpanded) {
                    std::unique_ptr<RoutingMethodInterface> p_rm_in = CreateRouting(*apps_[i]);
                    apps_[i]->InvokeMeWhenInstallAppToSetupDtnAppRoutingAssister(ex_rm_, std::move(p_rm_in), adob);
                } else if (ex_rm_ == DtnApp::RoutingMethod::SprayAndWait) {
                    apps_[i]->InvokeMeWhenInstallAppToSetupDtnAppRoutingAssister(ex_rm_, adob);
                } else {
                    std::cout << "Error : can't find Routing method" << __LINE__ << std::endl;
                    std::abort();
                }
            }
        }

        void DtnExampleInterface::ScheduleTask() {
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
                    apps_[i]->ScheduleTx(Seconds(sch_time), dstnode, sch_size);
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
            Ipv4GlobalRoutingHelper::PopulateRoutingTables();
        }

        DtnExampleInterface::DtnExampleInterface() {
            // add some default value for settings in Configure() call
            random_seed_ = 214127;
            node_number_ = 20 ;
            simulation_duration_ = 600;
            // TODO, make files to be relative path
            std::stringstream ss0;
            ss0 << root_path << "/box/current_trace/current_trace.tcl";
            trace_file_ = ss0.str();
            std::stringstream ss1;
            ss1 << root_path << "/box/current_trace/teg.txt";
            teg_file_ = ss1.str();
            std::stringstream ss2;
            ss2 << root_path << "/box/dtn_simulation_result/dtn_trace_log.txt";
            log_file_ = ss2.str();
            //trace_file_ = "/home/dtn-012345/ns-3_build/ns3-dtn-bit/box/current_trace/current_trace.tcl";
            //teg_file_ = "/home/dtn-012345/ns-3_build/ns3-dtn-bit/box/current_trace/teg.txt";
            //log_file_ = "~/ns-3_build/ns3-dtn-bit/box/dtn_simulation_result/dtn_trace_log.txt";
        }


        /* we define this would rewrite ( you can say shade ) the dufault CourseChange
         * and this rewrite is supposed in tutorial
         */
        static void CourseChanged(std::ostream *myos, std::string something, Ptr<const MobilityModel> mobility) {
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
        //NS_LOG_COMPONENT_DEFINE ("Ns2MobilityHelper");
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
            file_stream_ << "begin moveing\n" << std::endl;
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
            std::cout << "******************* ScheduleTask *************" << std::endl;
            ScheduleTask();
            std::cout << "******************** simulation ***************" << std::endl;
            std::cout << "simulator began, nodes =" << node_number_ << " simulate time = " << simulation_duration_ << ", randmon seed = " << random_seed_ << std::endl;
            Simulator::Stop(Seconds(simulation_duration_));
            Simulator::Run();
            file_stream_.close();
            Simulator::Destroy();
        }

    } /* ns3dtnbit */ 

} /* ns3 */ 
