#ifndef COMMON_HEADER_H
#define COMMON_HEADER_H 

// every header file would need outside dependent should include this one

#include <cassert>
#include <iostream>
#include <regex>
#include <fstream>
#include <type_traits>
#include <execinfo.h>
#include <cmath>
#include <limits>
#include <vector>
#include <cstdio>
#include <memory>
#include <type_traits>
#include <sstream>
#include <memory>
#include <algorithm>
#include <string>
#include <map>
#include <unordered_map>
#include <tuple>
#include <utility>
#include <boost/version.hpp>
//#include <boost/config.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/graph/bellman_ford_shortest_paths.hpp>
#include <boost/graph/graphviz.hpp>


//#define DEBUG
#define NS3DTNBIT_PORT_NUMBER 1234
#define NS3DTNBIT_HELLO_PORT_NUMBER 80
#define NS3DTNBIT_MAC_MTU 2296  
#define NS3DTNBIT_MAX_TRANSMISSION_TIMES 4
#define NS3DTNBIT_HYPOTHETIC_TRANS_SIZE_FRAGMENT_MAX 1472
#define NS3DTNBIT_HELLO_BUNDLE_SIZE_MAX 2280
#define NS3DTNBIT_HYPOTHETIC_NEIGHBOR_BAQ_NUMBER_MAX 400
#define NS3DTNBIT_BUFFER_CHECK_INTERVAL 0.3
#define NS3DTNBIT_HELLO_BUNDLE_INTERVAL_TIME 0.2
#define NS3DTNBIT_RETRANSMISSION_INTERVAL 15.0
#define NS3DTNBIT_HYPOTHETIC_BUNDLE_EXPIRED_TIME 650.0
#define NS3DTNBIT_ANTIPACKET_EXPIRED_TIME 650.0
#define NS3DTNBIT_HYPOTHETIC_INFINITE_DELAY (std::numeric_limits<int>::max()/2)
#define NS3DTNBIT_HYPOTHETIC_CACHE_FACTOR 978

namespace ns3 {

    namespace ns3dtnbit {
        using std::vector;
        using std::string;
        using std::endl;
        using std::cout;
        using std::pair;
        using std::map;
        using std::unordered_map;
        using std::min;
        using std::set;
        using std::tuple;
        using std::to_string;
        using std::make_pair;
        using std::tie;
        using std::make_tuple;
        using std::ostream;
        using std::ofstream;
        using std::ifstream;
        using std::stringstream;

        using dtn_time_t = double;
        using dtn_seqnof_t = int32_t;
        using dtn_seqno_t = uint32_t;

    } /* ns3dtnbit */ 
} /* ns3  */ 

#endif /* ifndef COMMON_HEADER_H */
