#include "dtn_package.h"
#include "ns3/packet.h"

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
            start.WriteU32(source_unique_id_);
            start.WriteU32(bundle_size_);
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
            source_unique_id_ = i.ReadU32();
            bundle_size_ = i.ReadU32();
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
                    sizeof(BPHeader::source_unique_id_) +
                    sizeof(BPHeader::bundle_size_) +
                    sizeof(BPHeader::src_time_stamp_) +
                    sizeof(BPHeader::hop_time_stamp_));
        }

        void BPHeader::Print(std::ostream& os) const {
            os << "destination ip" << destination_ip_
            << "source ip" << source_ip_
            << "source unique id" << source_unique_id_
            << "bundle size" << bundle_size_
            << "src time stamp" << src_time_stamp_
            << "hop time stamp" << hop_time_stamp_
            << std::endl;
        }

        // APHeader
        NS_OBJECT_ENSURE_REGISTERED(APHeader);

        APHeader::APHeader() {

        }

        TypeId APHeader::GetTypeId() {

        }

        TypeId APHeader::GetInstanceTypeId() const {

        }
        
        void APHeader::Serialize(Buffer::Iterator start) const {

        }

        uint32_t APHeader::Deserialize(Buffer::Iterator start) const {

        }

        uint32_t APHeader::GetSerializedSize() const {

        }

        void APHeader::Print(std::ostream& os) const {

        }
    } /* ns3dtnbit */ 
    
} /* ns3  */ 
