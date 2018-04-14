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
            public:
            CGRRouting(DtnApp& dp);
            ~CGRRouting() override {}
            // s is source index, d is dest index, return next hop
            int DoRoute(int s, int d) override;

            void GetInfo(node_id_t destination_id, 
                node_id_t from_id, std::vector<node_id_t> vec_of_current_neighbor, 
                node_id_t own_id, dtn_time_t expired_time, 
                uint32_t bundle_size, map<node_id_t, vector<node_id_t>> id2cur_exclude_vec_of_id, 
                dtn_time_t local_time, dtn_seqno_t that_seqno, vector<node_id_t> exclude_node_of_thispkt) override;
            protected:
            class RouteResultCandidate {
                public:
                static RouteResultCandidate NewExhaustedAs(node_id_t dest, dtn_time_t expiretime){
                    auto c = RouteResultCandidate();
                    c.dest_ = dest;
                    c.is_exhausted_ = true;
                    c.cannot_arrive_until_ = expiretime;
                    return c;
                }
                RouteResultCandidate(){ 
                    is_exhausted_ = false;
                }
                ~RouteResultCandidate(){}
                RouteResultCandidate(RouteResultCandidate const & rhs) {
                    result_cash_end_ = rhs.result_cash_end_;
                    result_cash_begin_ = rhs.result_cash_begin_;
                    nexthop_ = rhs.nexthop_;
                    dest_ = rhs.dest_;
                    forfeit_time_ = rhs.forfeit_time_;
                    cannot_arrive_until_ = rhs.cannot_arrive_until_;
                    is_exhausted_ = rhs.is_exhausted_;
                    vec_ = rhs.vec_;
                    seqpkt_=rhs.seqpkt_;
                    debug_id_ = rhs.debug_id_;
                }
                RouteResultCandidate& operator=(RouteResultCandidate const & rhs) {
                    result_cash_end_ = rhs.result_cash_end_;
                    result_cash_begin_ = rhs.result_cash_begin_;
                    nexthop_ = rhs.nexthop_;
                    dest_ = rhs.dest_;
                    forfeit_time_ = rhs.forfeit_time_;
                    cannot_arrive_until_ = rhs.cannot_arrive_until_;
                    is_exhausted_ = rhs.is_exhausted_;
                    vec_ = rhs.vec_;
                    seqpkt_=rhs.seqpkt_;
                    debug_id_ = rhs.debug_id_;
                    return *this;

                }
                void PushXmit(CgrXmit cgrxmit) {
                    vec_.push_back(cgrxmit);
                }
                void PopXmit() {
                    vec_.pop_back();
                }
                string ToString() {
                    auto bb = is_exhausted_?"True":"False";
                    stringstream ss;
                    if (!is_exhausted_) {
                        ss << "RRC:" <<
                        ";is_exhausted_="<<bb<<
                        ";result_cash_begin_=" << result_cash_begin_ <<
                        ";result_cash_end_="<<result_cash_end_<<
                        ";nexthop_="<<nexthop_<<
                        ";dest_="<<dest_<<
                        ";forfeit_time_="<<forfeit_time_<<
                        ";debug_id_="<<debug_id_<<
                        ";pktseqnowhendecision="<<seqpkt_<<
                        ";iscomplete="<< IsComplete();
                    } else {
                        ss << "RRC:"
                        ";is_exhausted_="<<bb<<
                        ";result_cash_begin_=" << result_cash_begin_ <<
                        ";result_cash_end_="<<result_cash_end_<<
                        ";dest_="<<dest_<<
                        ";cannot_arrive_until_="<<cannot_arrive_until_<<
                        ";debug_id_="<<debug_id_<<
                        ";pktseqnowhendecision="<<seqpkt_<<
                        ";iscomplete="<< IsComplete();
                    }
                    return ss.str();
                }
                string VecString() const {
                    if (!is_exhausted_) {
                        stringstream ss;
                        for (auto const & xmit : vec_) {
                            ss << " \n " << xmit.ToString();
                        }
                        return ss.str();
                    } else {
                        assert(vec_.empty());
                        return "";
                    }
                }
                string VecPathStr() const {
                    if (!is_exhausted_) {
                        stringstream ss;
                        for (auto const & xmit : vec_) {
                            ss << " -- " << xmit.node_id_of_from_;
                        }
                        return ss.str();
                    } else {
                        assert(vec_.empty());
                        return "";
                    }
                }
                // @brief get path from next-to-dest to nexthop
                vector<node_id_t> GetPath() const {
                    // note: assume no loop-path, if have, would bug!
                    bool dohaveloop = false;
                    assert(!dohaveloop);
                    vector<node_id_t> path;
                    for (auto const & xmit : vec_) {
                        path.emplace_back(xmit.node_id_of_to_);
                        path.emplace_back(xmit.node_id_of_from_);
                    }
                    assert(path.size() >= 2);
                    // pick cherry out (cherry is dest and ownnode)
                    path.erase(path.begin());
                    path.erase(path.end() - 1);
                    return path;
                }
                size_t GetXmitSize() const {
                    return vec_.size();
                }
                dtn_time_t GetArriveDestTimeIfGood() const {
                    assert(!vec_.empty());
                    // first xmit is from next-to-dest-node to dest-node
                    return vec_[0].contact_start_time_;
                }
                dtn_time_t result_cash_end_;    // cash no longer work at this time
                dtn_time_t result_cash_begin_;  // cash works at this time
                node_id_t nexthop_; // if is exhausted, nexthop_ should be -1
                node_id_t dest_;
                dtn_time_t forfeit_time_;
                dtn_seqno_t seqpkt_;    // debug
                dtn_time_t cannot_arrive_until_;    // if later pkt expired time is before this, it's exhausted.
                bool is_exhausted_;
                bool IsComplete() const {return debug_id_ >= 0;}
                // @brief invoke when pushed into table_
                void Complete(int id){debug_id_ = id;}
                private:
                int debug_id_;
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
                void PushOneXmit(CgrXmit cgrxmit_ref);
                void InitCC(dtn_seqno_t seq);
                void PushFinalXmit(CgrXmit cgrxmit_ref,dtn_time_t local_forfeit_time, dtn_time_t local_best_delivery_time);
                void PopOneXmit();
                string CCPathStr() const{return cur_candidate_.VecPathStr();}
                private:
                std::string LogPrefix() {return out_app_.LogPrefix();}

                CGRRouting const & out_app_;
                RouteResultCandidate cur_candidate_;
                vector<RouteResultCandidate> table_;
                vector<RouteResultCandidate> this_route_;
                int seqno_for_rrc_ = 0;
                dtn_time_t last_remove_table_time_;
            };
            CashedRouteTable CRT_;

            // @brief first step of CGR-three-steps
            virtual void Init();

            // @brief second step of CGR-three-steps
            //Please read this link, if you want to know about this. https://tools.ietf.org/html/draft-burleigh-dtnrg-cgr-00
            //---- cur_d 
            //---- cur_deadline
            //---- proximate_vec_
            //---- excluded_vec_
            //---- forfeit_time_
            //---- best_delivery_time_
            virtual void ContactReviewProcedure(node_id_t cur_d, dtn_time_t cur_deadline, dtn_time_t best_deli);
            // @brief third step of CGR-three-steps
            virtual int ForwardDecision(vector<RouteResultCandidate> & rrc_vec);
            // implement this! TODO
            // check https://tools.ietf.org/html/draft-burleigh-dtnrg-cgr-00 -------- 2.5.3
            // @brief NetworkConfigureMaters Decision
            // @return index of rrc_vec
            virtual int NCMDecision(vector<RouteResultCandidate> const & rrc_vec);


            // ----------------- start of debug
            bool cgr_debug_flag_0 = false;
            bool cgr_debug_flag_1 = false;
            int debug_recurrsive_deep_;
            int debug_crp_enter_count_;
            map<node_id_t, int> debug_node_access_count_map_;

            stack<int> debug_recurrsive_path_stack_;
            bool debug_cgr_this_exhausted_search_not_found_;
            dtn_seqno_t debug_cgr_that_seqno_;
            // @brief inspect recurrsive deep and stack 
            void debugFunc01(node_id_t cur_d,string str);
            // @brief inspect Cgr_xmit
            void debugFunc02(node_id_t cur_d, vector<CgrXmit> const & cgr_xmit_vec_ref) const;
            // @brief print some
            void debugFunc03(string str) const;
            // @brief print some
            void debugFunc04(CgrXmit const & m, dtn_time_t cur_deadline,dtn_time_t local_forfeit_time, double next_deadline, string str) const;
            void DebugPrintXmit(vector<CgrXmit> const & cgr_xmit_vec_ref, int cur_d) const;

            // -------------- end of debug

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
            map<node_id_t, vector<node_id_t>> id_of_d2cur_excluded_vec_of_d_;

            dtn_time_t forfeit_time_;
            dtn_time_t best_delivery_time_;
        };
    }
} /* ns3  */ 
#endif /* ifndef CGRROUTING_H */