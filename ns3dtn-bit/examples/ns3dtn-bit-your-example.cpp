/*
 * 1. generate your trace file
 * 2. define your example, mainly how node are connected
 * 3. run your example
 */
#include "ns3/ns3dtn-bit-helper.h"
#include "ns3/ns3dtn-bit-example-interface.h"
#include "ns3/ns3dtn-bit.h"

using namespace ns3;
namespace ns3 {
    namespace ns3dtnbit {
        class YourExample : public DtnExampleInterface {
            public :
                YourExample() : DtnExampleInterface() {
                    node_number_ = 5;
                    print_log_boolean_ = true;
                }
            protected :
                /*
                 * copy from ns3 wifi-adhoc example
                 */
                void CreateDevices() override {
                    WifiHelper wifi;
                    std::string phyMode ("DsssRate1Mbps");
                    double rss = -80;  // -dBm
                    if (print_log_boolean_) {
                        wifi.EnableLogComponents ();  // Turn on all Wifi logging
                    }
                    wifi.SetStandard (WIFI_PHY_STANDARD_80211b);

                    YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
                    // This is one parameter that matters when using FixedRssLossModel
                    // set it to zero; otherwise, gain will be added
                    wifiPhy.Set ("RxGain", DoubleValue (0) ); 
                    // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
                    wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO); 

                    YansWifiChannelHelper wifiChannel;
                    wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
                    // The below FixedRssLossModel will cause the rss to be fixed regardless
                    // of the distance between the two stations, and the transmit power
                    wifiChannel.AddPropagationLoss ("ns3::FixedRssLossModel","Rss",DoubleValue (rss));
                    wifiPhy.SetChannel (wifiChannel.Create ());

                    // Add a mac and disable rate control
                    WifiMacHelper wifiMac;
                    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                            "DataMode",StringValue (phyMode),
                            "ControlMode",StringValue (phyMode));
                    // Set it to adhoc mode
                    wifiMac.SetType ("ns3::AdhocWifiMac");
                    NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, nodes_container_);

                }

                void InstallInternetStack() override {
                    
                }
                void InstallApplications() override {

                }

        };

    } /* ns3dtnbit */ 

} /* ns3  */ 

int main(int argc, char *argv[]) {
    LogComponentEnable ("DtnRunningLog",LOG_LEVEL_DEBUG);
    LogComponentEnableAll (LOG_PREFIX_TIME);
    LogComponentEnableAll(LOG_PREFIX_NODE); 

    assert(std::is_move_constructible<ns3dtnbit::YourExample>::value);
    assert(std::is_move_assignable<ns3dtnbit::YourExample>::value);
    std::unique_ptr<ns3dtnbit::DtnExampleInterface> exp(new ns3dtnbit::YourExample());
    auto runner = ns3dtnbit::DtnExampleRunner();
    runner.RunnerLoad(exp).RunIt(argc, argv);
    return 0;
}
