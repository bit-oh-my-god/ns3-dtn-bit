#include "dtn_package.h"
#include "ns3/packet.h"

namespace ns3 {
    namespace ns3dtnbit {
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
            start.WriteU8()

        }

        uint32_t BPHeader::Deserialize(Buffer::Iterator start) {

        }

        uint32_t BPHeader::GetSerializedSize() const {

        }

        void BPHeader::Print(std::ostream& os) const {

        }

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
