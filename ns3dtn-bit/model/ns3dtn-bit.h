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
                    Other
                };

                // almost useless
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

                // use BPHeader to compare is an ambiguity, so use this to compare
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
                    bool IsLastSeen();
                };

                DtnApp () : transmit_assister_(*this) { }
                virtual ~DtnApp () { }
                void SetUp(Ptr<Node> node);
                void ScheduleTx(Time tNext, uint32_t dstnode, uint32_t payload_size);
                void ToSendHello(Ptr<Socket> socket, double simulation_end_time, Time hello_interval, bool hello_right_now_boolean);
                void ReceiveBundle(Ptr<Socket> socket);
                void ReceiveHello(Ptr<Socket> socket_handle);
                void Report(std::ostream& os);

            private :
                /*
                 * nested private class, just a implement usage
                 * */
                class DtnAppRoutingAssister {
                    public :
                        DtnAppRoutingAssister() { }
                        void SetIt() { is_init = true; }
                        bool IsSet() {return is_init;}
                        RoutingMethod get_rm() {return rm_;}
                        void set_rm(RoutingMethod rm) {rm_ = rm;}
                        void set_rmob(std::unique_ptr<RoutingMethodInterface> p_rm_in) {p_rm_in_ = std::move(p_rm_in);}
                        void load_ob(const vector<Adob>& v) { vec_ = v; }
                        int RouteIt(int src_node_n, int dest_node_n);
                        ~DtnAppRoutingAssister() { }
                        // some thing for CGR
                        // end of CGR
                        friend RoutingMethodInterface;
                        vector<Adob> vec_;
                        std::unique_ptr<RoutingMethodInterface> p_rm_in_;
                        bool is_init = false;
                        RoutingMethod rm_;
                        // some thing for CGR
                        // end of CGR
                };
                DtnAppRoutingAssister routing_assister_;

                /*
                 * nested private class for transmit-session init 
                 * used frequently in totransmit()
                 * */
                class DtnAppTransmitSessionAssister {
                    public :
                        DtnAppTransmitSessionAssister(DtnApp& dp) : out_app_(dp) { }
                        ~DtnAppTransmitSessionAssister() { }
                        vector<Ptr<Packet>> daemon_retransmission_packet_buffer_vec_;
                        vector<DaemonTransmissionInfo> daemon_transmission_info_vec_;
                        vector<DaemonBundleHeaderInfo> daemon_transmission_bh_info_vec_;
                        int get_need_to_bytes(int index) { return daemon_transmission_info_vec_[index].info_transmission_total_send_bytes_ - daemon_transmission_info_vec_[index].info_transmission_current_sent_acked_bytes_; }
                    private :
                        DtnApp& out_app_;
                };
                DtnAppTransmitSessionAssister transmit_assister_;

                /*
                 * 
                 * */
                class DtnpAppNeighborKeeper {

                };
                DtnpAppNeighborKeeper neighbor_keeper_;
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
                friend RoutingMethodInterface;

            private :
                // control the times of transmit from this node for one identical pkt
                bool SprayGoodDetail(BPHeader& bp_header, int flag);
                // control local2neighbor failed transmit
                void TransmitSessionFailCheck(DaemonBundleHeaderInfo bh_info, int last_current);
                // put anti in queue
                void ToSendAntipacketBundle(BPHeader& ref_bp_header);
                // response for bundle & anti
                void ToSendAck(BPHeader& ref_bp_header, Ipv4Address response_ip);
                // put bundle in queue
                void ToSendBundle(uint32_t dstnode_number, uint32_t payload_size);
                // the behavior is that when receive anti, corresponding bundle would be destroied, anti would remain until expired
                void RemoveBundleFromAntiDetail(Ptr<Packet> p_pkt);
                void StartApplication() override;
                // not used yet
                void PeriodReorderDaemonBundleQueueDetail();
                // send hello
                void CreateHelloBundleAndSendDetail(string msg_str, Ptr<Socket> broad_cast_skt);
                // invoke inside ReceiveBundle
                void BundleReceptionTailWorkDetail();
                // invoke when create a BPHeader
                void SemiFillBPHeaderDetail(BPHeader* p_bp_header);
                // not test yet TODO
                void FragmentReassembleDetail(int k);
                // find all available neighbor
                vector<int> BPHeaderBasedSendDecisionDetail(BPHeader& ref_bp_header, enum CheckState check_state);
                // invoke routing method, and check if result is in available neighbor, if do, decision is done, if not, decision is aborted
                bool FindTheNeighborThisBPHeaderTo(BPHeader& ref_bp_header, int& return_index_of_neighbor_you_dedicate, enum CheckState check_state);
                // invoke when create socket
                void CreateSocketDetail();
                // maintain info
                void UpdateNeighborInfoDetail(int which_info, int which_neighbor, int which_pkt_index);
                // remove expired pkt
                void RemoveExpiredBAQDetail();
                // 
                void ReceiveHelloBundleDetail(Ptr<Packet> p_pkt, std::string msg);
                //
                bool SocketSendDetail(Ptr<Packet> p_pkt, uint32_t flags, InetSocketAddress trans_addr);
                // 
                bool IsDuplicatedDetail(BPHeader& bp_header);
                // log out state of app
                void StateCheckDetail();
                //
                bool IsAntipacketExistDetail();
                // periodly check the state of this app, and check if available pkt can be routing
                void CheckBuffer(CheckState check_state);
                //
                void CheckBufferSwitchStateDetail(bool real_send_boolean, CheckState check_state);
                // after routing decision is made, handle local to neighbor transmit
                // retransmission : For now we don't have local2neighbor retransmission. What we have is that, if one transmission session is not successed, this session would be remained, and reboot next time when routing decision is made by which transmission session producted had the same 'session value', where normally results to a new transmission session. Should we change it?
                void ToTransmit(DaemonBundleHeaderInfo bh_info, bool is_retransmit);
                std::string LogPrefix();
                // to check state of wireless device
                Ptr<WifiPhy> wifi_ph_p;
                // for SprayGoodDetail
                std::map<dtn_seqno_t, int> spray_map_;
                map<dtn_seqno_t, int> seqno2fromid_map_;
                set<dtn_seqno_t> before_receive_seqno_set_;
                map<int, vector<int>> id2cur_exclude_vec_of_id_;    //used for CGR
                int anti_send_count_ = 0;
                int ack_send_count_ = 0;
                int bundle_send_count_ = 0;
                uint32_t daemon_baq_pkts_max_;
                uint32_t drops_count_;
                Ptr<Node> node_; 
                Ipv4Address own_ip_;
                //enum RunningFlag running_flag_;
                enum CongestionControlMethod congestion_control_method_;
                double congestion_control_parameter_; // will only works when enable Dynamic congestion control
                dtn_time_t retransmission_interval_;
                Ptr<Socket> daemon_socket_handle_; // note that hello socket is another socket
                Ptr<Queue> daemon_antipacket_queue_;
                Ptr<Queue> daemon_consume_bundle_queue_; // store the bundle which is aim to be sent to this node
                Ptr<Queue> daemon_reorder_buffer_queue_; // m_helper_queue
                Ptr<Queue> daemon_bundle_queue_; // m_queue, daemon bundle queue, this is where "store and forward" semantic stores
                vector<Ptr<Packet>> daemon_reception_packet_buffer_vec_;
                vector<DaemonReceptionInfo> daemon_reception_info_vec_;
                vector<NeighborInfo> neighbor_info_vec_;
        };

        class RoutingMethodInterface {
            public :
                RoutingMethodInterface(DtnApp& dp);
                virtual ~RoutingMethodInterface();
                // Aim :
                // src is the node number for traffic source node
                // dst is the node number for traffic sink node
                // return the next hop node number
                // Note : 
                // use adob in the out_app_
                virtual int DoRoute(int src, int dst) = 0;
                /*
                 * this time I modify this interface for CGR, next time I would do it again! Change RoutingMethodInterface to Generic!!!
                 * TODO
                 * */
                virtual void GetInfo(int destination_id, int from_id, std::vector<int> vec_of_current_neighbor, int own_id, dtn_time_t expired_time, int bundle_size, int networkconfigurationflag, map<int, vector<int>> id2cur_exclude_vec_of_id, dtn_time_t local_time, dtn_seqno_t that_seqno);
            protected :
                // read only 
                const DtnApp& out_app_;
                Adob get_adob();
        };
    } /* ns3dtnbit */ 
}

#endif /* NS3DTN_BIT_H */
