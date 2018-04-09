/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef NS3DTN_BIT_H
#define NS3DTN_BIT_H

#include "dtn-pre.h"

namespace ns3 {
    namespace ns3dtnbit {

        class RoutingMethodInterface;

        struct Adob;

        class DtnApp : public Application {
            public :

                enum class RoutingMethod {
                    Epidemic,
                    TimeExpanded,
                    SprayAndWait,
                    CGR,
                    DirectForward,
                    QM,
                    Other,
                };

                // DEPRECATED
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
                    Ipv4Address info_transmission_receive_from_ip_;
                    BundleType info_bundle_type_;
                    vector<Ptr<Packet>> info_fragment_pkt_pointer_vec_;
                    DaemonReceptionInfo() : info_bundle_source_ip_(Ipv4Address("10.0.0.55")), info_bundle_destination_ip_(info_bundle_source_ip_), info_transmission_receive_from_ip_(info_bundle_source_ip_) {}
                    DaemonReceptionInfo(uint32_t rec, uint32_t pay, dtn_time_t rec_t, Ipv4Address ip_s, Ipv4Address ip_d, dtn_seqno_t seq, Ipv4Address ip_from, BundleType info_b, vector<Ptr<Packet>> v) : info_bundle_source_ip_(ip_s), info_bundle_destination_ip_(ip_d), info_transmission_receive_from_ip_(ip_from) { }
                };

                // use BPHeader to compare is an ambiguity, so use this to compare
                struct DaemonBundleHeaderInfo {
                    Ipv4Address info_transmit_addr_;
                    dtn_seqno_t info_source_seqno_;
                    bool operator==(DaemonBundleHeaderInfo const & rhs)const  {
                        auto& ipl = info_transmit_addr_;
                        auto& ipr = rhs.info_transmit_addr_;
                        if (ipl.IsEqual(ipr)  && info_source_seqno_ == rhs.info_source_seqno_) {
                            return true;
                        } else {
                            return false;
                        }
                    }
                    bool operator<(DaemonBundleHeaderInfo const & rhs) const {
                        if (info_transmit_addr_.Get() < rhs.info_transmit_addr_.Get()) {
                            return true;
                        } else if (info_transmit_addr_.Get() == rhs.info_transmit_addr_.Get()){
                            return info_source_seqno_ < rhs.info_source_seqno_;
                        } else {
                            return false;
                        }
                    }
                    DaemonBundleHeaderInfo(Ipv4Address ipaddr, dtn_seqno_t seq) : info_transmit_addr_(ipaddr), info_source_seqno_(seq) {}
                };

                struct DaemonTransmissionInfo {
                    uint32_t info_transmission_total_send_bytes_;
                    uint32_t info_transmission_current_sent_acked_bytes_;
                    dtn_time_t info_transmission_bundle_first_sent_time_;
                    dtn_time_t info_transmission_bundle_last_sent_time_;
                    uint32_t info_transmission_bundle_last_sent_bytes_;
                    vector<Ptr<Packet>> info_transmission_pck_buffer_;
                    uint32_t info_retransmission_count_;
                };

                struct NeighborInfo {
                    uint32_t info_daemon_baq_available_bytes_;
                    dtn_time_t info_last_seen_time_;
                    bool IsLastSeen();
                    NeighborInfo() { }
                    NeighborInfo(uint32_t i, dtn_time_t t) : info_daemon_baq_available_bytes_(i), info_last_seen_time_(t) {}
                };

                DtnApp () : routing_assister_(*this), transmit_assister_(*this), neighbor_keeper_(*this), lower_layer_adaptor_(*this) {}
                virtual ~DtnApp () { }
                void SetUp(Ptr<Node> node);
                void ScheduleTx(Time tNext, uint32_t dstnode, uint32_t payload_size);
                void ToSendHello(Ptr<Socket> socket, double simulation_end_time, Time first_time);
                void ReceiveBundle(Ptr<Socket> socket);
                void ReceiveHello(Ptr<Socket> socket_handle);
                void Report(std::ostream& os);
                int GetNodeId() const {return node_->GetId();}
                std::string LogPrefix();

