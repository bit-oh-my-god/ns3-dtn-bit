/*
 * 1. generate your trace file
 * 2. define your example, mainly 3 parts of it
 *      a. constructor of YourExample
 *      b. CreateRouting()
 *      c. YouRouting()
 *          note that you do can use value of s,d as index of predecessor until listS is set to be the vertex container of Graph
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
                // s is source index, d is dest index, return next hop
                int DoRoute(int s, int d) override {
                    using namespace boost;
                    const DtnApp::Adob& ref_adob = RoutingMethodInterface::get_adob();
                    auto g = ref_adob.get_graph_for_now();

                    using Graph_T = decltype(g);
                    using Vertex_D = boost::graph_traits<Graph_T>::vertex_descriptor;
                    using Edge_D = boost::graph_traits<Graph_T>::edge_descriptor;

                    Vertex_D s_des, d_des;
                    {
                        // get vertex_descriptor
                        std::stringstream ss1;
                        ss1 << "node-" << s;
                        string s_str = ss1.str();
                        std::stringstream ss0;
                        ss0 << "node-" << d;
                        string d_str = ss0.str();
                        boost::graph_traits<Graph_T>::vertex_iterator vi, vi_end;
                        for(boost::tie(vi, vi_end) = boost::vertices(g); vi != vi_end; ++vi){
                            if (g[*vi].name_ == s_str) {
                                s_des = *vi;
                            } else if (g[*vi].name_  == d_str) {
                                d_des = *vi;
                            }
                        }
                    }

                    std::vector<Vertex_D> predecessor(boost::num_vertices(g));
                    std::vector<int> distances(boost::num_vertices(g));

                    dijkstra_shortest_paths(g, s_des,
                            weight_map(get(&my_edge_property::distance_, g)).
                            distance_map(make_iterator_property_map(distances.begin(), get(vertex_index, g))).
                            predecessor_map(make_iterator_property_map(predecessor.begin(), get(vertex_index, g)))
                            );
                    Vertex_D cur = d_des;
                    int count = num_vertices(g);
                    while (predecessor[cur] != s_des && count-- > 0) {
                        cur = predecessor[cur];
                    }
                    if (count <= 0) {
                        std::cout << "fuckhere, you can't find the next hop, print predecessor and abort()" << std::endl;
                        for (auto v : predecessor) {
                            std::cout << " v=" << v << ";";
                        }
                        std::cout << endl;
                    }
                    std::cout << "fuckhere , s_dex =" << s_des << "; d_des=" << d_des <<  ";predecessor[cur]=" << predecessor[cur] << ";cur=" << cur << std::endl;
                    int result = (int)cur;
                    return result;
                }
        };

        class YourExample : public DtnExampleInterface {
            public :
                YourExample() : DtnExampleInterface() {
                    node_number_ = 3;
                    // simulation time should be less than trace_file_ time !Important
                    simulation_duration_ = 1601;
                    print_log_boolean_ = true;
                    ex_rm_ = DtnApp::RoutingMethod::Other;
                    //ex_rm_ = DtnApp::RoutingMethod::SprayAndWait;
                }

                void ReportEx(std::ostream& os) override {
                    os << "Here In DtnExampleInterface::ReportEx" << endl;
                }

                std::unique_ptr<RoutingMethodInterface> CreateRouting(DtnApp& dtn) override {
                    auto p = new YouRouting(dtn);
                    return std::unique_ptr<RoutingMethodInterface>(p);
                }

                void ScheduleTask() override {
                    int sch_size = 341;
                    auto handy_func = [sch_size, this](dtn_time_t sch_time, int i, int dstnode){
                        std::cout << "bundle send schedule: time=" << sch_time << ";node-" << i << "send " << sch_size << " size-pkt to node-" << dstnode << std::endl;
                        app_[i]->ScheduleTx(Seconds(sch_time), dstnode, sch_size);
                    };
                    handy_func(3.0, 0, 1);
                    handy_func(7.0, 0, 2);
                    handy_func(22.0, 0, 1);
                    handy_func(123.0, 0, 2);
                    handy_func(34.0, 2, 1);
                    handy_func(55.0, 2, 0);
                    handy_func(421.0, 2, 0);
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
