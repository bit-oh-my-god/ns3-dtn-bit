#ifndef CGRROUTING_H
#define CGRROUTING_H value

#include "routingInterface.h"

namespace ns3 {
    // CGR
    namespace ns3dtnbit {
        // read this :
        // https://tools.ietf.org/html/draft-burleigh-dtnrg-cgr-00
        // note this algo is not CGR-EB, it's standard CGR (which can be found in here: Analysis of the contact graph routing algorithm: Bounding interplanetary paths)
        class CGRRouting : public RoutingMethodInterface {
            public :
            CGRRouting(DtnApp& dp);
            // s is source index, d is dest index, return next hop
            int DoRoute(int s, int d) override;

            void GetInfo(node_id_t destination_id, node_id_t from_id, std::vector<node_id_t> vec_of_current_neighbor, node_id_t own_id, dtn_time_t expired_time, int bundle_size, int networkconfigurationflag, map<int, vector<int>> id2cur_exclude_vec_of_id, dtn_time_t local_time, dtn_seqno_t that_seqno) override;

            private :
            
            class RouteResultCandidate {
                public:
                static RouteResultCandidate NewExhaustedAs(node_id_t dest, dtn_time_t expiretime){
                    auto c = RouteResultCandidate();
                    c.dest_ = dest;
                    c.is_exhausted_ = true;
                    c.cannot_arrive_until_ = expiretime + NS3DTNBIT_CGR_OPTIMAL_OPTION_REUSE_INTERVAL2;
                    return c;
                }
                RouteResultCandidate(){
                    result_cash_end_ = -0.1;
                    result_cash_begin_=-0.1;
                    nexthop_=-1;
                    dest_=-1;
                    forfeit_time_=-0.1;
                    cannot_arrive_until_=-0.1;
                    is_exhausted_=false;
                }
                void PushXmit(CgrXmit cgrxmit) {
                    vec_.push_back(cgrxmit);
                }
                void PopXmit() {
                    vec_.pop_back();
                }
                string ToString() {
                    char str[200];
                    sprintf(str, "result_cash_begin_=%f,result_cash_end_=%f,nexthop_=%d, dest_=%d,forfeit_time_=%f,cannot_arrive_until_=%f,is_exhausted_=%s", 
                    result_cash_begin_, result_cash_end_, nexthop_, dest_, 
                    forfeit_time_, cannot_arrive_until_, is_exhausted_);
                    return string(str);
                }
                dtn_time_t result_cash_end_;    // cash no longer work at this time
                dtn_time_t result_cash_begin_;  // cash works at this time
                node_id_t nexthop_; // if is exhausted, nexthop_ should be -1
                node_id_t dest_;
                dtn_time_t forfeit_time_;
                dtn_time_t cannot_arrive_until_;    // if later pkt expired time is before this, it's exhausted.
                bool is_exhausted_;
                private:
                vector<CgrXmit> vec_;
            };
            // @brief record for later use 
            class CashedRouteTable {
                public:
                CashedRouteTable(CGRRouting& out_app) : out_app_(out_app){
                    cur_candidate_ = RouteResultCandidate();
                    table_ = vector<RouteResultCandidate>();
                    this_route_ = vector<RouteResultCandidate>();
                }
                vector<RouteResultCandidate> RecordCRPResult(string str,node_id_t dest, dtn_time_t expiretime, bool flag);
                vector<RouteResultCandidate> ReuseOldResult(node_id_t dest, dtn_time_t expiretime);
                void PushOneXmit(CgrXmit& cgrxmit_ref);
                void InitCC() {
                    cur_candidate_ = RouteResultCandidate();    // empty one
                }
                void PushFinalXmit(CgrXmit& cgrxmit_ref,dtn_time_t local_forfeit_time, dtn_time_t local_best_delivery_time);
                void PopOneXmit();
                private:
                std::string LogPrefix() {return out_app_.LogPrefix();}
#ifdef CGR_DEBUG_2
            bool cgr_debug_flag_2 = true;
#endif
#ifndef CGR_DEBUG_2
            bool cgr_debug_flag_2 = false;
#endif
                CGRRouting const & out_app_;
                RouteResultCandidate cur_candidate_;
                vector<RouteResultCandidate> table_;
                vector<RouteResultCandidate> this_route_;
            };
            CashedRouteTable CRT_;

            // @brief first step of CGR-three-steps
            void Init();
            // @brief second step of CGR-three-steps
            //Please read this link, if you want to know about this. https://tools.ietf.org/html/draft-burleigh-dtnrg-cgr-00
            //---- cur_d 
            //---- cur_deadline
            //---- proximate_vec_
            //---- excluded_vec_
            //---- forfeit_time_
            //---- best_delivery_time_
            void ContactReviewProcedure(node_id_t cur_d, dtn_time_t cur_deadline, dtn_time_t best_deli);
            // @brief third step of CGR-three-steps
            //int ForwardDecision(bool if_cannot_find_route=false,bool if_route_candidate_cashed=false);
            int ForwardDecision(vector<RouteResultCandidate> rrc_vec);
            // implement this! TODO
            // check https://tools.ietf.org/html/draft-burleigh-dtnrg-cgr-00 -------- 2.5.3
            // @brief return index of proximate_vec_
            int NCMDecision();

            // ----------------- start of debug
            bool cgr_debug_flag_0 = false;
            bool cgr_debug_flag_1 = false;
            bool cgr_debug_flag_2 = false;
            int debug_recurrsive_deep_;
            int debug_crp_enter_count_;
            map<int, int> debug_node_access_count_map_;
            stack<int> debug_recurrsive_path_stack_;
            bool debug_cgr_this_exhausted_search_not_found_;
            dtn_seqno_t debug_cgr_that_seqno_;
            // @brief inspect recurrsive deep and stack 
            void debugFunc01(node_id_t cur_d,string str);
            // @brief inspect Cgr_xmit
            void debugFunc02(node_id_t cur_d, vector<CgrXmit>& cgr_xmit_vec_ref) const;
            // @brief print some
            void debugFunc03(string str) const;
            // @brief print some
            void debugFunc04(CgrXmit& m, dtn_time_t cur_deadline,dtn_time_t local_forfeit_time, double next_deadline, string str) const;
            void DebugPrintXmit(vector<CgrXmit>& cgr_xmit_vec_ref, int cur_d) const;
            // -------------- end of debug

            private :
            node_id_t destination_id_; 
            node_id_t own_id_;
            dtn_time_t local_time_;
            int cgr_find_proximate_count_;
            int ecc_;
            node_id_t node_id_transmit_from_;
            std::vector<node_id_t> id_of_current_neighbor_;
            dtn_time_t expired_time_;
            vector<node_id_t> proximate_vec_;
            vector<node_id_t> excluded_vec_;
            map<int, vector<node_id_t>> id_of_d2cur_excluded_vec_of_d_;
            dtn_time_t forfeit_time_;
            dtn_time_t best_delivery_time_;
        };
    }
} /* ns3  */ 
#endif /* ifndef CGRROUTING_H */