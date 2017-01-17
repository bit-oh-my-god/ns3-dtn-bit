#include "dtn_package.h"

namespace ns3 {
    namespace ns3dtnbit {
        // BPHeader
        NS_OBJECT_ENSURE_REGISTERED(BPHeader);

        BPHeader::BPHeader() {

        }

        TypeId BPHeader::GetTypeId() {
            static TypeId tid = TypeId("ns3::ns3dtnbit::BPHeader")
                .SetParent<Header>()
                .AddConstructor<BPHeader>();
            return tid;

        }

        TypeId BPHeader::GetInstanceTypeId() {
            return GetTypeId();

        }

        void BPHeader::Serialize(Buffer::Iterator start) const {
            start.WriteU32(hop_count_);
            start.WriteU32(spray_);
            start.WriteU32(retransmission_count_);
            WriteTo(start, destination_ip_);
            WriteTo(start, source_ip_);
            start.WriteU32(source_seqno_);
            start.WriteU32(payload_size_);
            start.WriteU32(offset_size_);
            start.WriteU32(src_time_stamp_);
            start.WriteU32(hop_time_stamp_);
        }

        uint32_t BPHeader::Deserialize(Buffer::Iterator start) {
            Buffer::Iterator i = start;
            hop_count_ = i.ReadU32();
            spray_ = i.ReadU32();
            retransmission_count_ = i.ReadU32();
            ReadFrom(i, destination_ip_);
            ReadFrom(i, source_ip_);
            source_seqno_ = i.ReadU32();
            payload_size_ = i.ReadU32();
            offset_size_ = i.ReadU32();
            src_time_stamp_ = i.ReadU32();
            hop_time_stamp_ = i.ReadU32();

            uint32_t dist = i.GetDistanceFrom(start);
            NS_ASSERT(dist == GetSerializedSize());
            return dist;
        }

        uint32_t BPHeader::GetSerializedSize() const {
            return (sizeof(BPHeader::hop_count_) +
                    sizeof(BPHeader::spray_) +
                    sizeof(BPHeader::retransmission_count_) +
                    sizeof(BPHeader::destination_ip_) +
                    sizeof(BPHeader::source_ip_) +
                    sizeof(BPHeader::source_seqno_) +
                    sizeof(BPHeader::payload_size_) +
                    sizeof(BPHeader::offset_size_) +
                    sizeof(BPHeader::src_time_stamp_) +
                    sizeof(BPHeader::hop_time_stamp_));
        }

        void BPHeader::Print(std::ostream& os) const {
            os << ",destination ip" << destination_ip_
            << ",source ip" << source_ip_
            << ",source seqno" << source_seqno_
            << ",payload size" << payload_size_
            << ",offset size" << offset_size_
            << ",src time stamp" << src_time_stamp_
            << ",hop time stamp" << hop_time_stamp_
            << std::endl;
        }

        // AppHeader
        NS_OBJECT_ENSURE_REGISTERED(AppHeader);

        AppHeader::AppHeader() {

        }

        TypeId AppHeader::GetTypeId() {

        }

        TypeId AppHeader::GetInstanceTypeId() const {

        }
        
        void AppHeader::Serialize(Buffer::Iterator start) const {

        }

        uint32_t AppHeader::Deserialize(Buffer::Iterator start) const {

        }

        uint32_t AppHeader::GetSerializedSize() const {

        }

        void AppHeader::Print(std::ostream& os) const {

        }
    } /* ns3dtnbit */ 
    
} /* ns3  */ 