            private :
                /*
                 * nested private class, just a implement usage, note a nested class is friend of outer class
                 * */
                struct DtnAppRoutingAssister {
                    DtnAppRoutingAssister(DtnApp& p) : out_app_(p) { std::srand(rand_seed); }
                    ~DtnAppRoutingAssister() {}
                    void SetIt() { is_init = true; }
                    bool IsSet() {return is_init;}
                    RoutingMethod get_rm() {return rm_;}
                    void set_rm(RoutingMethod rm) {rm_ = rm;}
                    void set_rmob(std::unique_ptr<RoutingMethodInterface> p_rm_in) {p_rm_in_ = std::move(p_rm_in);}
                    void load_ob(const vector<Adob>& v) { vec_ = v; }
                    int RouteIt(int src_node_n, int dest_node_n);
                    int FindTheNeighborThisBPHeaderTo(BPHeader& ref_bp_header);
                    std::string LogPrefix() {return out_app_.LogPrefix();}
                    bool IsRouteMethodQM(){return rm_ == RoutingMethod::QM;}
                    friend RoutingMethodInterface;
                    // CGRQM TODO
                    void StorageinfoMaintainInterface(string s 
                            ,map<int, pair<int, int>> parsed_storageinfo_from_neighbor
                            ,map<int, pair<int, int>>& move_storageinfo_to_this
                            ,map<int, int> storagemax
                            ,vector<int> path_of_route
                        ,pair<int, int> update ={ -1, -1 }
                            );

                    // data
                    vector<Adob> vec_;
                    std::unique_ptr<RoutingMethodInterface> p_rm_in_;
                    bool is_init = false;
                    RoutingMethod rm_;
                    private :
                    map<dtn_seqno_t, int> shall_wait_map_;
                    bool ShallWait();
                    int rand_seed = std::time(0);
                    DtnApp& out_app_;
                };
                DtnAppRoutingAssister routing_assister_;

                /*
                 * nested private class for transmit-session init 
                 * */
                struct DtnAppTransmitSessionAssister {
                    DtnAppTransmitSessionAssister(DtnApp& dp) : out_app_(dp) { }
                    ~DtnAppTransmitSessionAssister() { }

                    // Transmit
                    void InitTransmission(Ipv4Address nei_ip, BPHeader bp_header, bool& is_exist);
                    void ToTransmit(DaemonBundleHeaderInfo bh_info);
                    void TransmitSessionFailCheck(DaemonBundleHeaderInfo bh_info, int last_current);

                    // Receive
                    void ReceiveBundleDetail(Ptr<Socket>& socket);
                    void BundleReceptionTailWorkDetail(DaemonBundleHeaderInfo tmp_header_info);

                    // Duplicate
                    bool NotDuplicatedBundle() {return !this_dup_;};
                    void DuplicatedBundleGuard() {this_dup_ = true;}
                    void ThisIsNotDup() {this_dup_ = false;}

                    // Ack
                    void ToSendAckDetail(BPHeader& ref_bp_header, Ipv4Address response_ip);

                    // Raw send
                    bool SocketSendDetail(Ptr<Packet> p_pkt, uint32_t flags, InetSocketAddress trans_addr);
                    std::string LogPrefix() {return out_app_.LogPrefix();}

                    // data
                    map<DaemonBundleHeaderInfo, DaemonTransmissionInfo> daemon_transmission_info_map_;
                    map<DaemonBundleHeaderInfo, DaemonReceptionInfo> daemon_reception_info_map_;
                    private :
                    bool this_dup_ = false;
                    DtnApp& out_app_;
                };
                DtnAppTransmitSessionAssister transmit_assister_;

