#ifndef DTN_PRE_H
#define DTN_PRE_H value


#include "dtn_package.h"
#include "common_header.h"

// this file declare some class that was instantialized before DTN sim runs

namespace ns3 {

    namespace ns3dtnbit {

        /*
         * by default, get the function name called the logfunc which called this
         */
        std::string GetCallStack(int);
        std::string FilePrint(std::string);
        std::string GetLogStr(std::string);

        // see RFC - CGR
        struct CgrXmit {
            // D' list of xmits means that all xmit in list have node_id_of_to_ to be D
            dtn_time_t contact_start_time_;
            dtn_time_t contact_end_time_;
            int node_id_of_from_;       // transmission node
            int node_id_of_to_;         // receiving node
            double data_transmission_rate_; // set it to 80 000
        };

        bool operator<(CgrXmit const & lhs, CgrXmit const & rhs);

        // used for boost graph
        struct my_edge_property {
            my_edge_property () { }
            my_edge_property(int v, int c) {
                distance_ = v;
                message_color_ = c;
            }
            // physic distance
            int distance_;
            // see teg paper
            int message_color_;
        };

        // used for boost graph
        struct my_vertex_property {
            my_vertex_property() {}
            my_vertex_property(string s) {
                name_ = s;
            }
            string name_;
        };

    } /* ns3dtnbit */ 
} /* ns3 */ 

#endif /* ifndef DTN_PRE_H */
