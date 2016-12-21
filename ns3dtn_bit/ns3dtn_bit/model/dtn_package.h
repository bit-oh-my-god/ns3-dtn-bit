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

#include "common_header.h"
#include "ns3/header.h"

namespace ns3 {
    namespace ns3dtn_bit {

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

                // own stuff
            private :
        }; 

        class APHeader : public Header {
            public:
                APHeader ();
                // inherited items
                //    // redefine GetTypeId()
                TypeId GetTypeId() static;

                TypeId GetInstanceTypeId() const;
                uint32_t GetSerializedSize () const;
                void Serialize (Buffer::Iterator start) const;
                uint32_t Deserialize (Buffer::Iterator start);
                void Print (std::ostream &os) const;

                // own stuff
            private:


        };
    } /* ns3dtn_bit */ 

} /* ns3 */ 
#endif