                /*
                 * 
                 * */
                struct DtnAppNeighborKeeper {
                    DtnAppNeighborKeeper(DtnApp& dp) : out_app_(dp) {}
                    ~DtnAppNeighborKeeper() {}
                    void SendHelloDetail(Ptr<Socket> socket);
                    string HelloContent();
                    // send hello
                    vector<Ipv4Address> PackageStillNeighborAvailableDetail(BPHeader& ref_bp_header);
                    void ReceiveHelloDetail(Ptr<Socket>& socket_handle);
                    pair<bool, Ipv4Address> HasNewNeighbor();
                    std::string LogPrefix() {return out_app_.LogPrefix();}
                    bool CheckBufferTimePass() {
                        if (Simulator::Now().GetSeconds() - last_check_buffer_time_.GetSeconds() > NS3DTNBIT_BUFFER_CHECK_INTERVAL) {
                            last_check_buffer_time_ = Simulator::Now();
                            return true;
                        } else {
                            return false;
                        }
                    }

                    // data
                    map<Ipv4Address, NeighborInfo> neighbor_info_map_;
                    private :
                    Time last_check_buffer_time_ = Simulator::Now();
                    set<Ipv4Address> cur_neighbor_;
                    DtnApp& out_app_;
                };
                DtnAppNeighborKeeper neighbor_keeper_;

                struct DtnAppLowLayerAdaptor {
                    DtnAppLowLayerAdaptor (DtnApp& p) : out_app_(p) {}
                    virtual ~DtnAppLowLayerAdaptor (){}

                    std::string LogPrefix() {return out_app_.LogPrefix();}
                    private:
                    DtnApp& out_app_;
                };
                DtnAppLowLayerAdaptor lower_layer_adaptor_;

            public :

                void SetQueueParameter(int v) {
                    daemon_baq_pkts_max_ = v;
                }

                void SetId2cur(map<int, vector<int>> tmpmap) {
                    id2cur_exclude_vec_of_id_ = tmpmap;
                }

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
                friend RoutingMethodInterface;

            private :
                // control the times of transmit from this node for one identical pkt
                bool ReplicationGoodDetail(BPHeader& bp_header, int flag);
                // put bundle in queue
                void ToSendBundle(uint32_t dstnode_number, uint32_t payload_size);
                //
                void StartApplication() override;
                // invoke when create a BPHeader
                void SemiFillBPHeaderDetail(BPHeader* p_bp_header);
                //
                void ReactMain(string s);
                // invoke when create socket
                void CreateSocketDetail();
                // 
                bool IsDuplicatedDetail(BPHeader& bp_header);
                // log out state of app
                void StateCheckDetail();
                // periodly check the state of this app, remove expired package
                void CheckBuffer(CheckState check_state);
                // to check state of wireless device
                Ptr<WifiPhy> wifi_ph_p;
                // for ReplicationGoodDetail
                bool hello_schedule_flag_ = false;
                std::map<dtn_seqno_t, int> spray_map_;
                map<dtn_seqno_t, int> seqno2fromid_map_;
                set<dtn_seqno_t> before_receive_seqno_set_;
                map<int, vector<int>> id2cur_exclude_vec_of_id_;    //used for CGR
                uint32_t daemon_baq_pkts_max_ = 11111;
                Ptr<Node> node_; 
                Ipv4Address own_ip_;
                //enum RunningFlag running_flag_;
                Ptr<Socket> daemon_socket_handle_; // note that hello socket is another socket
                Ptr<Queue> daemon_antipacket_queue_;
                Ptr<Queue> daemon_consume_bundle_queue_; // store the bundle which is aim to be sent to this node
                Ptr<Queue> daemon_bundle_queue_; // m_queue, daemon bundle queue, this is where "store and forward" semantic stores
        };

    } /* ns3dtnbit */ 
}

#endif /* NS3DTN_BIT_H */
