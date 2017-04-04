/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef NS3DTN_BIT_H
#define NS3DTN_BIT_H

#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h" 
#include "ns3/v4ping-helper.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/double.h"
#include "ns3/random-variable-stream.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ns2-mobility-helper.h"
#include "ns3/qos-utils.h"
#include "ns3/log.h"
#include "ns3/test.h"
#include "dtn_package.h"
#include "common_header.h"

#ifdef DEBUG
std::string GetCallStack(int);
std::string FilePrint(std::string);
std::string GetLogStr(std::string);

#endif /* ifndef DEBUG */

namespace ns3 {
    namespace ns3dtnbit {

        class RoutingMethodInterface;
        
        class DtnApp : public Application {

            public :
                struct Adob {
                    struct edge_property {
                        edge_property() {
                            distance_ = -1;
                        }
                        edge_property(int v) {
                            distance_ = v;
                        }
                        int distance_; 
                    };

                    using EdgeProperties = edge_property;
                    using NameProperties = boost::property<boost::vertex_name_t, std::string>;
                    using Graph = boost::adjacency_list < boost::vecS, boost::vecS, boost::directedS, NameProperties, EdgeProperties, boost::no_property>;
                    using VeDe = boost::graph_traits < Graph >::vertex_descriptor;
                    using EdDe = boost::graph_traits < Graph >::edge_descriptor;

                    Adob() {}
                    Adob(std::map<int, vector<vector<int>>> t_2_adjacent_array, int node_number) {
                        // boost code
                        for (auto t2 : t_2_adjacent_array) {
                            auto time_index = get<0>(t2);
                            auto t3 = get<1>(t2);

                            Graph my_g;
                            vector<VeDe> vec_vertex_des;
                            // add node & node NameProperties
                            for (int i = 0; i < node_number; i++) {
                                std::stringstream ss;
                                ss << "node-" << i;
                                vec_vertex_des.push_back(add_vertex(ss.str(), my_g));
                            }
                            //auto vec_edge_des = std::vector<vector<EdDe>>(node_number_, std::vector<EdDe>(node_number_, EdDe()));
                            for (int i = 0; i < node_number; ++i) {
                                for (int j = 0; j < node_number; ++j) {
                                    if (i == j) {break;}
                                    add_edge(vec_vertex_des[i], vec_vertex_des[j], EdgeProperties(t3[i][j]), my_g);
                                    auto tedp = EdgeProperties(t3[i][j]);
                                    std::cout << "edge_property of " << i << "and" << j 
                                    << "=" << tedp.distance_ << std::endl;
                                }
                            }
                            // load it
                            t_vec_.push_back(time_index);
                            g_vec_.push_back(my_g);
                        }
                    }
                    ~Adob() {}
                    private :
                    vector<Graph> g_vec_;
                    vector<dtn_time_t> t_vec_;
                };

                enum class RoutingMethod {
                    Epidemic,
                    SprayAndWait,
                    Other
                };

                enum class RunningFlag {
                    PowerOn,
                    PowerOff
                };

                enum class CongestionControlMethod {
                    NoControl,
                    StaticControl,
                    DynamicControl
                };

                enum class CheckState {
                    // lower layer is busy
                    State_0,
                    // lower layer is not busy, check bundle-pkt
                    State_1,
                    // lower layer is not busy, check anti-pkt
                    State_2
                };

                struct DaemonReceptionInfo {
                    uint32_t info_daemon_received_bytes_;
                    uint32_t info_bundle_should_receive_bytes_;
                    dtn_time_t info_last_received_time_stamp_;
                    Ipv4Address info_bundle_source_ip_;
                    Ipv4Address info_bundle_destination_ip_;
                    dtn_seqno_t info_bundle_seqno_;
                    Ipv4Address info_trasmission_receive_from_ip_;
                    BundleType info_bundle_type_;
                    vector<Ptr<Packet>> info_fragment_pkt_pointer_vec_;
                };

                // use BPHeader to compare is an ambiguity
                struct DaemonBundleHeaderInfo {
                    InetSocketAddress info_transmit_addr_;
                    uint32_t info_retransmission_count_;
                    dtn_seqno_t info_source_seqno_;
                    bool operator==(struct DaemonBundleHeaderInfo& rhs) {
                        auto ipl = info_transmit_addr_.GetIpv4();
                        auto ipr = rhs.info_transmit_addr_.GetIpv4();
                        return (ipl.IsEqual(ipr) && info_retransmission_count_ == rhs.info_retransmission_count_ && info_source_seqno_ == rhs.info_source_seqno_);
                    }
                };

