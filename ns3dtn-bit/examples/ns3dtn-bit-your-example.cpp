/*
 * 1. generate your trace file
 * 2. define your example, mainly how node are connected,
 *  you can override it, but must implement ReportEx(),
 * 3. run your example
 */
#include "ns3/ns3dtn-bit-helper.h"
#include "ns3/ns3dtn-bit-example-interface.h"
#include "ns3/ns3dtn-bit.h"

using namespace ns3;
namespace ns3 {
    namespace ns3dtnbit {

        class YouRouting : public RoutingMethodInterface {
            public :
                YouRouting(DtnApp& dp) : RoutingMethodInterface(dp) {}
                int DoRoute(int s, int d) override {
                    using namespace boost;
                    const DtnApp::Adob& ref_adob = RoutingMethodInterface::get_adob();
                    auto g = ref_adob.get_graph_for_now();

                    using Graph_T = decltype(g);
                    using Vertex_D = boost::graph_traits<Graph_T>::vertex_descriptor;
                    using Edge_D = boost::graph_traits<Graph_T>::edge_descriptor;

                    //boost::property_map<Graph_T, edge_mycost_t>::type mycostmap = boost::get(edge_mycost, g);
                    std::vector<Vertex_D> predecessor(boost::num_vertices(g));
                    std::vector<int> distances(boost::num_vertices(g));
                    auto s_des = predecessor[s];
                    auto d_des = predecessor[d];

                    dijkstra_shortest_paths(g, s_des,
                            weight_map(get(&my_edge_property::distance, g)).
                            distance_map(make_iterator_property_map(distances.begin(), get(vertex_index, g))).
                            predecessor_map(make_iterator_property_map(predecessor.begin(), get(vertex_index, g)))
                            );
                    Vertex_D cur = d_des;
                    // dangerous cast but works
                    while (predecessor[cur] != s_des) {
                        cur = predecessor[cur];
                    }
                    int result = (int)cur;
                    return result;
                }
        };

        class YourExample : public DtnExampleInterface {
            public :
                YourExample() : DtnExampleInterface() {
                    node_number_ = 5;
                    // simulation time should be less than trace_file_ time !Important
                    simulation_duration_ = 802;
                    print_log_boolean_ = true;
                    ex_rm_ = DtnApp::RoutingMethod::SprayAndWait;
                }
                void ReportEx(std::ostream& os) override {
                    os << "Here In DtnExampleInterface::ReportEx" << endl;
                }
                std::unique_ptr<RoutingMethodInterface> CreateRouting(DtnApp& dtn) override {
                    auto p = new YouRouting(dtn);
                    return std::unique_ptr<RoutingMethodInterface>(p);
                }
        };

    } /* ns3dtnbit */ 

} /* ns3  */ 

int main(int argc, char *argv[]) {
    //!important LOG control
    //LogComponentEnable ("DtnRunningLog",LOG_LEVEL_WARN);
    //LogComponentEnable ("DtnRunningLog",LOG_LEVEL_DEBUG);
    LogComponentEnable ("DtnRunningLog",LOG_LEVEL_INFO);
    //LogComponentEnable ("DtnRunningLog",LOG_LEVEL_LOGIC);

    assert(std::is_move_constructible<ns3dtnbit::YourExample>::value);
    assert(std::is_move_assignable<ns3dtnbit::YourExample>::value);
    std::unique_ptr<ns3dtnbit::DtnExampleInterface> exp(new ns3dtnbit::YourExample());
    auto runner = ns3dtnbit::DtnExampleRunner();
    runner.RunnerLoad(exp).RunIt(argc, argv);
    return 0;
}
