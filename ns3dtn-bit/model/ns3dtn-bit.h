/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef NS3DTN_BIT_H
#define NS3DTN_BIT_H

#include "dtn_package.h"

namespace ns3 {
    namespace ns3dtnbit {

        class DtnApp {  // dtn daemon or dtn agent

            public :
                
                enum PacketTagType {};

                enum RoutingMethod {
                    Epidemic,
                    SprayAndWait 
                };

                enum RunningFlag {
                    PowerOn,
                    PowerOff
                };

                enum CongestionControlMethod {
                    NoControl,
                    StaticControl,
                    DynamicControl
                };

                enum CheckState {
                    State_0,
                    State_1,
                    State_2
                };

                struct DaemonReceiveInfo {
                    uint32_t info_daemon_received_bytes_;
                    uint32_t info_bundle_should_receive_bytes_;
                    dtn_time_t info_bundle_time_stamp_;
                    dtn_seqno_t info_bundle_seqno_;
                    InetSocketAddress info_bundle_addr_;
                    BundleType info_bundle_type_;
                };

                struct DaemonBundleHeaderInfo {
                    InetSocketAddress info_dest_addr_;
                    uint32_t info_retransmission_count_;
                    dtn_seqno_t info_source_seqno_;
                };

                struct DaemonTransmissionInfo {
                    uint32_t info_transmission_received_bytes_;
                    uint32_t info_transmission_total_send_bytes_;
                    uint32_t info_transmission_current_sent_bytes_;
                    dtn_time_t info_transmission_bundle_first_sent_time_;
                    dtn_time_t info_transmission_bundle_last_sent_time_;
                    uint32_t info_transmission_bundle_last_sent_bytes_;
                };

                struct NeighborInfo {
                    InetSocketAddress info_address_;
                    uint32_t info_daemon_baq_available_bytes_;
                    dtn_time_t info_last_seen_time_;
                    vector<dtn_seqnof_t> info_baq_seqnof_vec_;
                    vector<dtn_seqnof_t> info_sent_bp_seqnof_vec_;
                    vector<dtn_seqnof_t> info_sent_ap_seqnof_vec_;
                    vecotr<dtn_time_t> info_sent_ap_time_vec_;
                };

                DtnApp ();
                virtual ~DtnApp ();
                /* setup
                 */
                void SetUp(Ptr<Node> node);

                /* start app
                 */
                void StartApplication();
                /* stop app
                 */
                void StopApplication();

                /* create a bundle and enqueue, waitting for CheckBuffer() to call SendBundleDetail
                 */
                void ToSendBundle(uint32_t dstnode_number, uint32_t payload_size);

                /* receive bundle from a socket
                 * should check the 'packet type' : 'acknowledge in one connection' 'bundle you should receive' 'antipacket'
                 * find why 'this node' receive this bundle, e.g. you receive 'ack bundle', you need find the 'sent bundle' 
                 * which causes this 'ack' in your 'queue or vector or array or something', make sure which state you are
                 * 
                 * note that you also need to warry about the bp connection session which you are in
                 * note that you should decouple implementation by calling 'ReceiveBundleDetail'
                 */
                void ReceiveBundle(Ptr<socket> socket);

                // call Schedule to call SendBundle and return eventid to DtnApp
                void ScheduleSend(uint32_t dstnode_number, uint32_t payload_size);

                /* the interface of bp ip neighbor discovery functionality
                 * broadcast, 
                 * notify msg : how many bytes you can receive
                 * reorder the packet sequence with daemon_hello_send_buffer_queue_ then
                 * notify msg : how many bundle you already have
                 * notyfy msg : the source unique seqno of all pkt in queue and all in antiqueue
                 * then use this msg to send 'socket raw packet' without header really ? // TODO
                 */
                void ToSendHello(Ptr<Socket> socket, double simulation_end_time, Time hello_interval, bool hello_right_now_boolean);

                /* check addr of hello pkt, if new neighbor create new one
                 * then update neighbor_daemon_baq_avilable_bytes_ & neighbor_hello_neighbor_baq_seqno_vec_
                 */
                void ReceiveHello(Ptr<Socket> socket_handle);

                /* the interface of bp cancellation functionality
                 * send anti
                 */
                void ToSendAntipacketBundle(Ipv4Address scraddr,Ipv4Address dstaddr, dtn_seqno_t bundle_seqno, dtn_time_t src_time_stamp);
                
                /* this func would be invoked only in ReceiveBundle()
                 * it would find the pkt in daemon_bundle_queue_, then dequeue it 
                 * and update neighbor_sent_bp_seqno_vec_ & daemon_transmission_bh_info_vec_
                 */
                void RemoveBundle(Ptr<Packet> pkt);

            private :

                void ReorderDaemonBundleQueueDetail();
                void CreateHelloBundleAndSendDetail(string msg_str);
                void CreateSocketDetail();
                void RemoveExpiredBAQDetail();
                void ReceiveHelloBundleDetail(Ptr<Packet> p_pkt, std::string msg);
                void SocketSendDetail(Ptr<Packet> p_pkt, uint32_t flags, const Address& dst_addr);
                /* check whether one packet is already in bundle queue
                 */
                void IsDuplicated(Ptr<packet> pkt, Ptr<Queue> queue);
                /* check your 'bundle queue' buffer and other related buffer periodly
                 * make code refactory to this 
                 */
                void CheckBuffer(enum CheckState check_state);
                /* check whether one packet is already in your 'antipacket_queue' 
                 * by check the 'uni seqno number of the packet'
                 */
                bool IsAntipacketExist(Ptr<packet> pkt);
                /* log for analisis
                 */
                void LogPrint();

