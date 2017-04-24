#ifndef DTN_PRE_H
#define DTN_PRE_H value


#include "dtn_package.h"
#include "common_header.h"


namespace ns3 {

    namespace ns3dtnbit {

#ifdef DEBUG
        std::string GetCallStack(int);
        std::string FilePrint(std::string);
        std::string GetLogStr(std::string);
#endif /* ifndef DEBUG */

        struct cgr_xmit {
            dtn_time_t contact_start_time_;
            dtn_time_t contact_end_time_;
            int node_id_of_from_;       // transmission node
            int node_id_of_to_;         // receiving node
            double data_transmission_rate_;
        };

        struct my_edge_property {
            my_edge_property () { }
            my_edge_property(int v, int c) {
                distance_ = v;
                message_color_ = c;
            }
            // physic distance
            int distance_;
            // see that paper
            int message_color_;
        };

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
