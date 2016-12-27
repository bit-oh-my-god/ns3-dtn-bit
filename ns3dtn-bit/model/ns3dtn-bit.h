/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef NS3DTN_BIT_H
#define NS3DTN_BIT_H

#include "./common_header.h"

namespace ns3 {
    namespace ns3dtnbit {

        class DtnApp {  // dtn daemon or dtn agent

            public :
                struct DaemonQueueInfo {

                };

                struct DaemonBundleHeaderInfo {
                    InetSocketAddress info_dest_addr_;
                    uint32_t info_retransmition_count_;
                    dtn_id_t info_original_id_;
                };

                enum RoutingMethod {
                    Epidemic,
                    SprayAndWait 
                };

                enum PacketType {
                    Ack,
                    Hello
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
                    State_2,
                    State_3
                };

                DtnApp ();
                virtual ~DtnApp ();

                // inilialize a bundle and enqueue, waitting for CheckBuffer() to call SendBundleDetail
                void SendBundle(uint32_t dstnode_number, uint32_t payload_size);

                // receive bundle from a socket
                // should check the 'packet type' : 'acknowledge in one connection' 'bundle you should receive' 'antipacket'
                // find why 'this node' receive this bundle, e.g. you receive 'ack bundle', you need find the 'sent bundle' 
                // which causes this 'ack' in your 'queue or vector or array or something', make sure which state you are
                // 
                // note that you also need to warry about the bp connection session which you are in
                // note that you should decouple implementation by calling 'ReceiveBundleDetail'
                void ReceiveBundle(Ptr<socket> socket);

                // call Schedule to call SendBundle and return eventid to DtnApp
                void ScheduleSend(uint32_t dstnode_number, uint32_t payload_size);

                // *******************
                // the interface of bp ip neighbor discovery functionality
                // you can use this in example like :
                // DtnApp_obj->SendHello(source_socket, simulation_end_time, hello_interval, 0)
                void SendHello(Ptr<Socket> socket, double simulation_end_time, Time hello_interval, bool hello_right_now_boolean);
                void ReceiveHello();

                // *******************
                // the interface of bp cancellation functionality
                void Antipacket();

            private :

                void SendBundleDetail();
                void ReceiveBundleDetail();

                // check whether one packet is already in bundle queue
                void IsDuplicated(Ptr<packet> pkt, Ptr<Queue> queue);
                // check your 'bundle queue' buffer and other related buffer periodly
                void CheckBuffer(CheckState check_state);
                // check whether one packet is already in your 'antipacket_queue' by check the 'uni id number of the packet'
                bool IsAntipacketExist(Ptr<packet> pkt);

                // **************
                // data *********
                uint32_t bundles_count_; // bundles
                uint32_t drops_count_; // drops
                double congestion_control_parameter_; //t_c
                Ptr<Node> node_; // m_node
                RunningFlag running_flag_; // m_running
                RoutingMethod routing_method_; // rp
                CongestionControlMethod congestion_control_method_; // cc
                dtn_id_t send_event_id_; // m_sendEvent

                // daemon
                Ptr<Queue> daemon_antipacket_queue_; //m_antipacket_queue
                Ptr<Socket> daemon_socket_handle_; // m_socket
                Ptr<Queue> daemon_mac_queue_; // mac_queue
                vector<Packet> daemon_new_packet_buffer_vec_; // newpkt
                vector<Packet> daemon_retransmission_packet_buffer_vec_; // retxpkt
                // daemon bundle queue
                Ptr<Queue> daemon_bundle_queue_; // m_queue
                uint32_t daemon_bundle_queue_max_; // b_s   
                vector<dtn_time_t> daemon_bundle_queue_time_stamp_vec_; // bundle_ts
                Ptr<Queue> daemon_bundle_sending_tmp_queue_; // m_help_queue
                vector<uint32_t> daemon_bundle_queue_size_vec_; // bundle_size
                vector<dtn_id_t> daemon_bundle_queue_id_vec_; // bundle_seqno
                vector<PacketType> daemon_bundle_queue_packet_type_vec_; // bundle_retx
                vector<InetSocketAddress> daemon_bundle_queue_destination_address_vec_; // bundle_address

                // neighbor
                vector<uint32_t> neighbor_daemon_queue_size_; // b_a     
                uint32_t neighbor_count_; // neighbors
                vector<InetSocketAddress> neighbor_address_vec_; // neighbor_address
                vector<dtn_time_t> neighbor_last_seen_time_vec_; // neighbor_last_seen
                vector<vector<dtn_id_t>> neighbor_hello_id_vec_; // neighbor_hello_bundles
                vector<vector<dtn_id_t>> neighbor_sent_bp_id_vec_; // neighbor_sent_bundle
                vector<vector<dtn_id_t>> neighbor_sent_ap_id_vec_; // neighbor_sent_aps
                vector<vector<dtn_time_t>> neighbor_sent_time_vec_; // neighbor_sent_ap_when

                // retransmission
                vector<uint32_t> daemon_transmition_receive_bytes_vec_; // currentServerRxBytes
                vector<uint32_t> daemon_transmition_total_bytes_vec_; // TotalTxBytes 
                vector<uint32_t> daemon_transmission_sent_bytes_vec_; // currentTxBytes
                uint32_t daemon_flow_count_; // NumFlows

                //vector<InetSocketAddress> daemon_bh_des_address_vec_; // sendTos
                //vector<uint32_t> daemon_bh_retransmition_count_vec_; // retxs
                //vecotr<dtn_id_t> daemon_bh_original_unique_id_vec_; // ids
                // 
                // look at DaemonBundleHeaderInfo
                vector<DaemonBundleHeaderInfo> daemon_bh_info_vec_;
                vector<dtn_time_t> bundle_first_sent_time_vec_; // firstsendtime
                vector<dtn_time_t> bundle_last_sent_time_vec_; // lastsendtime
                vector<uint32_t> bundle_last_sent_bytes_vec_; // lastTxBytes

        };

    } /* ns3dtnbit */ 


}

#endif /* NS3DTN_BIT_H */

