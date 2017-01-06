#ifndef DTN_PACKAGE_H
#define DTN_PACKAGE_H

/* this file would define all data presentation of a bundle package
 * 

 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1  
 Primary Bundle Block
 +----------------+----------------+----------------+----------------+
 |    Version     |                  Proc. Flags (*)                 |
 +----------------+----------------+----------------+----------------+
 |                          Block length (*)                         |
 +----------------+----------------+---------------------------------+
 |   Destination scheme offset (*) |     Destination SSP offset (*)  |
 +----------------+----------------+----------------+----------------+
 |      Source scheme offset (*)   |        Source SSP offset (*)    |
 +----------------+----------------+----------------+----------------+
 |    Report-to scheme offset (*)  |      Report-to SSP offset (*)   |
 +----------------+----------------+----------------+----------------+
 |    Custodian scheme offset (*)  |      Custodian SSP offset (*)   |
 +----------------+----------------+----------------+----------------+
 |                    Creation Timestamp time (*)                    |
 +---------------------------------+---------------------------------+
 |             Creation Timestamp sequence number (*)                |
 +---------------------------------+---------------------------------+
 |                           Lifetime (*)                            |
 +----------------+----------------+----------------+----------------+
 |                        Dictionary length (*)                      |
 +----------------+----------------+----------------+----------------+
 |                  Dictionary byte array (variable)                 |
 +----------------+----------------+---------------------------------+
 |                      [Fragment offset (*)]                        |
 +----------------+----------------+---------------------------------+
 |              [Total application data unit length (*)]             |
 +----------------+----------------+---------------------------------+

 Bundle Payload Block
 +----------------+----------------+----------------+----------------+
 |  Block type    | Proc. Flags (*)|        Block length(*)          |
 +----------------+----------------+----------------+----------------+
 /                     Bundle Payload (variable)                     /
 +-------------------------------------------------------------------+

 * */

#include "ns3/enum.h"
#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "common_header.h"

namespace ns3 {
    namespace ns3dtnbit {
        
        enum BundleType {BundlePacket, AntiPacket, HelloPacket, TransmissionAck};

        // use this to check bp_headers //TODO
        struct BPHeaderID {
            Ipv4Address source_ip_;
            dtn_seqno_t source_sequence_no_;
        }

        class BPHeader : public Header {
            public :
                BPHeader ();
                // inherited items
                //    // redefine GetTypeId()
                TypeId GetTypeId() static;

                TypeId GetInstanceTypeId() const;

                uint32_t GetSerializedSize () const;
                void Serialize (Buffer::Iterator start) const;
                uint32_t Deserialize (Buffer::Iterator start);
                void Print (std::ostream &os) const;

                // own stuff *********
                bool operator==(BPHeader const& rh) const;

                // assuming that different BundleType has different payload size, and payload size themselves could be different TODO
                enum BundleType get_bundle_type() {return bundle_type_;}
                void set_bundle_type(enum BundleType arg) {bundle_type_ = arg;}
                bool is_valid() {return is_valid_;}
                void set_valid(bool arg) {is_valid_ = arg;}
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
                void set_payload_size_(uint32_t arg) {payload_size_ = arg;}
                Time get_src_time_stamp() {return Time(MilliSeconds(src_time_stamp_));}
                void set_src_time_stamp(Time arg) {src_time_stamp_ = arg.GetMilliSeconds();}
                Time get_hop_time_stamp() {return Time(MilliSeconds(hop_time_stamp_));}
                void set_hop_time_stamp(Time arg) {hop_time_stamp_ = arg.GetMilliSeconds();}

            private :

                bool is_valid_;
                enum BundleType bundle_type_;

                uint32_t hop_count_;
                uint32_t spray_;
                uint32_t retransmission_count_;
                Ipv4Address destination_ip_;
                Ipv4Address source_ip_;
                dtn_seqno_t source_seqno_;
                uint32_t payload_size_;
                dtn_time_t src_time_stamp_;
                dtn_time_t hop_time_stamp_;
        }; 

        std::ostream& operator<< (std::ostream& os, BPHeader const& rh);

        // this AppHeader is going to be useless
        class AppHeader : public Header {
            public:
                AppHeader ();
                // inherited items
                //    // redefine GetTypeId()
                TypeId GetTypeId() static;

                TypeId GetInstanceTypeId() const;
                uint32_t GetSerializedSize () const;
                void Serialize (Buffer::Iterator start) const;
                uint32_t Deserialize (Buffer::Iterator start);
                void Print (std::ostream &os) const;

                // own stuff *******
                bool operator==(APHeader const& rh) const;
                enum AppType get_app_type() {return ap_type_;}
                void set_app_type(enum AppType arg) {ap_type_ = arg;}
            private:
                enum AppType app_type_;

        };

        std::ostream& operator<< (std::ostream& os, APHeader const& rh);

    } /* ns3dtnbit */ 

} /* ns3 */ 
#endif