                void UpdateNeighborInfo(int which_info, int which_neighbor, int which_pkt_index);
                 
                void ToTransmit(struct DaemonBundleHeaderInfo bh_info);

                /* if total send bytes > current send bytes
                 */
                void ToSendMore(struct DaemonBundleQueueInfo bh_info);

                void PowerOn();

                void PowerOff();

                // uint32_t bundles_count_; // bundles you can use daemon_reception_info_vec_.size()
                uint32_t drops_count_; // drops
                Ptr<Node> node_; // m_node
                uint32_t daemon_flow_count_; // NumFlows
                enum RunningFlag running_flag_; // m_running
                enum RoutingMethod routing_method_; // rp
                enum CongestionControlMethod congestion_control_method_; // cc
                double congestion_control_parameter_ = 1.0; //t_c     // will only works when enable Dynamic congestion control
                EventId send_event_id_; // m_sendEvent

                /* daemon
                 */
                Ptr<Socket> daemon_socket_handle_; // m_socket
                uint32_t daemon_baq_bytes_max_; // b_s   
                Ptr<Queue> daemon_antipacket_queue_; //m_antipacket_queue
                Ptr<Queue> daemon_mac_queue_; // mac_queue
                Ptr<Queue> daemon_hello_send_buffer_queue_; // m_help_queue
                vector<Ptr<Packet>> daemon_retransmission_packet_buffer_vec_; // retxpkt
                /* daemon bundle queue, this is where "store and forward" semantic stores
                 */
                Ptr<Queue> daemon_bundle_queue_; // m_queue
                vector<Ptr<Packet>> daemon_reception_packet_buffer_vec_; // newpkt
                /* vector<uint32_t> daemon_receive_bytes_vec_; // currentServerRxBytes // fragment probablly
                 * vector<uint32_t> daemon_bundle_receive_size_vec_; // bundle_size
                 * vector<dtn_time_t> daemon_bundle_receive_time_stamp_vec_; // bundle_ts
                 * vector<dtn_seqno_t> daemon_bundle_receive_seqno_vec_; // bundle_seqno
                 * vector<InetSocketAddress> daemon_bundle_receive_destination_address_vec_; // bundle_address
                 * vector<BundleType> daemon_bundle_receive_packet_tag_type_vec_; // bundle_retx
                 * 
                 * @DaemonReceptionInfo
                 */
                vector<struct DaemonReceptionInfo> daemon_reception_info_vec_;

                /* neighbor
                 * uint32_t neighbor_count_; // neighbors  // we don't need it since we can do neighbor_info_vec_.size()
                 * vector<InetSocketAddress> neighbor_address_vec_; // neighbor_address
                 * baq means bundle and antipacket queue
                 * vector<uint32_t> neighbor_daemon_baq_avilable_bytes_; // b_a     
                 * vector<dtn_time_t> neighbor_last_seen_time_vec_; // neighbor_last_seen
                 * vector<vector<dtn_seqnof_t>> neighbor_hello_neighbor_baq_seqnof_vec_; // neighbor_hello_bundles
                 * vector<vector<dtn_seqnof_t>> neighbor_sent_bp_seqnof_vec_; // neighbor_sent_bundle
                 * vector<vector<dtn_seqnof_t>> neighbor_sent_ap_seqnof_vec_; // neighbor_sent_aps
                 * vector<vector<dtn_time_t>> neighbor_sent_ap_time_vec_; // neighbor_sent_ap_when
                 * 
                 * @NeighborInfo
                 */
                vector<struct NeighborInfo> neighbor_info_vec_;

                /* retransmission and transmission
                 * vector<uint32_t> daemon_transmission_send_total_bytes_vec_; // TotalTxBytes 
                 * vector<uint32_t> daemon_transmission_current_sent_bytes_vec_; // currentTxBytes
                 * vector<dtn_time_t> daemon_transmission_bundle_first_sent_time_vec_; // firstsendtime
                 * vector<dtn_time_t> daemon_transmission_bundle_last_sent_time_vec_; // lastsendtime
                 * vector<uint32_t> daemon_transmission_bundle_last_sent_bytes_vec_; // lastTxBytes
                 * 
                 * @DaemonTransmissionInfo
                 */
                vector<struct DaemonTransmissionInfo> daemon_transmission_info_vec_;
                /* vector<InetSocketAddress> daemon_bh_des_address_vec_; // sendTos
                 * vector<uint32_t> daemon_bh_retransmission_count_vec_; // retxs
                 * vecotr<dtn_seqno_t> daemon_bh_source_unique_seqno_vec_; // ids
                 * 
                 * @DaemonBundleHeaderInfo
                 */
                vector<struct DaemonBundleHeaderInfo> daemon_transmission_bh_info_vec_;
        };
    } /* ns3dtnbit */ 
}

#endif /* NS3DTN_BIT_H */
