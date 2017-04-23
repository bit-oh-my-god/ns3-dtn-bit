#ifndef DTN_PACKAGE_H
#define DTN_PACKAGE_H

/* this file would define all data presentation of a bundle package
*/

#include "ns3/enum.h"
#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/log.h"
#include "ns3/address-utils.h"
#include "common_header.h"

namespace ns3 {
    namespace ns3dtnbit {

        //enum BundleType {BundlePacket = 0, AntiPacket = 1, HelloPacket = 2, TransmissionAck = 3};
        enum class BundleType : std::uint32_t {
            BundlePacket, AntiPacket, HelloPacket, TransmissionAck, UnKnow};
        std::ostream& operator<<(std::ostream& os, BundleType&& rh);

        class BPHeader : public Header {
            public :
                BPHeader ();
                // inherited items
                static TypeId GetTypeId();
                TypeId GetInstanceTypeId() const;
                uint32_t GetSerializedSize () const;
                void Serialize (Buffer::Iterator start) const;
                uint32_t Deserialize (Buffer::Iterator start);
                void Print (std::ostream &os) const;

                // own stuff *********
                // should not define this, maybe a method named semi_equal() TODO
                //bool operator==(BPHeader const& rh) const;

                // assuming that different BundleType has different payload size, and payload size themselves could be different , used to be todo, now done
                BundleType get_bundle_type() {return bundle_type_;}
                void set_bundle_type(BundleType arg) {bundle_type_ = arg;}
                // bool is_valid() {return is_valid_;}
                // void set_valid(bool arg) {is_valid_ = arg;}
                uint32_t get_hop_count() {return hop_count_;}
                void set_hop_count(uint32_t arg) {hop_count_ = arg;}
                uint32_t get_spray() {return spray_;}
                void set_spray(uint32_t arg) {spray_ = arg;}
                uint32_t get_retransmission_count() {return retransmission_count_;}
                void set_retransmission_count(uint32_t arg) {retransmission_count_ = arg;}
                Ipv4Address get_destination_ip() {return destination_ip_;}
                void set_destination_ip(Ipv4Address arg) {destination_ip_ = arg;}
                Ipv4Address get_source_ip() {return source_ip_;}
                void set_source_ip(Ipv4Address arg) {source_ip_ = arg;}
                dtn_seqno_t get_source_seqno() {return source_seqno_;}
                void set_source_seqno(dtn_seqno_t arg) {source_seqno_ = arg;}
                uint32_t get_payload_size() {return payload_size_;}
                void set_payload_size(uint32_t arg) {payload_size_ = arg;}
                uint32_t get_offset() {return offset_size_;}
                void set_offset(uint32_t arg) {offset_size_ = arg;}
                //Time get_src_time_stamp() {return Time(MilliSeconds(src_time_stamp_));}
                Time get_src_time_stamp() {return Time(Seconds(src_time_stamp_));}
                //void set_src_time_stamp(Time arg) {src_time_stamp_ = arg.GetMilliSeconds();}
                void set_src_time_stamp(Time arg) {src_time_stamp_ = arg.GetSeconds();}
                //Time get_hop_time_stamp() {return Time(Seconds(hop_time_stamp_));}
                Time get_hop_time_stamp() {return Time(Seconds(hop_time_stamp_));}
                //void set_hop_time_stamp(Time arg) {hop_time_stamp_ = arg.GetMilliSeconds();}
                void set_hop_time_stamp(Time arg) {hop_time_stamp_ = arg.GetSeconds();}

            private :

                // bool is_valid_;
                //TODO// this may not be a good design
                BundleType bundle_type_;
                uint32_t hop_count_;
                uint32_t spray_;
                uint32_t retransmission_count_;
                Ipv4Address destination_ip_;
                Ipv4Address source_ip_;
                dtn_seqno_t source_seqno_;
                // payload_size_ can be bigger than avialable 'bundle pkt size', 
                // bigger pkt would be fragment later
                uint32_t payload_size_; 
                // offset = acked bytes + current size
                uint32_t offset_size_;
                dtn_time_t src_time_stamp_;
                dtn_time_t hop_time_stamp_;
        }; 

        std::ostream& operator<< (std::ostream& os, BPHeader const& rh);

    } /* ns3dtnbit */ 

} /* ns3 */ 
#endif
