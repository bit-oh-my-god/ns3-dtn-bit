/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef NS3DTN_BIT_H
#define NS3DTN_BIT_H

namespace ns3 {
    namespace ns3dtnbit {
        class dtnapp {  // dtn daemon or dtn agent
            public:
                dtnapp ();
                virtual ~dtnapp ();

                // inilialize a bundle and enqueue, waitting for CheckBuffer() to call SendBundle_Pri
                void SendBundle(uint32_t dstnode_number, uint32_t payload_size);

                // receive bundle from a socket
                // should check the 'packet type' : 'acknowledge in one connection' 'bundle you should receive' 'antipacket'
                // find why 'this node' receive this bundle, e.g. you receive 'ack bundle', you need find the 'sent bundle' 
                // which causes this 'ack' in your 'queue or vector or array or something', make sure which state you are
                // 
                // note that you also need to warry about the bp connection session which you are in
                // note that you should decouple implementation by calling 'ReceiveBundle_Pri'
                void ReceiveBundle(Ptr<socket> socket);

                // call Schedule to call SendBundle and return eventid to dtnapp
                void ScheduleSend(uint32_t dstnode_number, uint32_t payload_size);
                // the interface of bp ip neighbor discovery functionality
                
                // ******************************
                // you can use this in example like :
                // dtnapp_obj->SendHello(source_socket, simulation_end_time, hello_interval, 0)
                void SendHello(Ptr<Socket> socket, double simulation_end_time, Time hello_interval, bool flag_hello_right_now);
                void ReceiveHello();
                // the interface of bp cancellation functionality
                void Antipacket();
                
            private:

                enum CheckState { state_1, state_2, state_3 };

                void SendBundle_Pri();
                void ReceiveBundle_Pri();

                // check whether one packet is already in bundle queue
                void IsDuplicated(Ptr<packet> pkt, Ptr<Queue> queue);
                // check your 'bundle queue' buffer and other related buffer periodly
                void CheckBuffer(CheckState check_state);
                // check whether one packet is already in your 'antipacket_queue' by check the 'uni id number of the packet'
                bool IsAntipacketExist(Ptr<packet> pkt)

                uint32_t bundles_this_carries;
                vector<uint32_t> bundle_services_rx_bytes_vector;
                Ptr<Queue> bundle_waiting_queue;
                Ptr<Queue> antipacket_queue;
        };

    } /* ns3dtnbit */ 


}

#endif /* NS3DTN_BIT_H */