                struct DaemonTransmissionInfo {
                    uint32_t info_transmission_total_send_bytes_;
                    uint32_t info_transmission_current_sent_acked_bytes_;
                    dtn_time_t info_transmission_bundle_first_sent_time_;
                    dtn_time_t info_transmission_bundle_last_sent_time_;
                    uint32_t info_transmission_bundle_last_sent_bytes_;
                };

                struct NeighborInfo {
                    InetSocketAddress info_address_;
                    uint32_t info_daemon_baq_available_bytes_;
                    dtn_time_t info_last_seen_time_;
                    vector<dtn_seqno_t> info_baq_seqno_vec_;
                    vector<dtn_seqno_t> info_sent_bp_seqno_vec_;
                    vector<dtn_seqno_t> info_sent_ap_seqno_vec_;
                    vector<dtn_time_t> info_sent_ap_time_vec_;
                };

                DtnApp () : transmit_assister_(*this) {

                }

                virtual ~DtnApp () {

                }
                void SetUp(Ptr<Node> node);
                void ScheduleTx(Time tNext, uint32_t dstnode, uint32_t payload_size);
                /* create a bundle and enqueue, waitting for CheckBuffer() to call SendBundleDetail
                */
                void ToSendBundle(uint32_t dstnode_number, uint32_t payload_size);
                void ToSendAck(BPHeader& ref_bp_header, Ipv4Address response_ip);

                /* receive bundle from a socket
                 * should check the 'packet type' : 'acknowledge in one connection' 'bundle you should receive' 'antipacket'
                 * find why 'this node' receive this bundle, e.g. you receive 'ack bundle', you need find the 'sent bundle' 
                 * which causes this 'ack' in your 'queue or vector or array or something', make sure which state you are
                 * 
                 * note that you also need to warry about the bp connection session which you are in
                 * note that you should decouple implementation by calling 'ReceiveBundleDetail'
                 */
                void ReceiveBundle(Ptr<Socket> socket);

                /* the interface of bp ip neighbor discovery functionality
                 * broadcast, 
                 * notify msg : how many bytes you can receive
                 * reorder the packet sequence with daemon_reorder_buffer_queue_ then
                 * notify msg : how many bundle you already have
                 * notyfy msg : the source unique seqno of all pkt in queue and all in antiqueue
                 * then use this msg to send 'socket raw packet' without header really ? // used todo, now done
                 */
                void ToSendHello(Ptr<Socket> socket, double simulation_end_time, Time hello_interval, bool hello_right_now_boolean);

                /* check addr of hello pkt, if new neighbor create new one
                 * then update neighbor_daemon_baq_avilable_bytes_ & neighbor_hello_neighbor_baq_seqno_vec_
                 */
                void ReceiveHello(Ptr<Socket> socket_handle);

                void Report(std::ostream& os);


            private :
                // define one method interface class
                /*
                 * nested private class, just a implement usage
                 * not implement yet, should used to hold user define, routing method.
                 * */
                class DtnAppRoutingAssister {
                    public :
                        DtnAppRoutingAssister() {

                        }

                        void SetIt() { is_init = true; }
                        bool IsSet() {return is_init;}
                        RoutingMethod get_rm() {return rm_;}
                        void set_rm(RoutingMethod rm) {rm_ = rm;}
                        void set_rmob(std::unique_ptr<RoutingMethodInterface> p_rm_in) {p_rm_in_ = std::move(p_rm_in);}
                        void load_ob(const vector<DtnApp::Adob>& v) {
                            vec_ = v;
                            adob_cur_ = vec_[0];
                        }

                        void RouteIt();

                        ~DtnAppRoutingAssister() {

                        }
                    private :
                        friend RoutingMethodInterface;
                        vector<Adob> vec_;
                        std::unique_ptr<RoutingMethodInterface> p_rm_in_;
                        Adob adob_cur_;
                        bool is_init = false;
                        RoutingMethod rm_;
                };

                DtnAppRoutingAssister routing_assister_;

                /*
                 * nested private class for transmit-session init 
                 * used frequently in to transmit
                 * */
                class DtnAppTransmitSessionAssister {
                    public :
                        DtnAppTransmitSessionAssister(DtnApp& dp) : out_app_(dp) {

                        }

                        //InitTransmitSession

                        ~DtnAppTransmitSessionAssister() {

                        }
                    private :
                        DtnApp& out_app_;

                };
                DtnAppTransmitSessionAssister transmit_assister_;

            public :

