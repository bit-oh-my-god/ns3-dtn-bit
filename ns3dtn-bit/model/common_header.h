#ifndef COMMON_HEADER_H
#define COMMON_HEADER_H 

// this file should be the neck of the header dependent graph
// every header file would need outside dependent should include this one
// 
// 

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <tuple>

#define NS3DTNBIT_HYPOTHETIC_NEIGHBOR_BAQ_NUMBER_MAX 1000
#define NS3DTNBIT_MAC_MTU 2296  // TODO find what this means

namespace ns3 {
    
    namespace ns3dtnbit {

        using std::vector;
        
        using dtn_time_t = uint32_t;
        // dtn_seqnof_t is used with sign, eg. -(seqno2003) means giving out while (seqno2003) means receiving
        using dtn_seqnof_t = int32_t;
        using dtn_seqno_t = uint32_t;
    } /* ns3dtnbit */ 
} /* ns3  */ 
#endif /* ifndef COMMON_HEADER_H */
