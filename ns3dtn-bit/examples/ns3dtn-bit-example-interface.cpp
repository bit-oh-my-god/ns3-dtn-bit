/*
*/
#include "ns3dtn-bit-example-interface.h"
#include "../model/routing.h"
#include "../model/cgrrouting.h"
#include "../model/cgrqmrouting.h"

namespace ns3 {
    namespace ns3dtnbit {
        DtnExampleInterface::DtnExampleInterface(DtnExampleInterface&& rh) {
        }

        void DtnExampleInterface::set_th(size_t th) {
            th_ = th;
        }

        /*
         * 
         * copy from ns3 wifi-adhoc example
         */
        void DtnExampleInterface::CreateDevices() {
            WifiHelper wifi;
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
                    "MaxRange", DoubleValue (NS3DTNBIT_MAXRANGE));
            wifiPhy.SetChannel(wifiChannel.Create());
            WifiMacHelper wifiMac;

            //wifi.SetRemoteStationManager("ns3::IdealWifiManager");
            wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode", StringValue ("DsssRate1Mbps"),
                                "ControlMode", StringValue ("DsssRate1Mbps"));

            // Set it to adhoc mode
            wifiMac.SetType("ns3::AdhocWifiMac");
            net_devices_container_ = wifi.Install(wifiPhy, wifiMac, nodes_container_);
        }

        // use vector for further modify
        vector<Adob> DtnExampleInterface::CreateAdjacentList() {
            std::cout << "=============== CreateAdjacentList ===============" << endl;
            vector<Adob> result;
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
            int time_of_simulation = simulation_duration_;
            int count_of_slice_of_adob = t_2_adjacent_array.size();
            if (time_of_simulation != count_of_slice_of_adob) {
                cout << "Error: time of simulation should be the same as count of slice of adob, the inequality would cause routing method undefined works. Please change your simulation time as your teg.txt time" << endl;
                std::abort();
            }
            {
                /*
                   std::cout << "DEBUG:outof608" << std::endl;
                   for (int a = 0; a < node_number_; ++a) {
                   for (int b = 0; b < node_number_; ++b) {
                   std::cout << "v-" << a << "-" << b << "=" << t_2_adjacent_array[608][a][b] << std::endl;
                   }
                   }
                   */
            }
            Adob adob_ob = Adob();
            adob_ob.AdobDo_01(t_2_adjacent_array, node_number_);
            // by default, recommand to set teg_layer_n to be time_duration_ / 2
            adob_ob.AdobDo_02(node_number_, simulation_duration_, NS3DTNBIT_MAXRANGE);
            std::cout << "NOTE: after AdobDo_02()" <<std::endl;
            adob_ob.AdobDo_03();
            std::cout << "NOTE: after AdobDo_03()" <<std::endl;
            adob_ob.AdobDo_04();
            std::cout << "NOTE: after AdobDo_04()" <<std::endl;
            result.emplace_back(std::move(adob_ob));
            std::cout << "=============== End of CreateAdjacentList ===============" << endl;
            return result;
        }

        void DtnExampleInterface::InstallApplications() {
            std::cout << "=============== InstallApplications ===============" << endl;
            TypeId udp_tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
            // create adob for app
            auto adob = CreateAdjacentList();
            //Ptr<DtnApp> app[node_number_];
            InitStorage();
            for (uint32_t i = 0; i < node_number_; ++i) { if (config_storage_max_.count(i)) { } else { config_storage_max_[i] = NS3DTNBIT_DEFAULT_QUEUE_MAX; } }
            for (uint32_t i = 0; i < node_number_; ++i) { 
                cout << "---> install app for node-" << i << endl;
                // create app and set
                apps_.push_back(CreateObject<DtnApp>());
                apps_[i]->SetQueueParameter(config_storage_max_[i]);
                apps_[i]->SetUp(nodes_container_.Get(i));
                nodes_container_.Get(i)->AddApplication(apps_[i]);
                apps_[i]->SetStartTime(Seconds(0.0));
                apps_[i]->SetStopTime(Seconds(5000.0));
                // set (storage limit)queue parameter and id_of_d2cur_excluded_vec_of_d_
                // bundle send socket would be set inside DtnApp
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
                Time tmp_t = Seconds(0.5 + 0.01 * (i));
                apps_[i]->ToSendHello(source, simulation_duration_, tmp_t); 
                // set hello listen socket 
                Ptr<Socket> recvSink = Socket::CreateSocket(nodes_container_.Get(i), udp_tid);
                InetSocketAddress local(Ipv4Address::GetAny(), NS3DTNBIT_HELLO_PORT_NUMBER);
                recvSink->Bind(local);
                recvSink->SetRecvCallback(MakeCallback(&DtnApp::ReceiveHello, apps_[i]));
                // load adob to each app, and load RoutingMethodInterface
                if (
                   ex_rm_ == DtnApp::RoutingMethod::Other 
                || ex_rm_ == DtnApp::RoutingMethod::TimeExpanded 
                || ex_rm_ == DtnApp::RoutingMethod::CGR
                || ex_rm_ == DtnApp::RoutingMethod::QM
                ) {
                    std::unique_ptr<RoutingMethodInterface> p_rm_in = CreateRouting(*apps_[i]);
                    apps_[i]->InvokeMeWhenInstallAppToSetupDtnAppRoutingAssister(ex_rm_, std::move(p_rm_in), adob);
                } else if (ex_rm_ == DtnApp::RoutingMethod::SprayAndWait 
                || ex_rm_ == DtnApp::RoutingMethod::DirectForward) {
                    apps_[i]->InvokeMeWhenInstallAppToSetupDtnAppRoutingAssister(ex_rm_, adob);
                } else {
                    std::cout << "Error : can't find Routing method" << __LINE__ << std::endl;
                    std::abort();
                }
            }
            std::cout << "=============== End of InstallApplications ===============" << endl;
        }

        void DtnExampleInterface::ScheduleTask() {
            Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
            unsigned int pkts_total = 2;
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
            // this function is not used now
            //Simulator::Schedule(Seconds(5), &DtnExampleInterface::LogCounter, this, 5);
        }

        void DtnExampleInterface::InstallInternetStack() {
            InternetStackHelper internet;
            internet.Install(nodes_container_);
            Ipv4AddressHelper ipv4;
            NS_LOG_UNCOND("Assign IP Addresses.");
            ipv4.SetBase("10.0.0.0", "10.252.0.0");
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
            ss2 << root_path << "/box/dtn_simulation_result/dtn_trace_log_" << th_ <<".txt";
            log_file_ = ss2.str();
            //trace_file_ = "/home/dtn-012345/ns-3_build/ns3-dtn-bit/box/current_trace/current_trace.tcl";
            //teg_file_ = "/home/dtn-012345/ns-3_build/ns3-dtn-bit/box/current_trace/teg.txt";
            //log_file_ = "~/ns-3_build/ns3-dtn-bit/box/dtn_simulation_result/dtn_trace_log.txt";
        }

        void DtnExampleInterface::InitStorage() { }

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

        std::unique_ptr<RoutingMethodInterface> DtnExampleInterface::CreateRouting(DtnApp& dtn) {
            if (ex_rm_ == DtnApp::RoutingMethod::SprayAndWait) {
                auto p = new EmptyRouting(dtn);
                cout << "BundleTrace:Itisnotonedeliverymethon\n" << endl;
                cout << "BundleTrace:install route :sprayandwait\n" << endl;
                return std::unique_ptr<RoutingMethodInterface>(p);
            } else if (ex_rm_ == DtnApp::RoutingMethod::TimeExpanded) {
                auto p = new TegRouting(dtn);
                cout << "BundleTrace:Itisonedeliverymethon\n" << endl;
                cout << "BundleTrace:install route :teg\n" << endl;
                return std::unique_ptr<RoutingMethodInterface>(p);
            } else if (ex_rm_ == DtnApp::RoutingMethod::CGR) {
                auto p = new CGRRouting(dtn);
                cout << "BundleTrace:Itisonedeliverymethon\n" << endl;
                cout << "BundleTrace:install route :cgr\n" << endl;
                return std::unique_ptr<RoutingMethodInterface>(p);
            } else if (ex_rm_ == DtnApp::RoutingMethod::Other) {
                auto p = new YouRouting(dtn);
                cout << "BundleTrace:Itisonedeliverymethon\n" << endl;
                cout << "BundleTrace:install route :heuris\n" << endl;
                return std::unique_ptr<RoutingMethodInterface>(p);
            } else if (ex_rm_ == DtnApp::RoutingMethod::QM) {
                auto p = new CGRQMRouting(dtn);
                cout << "CGRQM routing create" << endl;
                map<node_id_t, pair<int, int>> empty01;
                map<node_id_t, pair<int, int>> empty02;
                map<node_id_t, size_t> storagemax = config_storage_max_;
                pair<vector<node_id_t>, dtn_time_t> empty03;
                p->StorageinfoMaintainInterface("give storage_max_", empty01, empty02, storagemax, empty03);
                cout << "BundleTrace:Itisonedeliverymethon\n" << endl;
                cout << "BundleTrace:install route :qm\n" << endl;
                return std::unique_ptr<RoutingMethodInterface>(p);
            } else {
                cout << "error: can't find routing" << endl;
                std::abort();
            }
        }
    } /* ns3dtnbit */ 

} /* ns3 */ 