                bool InvokeMeWhenInstallAppToSetupDtnAppRoutingAssister(RoutingMethod rm, vector<Adob>& adob) {
                    routing_assister_.set_rm(rm);
                    routing_assister_.SetIt();
                    routing_assister_.load_ob(adob);
                    return true;
                };

                bool InvokeMeWhenInstallAppToSetupDtnAppRoutingAssister(RoutingMethod rm, std::unique_ptr<RoutingMethodInterface> p_rm_in, vector<Adob>& adob) {
                    routing_assister_.set_rm(rm);
                    routing_assister_.set_rmob(std::move(p_rm_in));
                    routing_assister_.SetIt();
                    routing_assister_.load_ob(adob);
                    return true;
                };

            private :
                void ToSendAntipacketBundle(BPHeader& ref_bp_header);
                void RemoveBundleFromAntiDetail(Ptr<Packet> p_pkt);
                void StartApplication() override;
                void PeriodReorderDaemonBundleQueueDetail();
                void CreateHelloBundleAndSendDetail(string msg_str, Ptr<Socket> broad_cast_skt);
                void BundleReceptionTailWorkDetail();
                void SemiFillBPHeaderDetail(BPHeader* p_bp_header);
                void FragmentReassembleDetail(int k);
                bool BPHeaderBasedSendDecisionDetail(BPHeader& ref_bp_header, int& return_index_of_neighbor, enum CheckState check_state);
                bool FindTheNeighborThisBPHeaderTo(BPHeader& ref_bp_header, int& return_index_of_neighbor_you_dedicate, enum CheckState check_state);
                void CreateSocketDetail();
                void UpdateNeighborInfoDetail(int which_info, int which_neighbor, int which_pkt_index);
                void RemoveExpiredBAQDetail();
                void ReceiveHelloBundleDetail(Ptr<Packet> p_pkt, std::string msg);
                bool SocketSendDetail(Ptr<Packet> p_pkt, uint32_t flags, InetSocketAddress trans_addr);
                bool IsDuplicatedDetail(BPHeader& bp_header);
                void StateCheckDetail();
                bool IsAntipacketExistDetail();
                void CheckBuffer(CheckState check_state);
                void CheckBufferSwitchStateDetail(bool real_send_boolean, CheckState check_state);
                void ToTransmit(DaemonBundleHeaderInfo bh_info, bool is_retransmit);
                std::string LogPrefix();

                Ptr<WifiPhy> wifi_ph_p;
                int anti_send_count_ = 0;
                int ack_send_count_ = 0;
                int bundle_send_count_ = 0;
                uint32_t daemon_baq_bytes_max_; // b_s   
                uint32_t drops_count_; // drops
                Ptr<Node> node_; // m_node
                Ipv4Address own_ip_;
                uint32_t daemon_flow_count_; // NumFlows
                enum RunningFlag running_flag_; // m_running
                enum CongestionControlMethod congestion_control_method_; // cc
                double congestion_control_parameter_; //t_c     // will only works when enable Dynamic congestion control
                dtn_time_t retransmission_interval_;
                //EventId send_event_id_; // m_sendEvent

                Ptr<Socket> daemon_socket_handle_; // m_socket, note that hello socket is another socket
                Ptr<Queue> daemon_antipacket_queue_; //m_antipacket_queue
                Ptr<Queue> daemon_consume_bundle_queue_; // store the bundle which is aim to be sent to this node
                Ptr<Queue> daemon_reorder_buffer_queue_; // m_helper_queue
                Ptr<Queue> daemon_bundle_queue_; // m_queue, daemon bundle queue, this is where "store and forward" semantic stores
                vector<Ptr<Packet>> daemon_reception_packet_buffer_vec_; // newpkt
                vector<Ptr<Packet>> daemon_retransmission_packet_buffer_vec_; // retxpkt
                vector<DaemonReceptionInfo> daemon_reception_info_vec_;
                vector<NeighborInfo> neighbor_info_vec_;
                vector<DaemonTransmissionInfo> daemon_transmission_info_vec_;
                vector<DaemonBundleHeaderInfo> daemon_transmission_bh_info_vec_;
                friend RoutingMethodInterface;
        };

        class RoutingMethodInterface {
            public :
            RoutingMethodInterface(DtnApp& dp) : out_app_(dp) {}
            virtual ~RoutingMethodInterface() {}
            virtual void DoRoute() {
                std::cout << "DoRoute, Error : empty ! abort!" << std::endl;
                std::abort();
            }
            private :
            // can only read
            const DtnApp& out_app_;
        };

    } /* ns3dtnbit */ 
}

#endif /* NS3DTN_BIT_H */
