#ifndef COMMON_HEADER_H
#define COMMON_HEADER_H 

// this file should be the neck of the header dependent graph
// every header file should need outside dependent should include this one

#include <iostream>
#include <vector>

namespace ns3 {
    
    namespace ns3dtnbit {

        using std::vector;
        
        using dtn_time_t = double;
        using dtn_id_t = uint32_t;
    } /* ns3dtnbit */ 
} /* ns3  */ 
#endif /* ifndef COMMON_HEADER_H */
