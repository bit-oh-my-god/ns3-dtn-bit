#include "cgrrouting.h"

namespace ns3 {

    //
    // ======================================== CGR standard
    namespace ns3dtnbit {
        NS_LOG_COMPONENT_DEFINE ("DtnCGRRouting");
        #define LogPrefixMacro LogPrefix()<<"[DtnCGRRouting]line-"<<__LINE__<<"]"

        vector<CGRRouting::RouteResultCandidate> CGRRouting::CashedRouteTable::RecordCRPResult(string str,node_id_t dest, dtn_time_t expiretime, bool flag) {
            NS_LOG_INFO(LogPrefixMacro<<"into RecordCRPResult");

            if (str != "out of CRP") {
                NS_LOG_ERROR(LogPrefixMacro);
                std::abort();
            }
            if (flag) {
                // exhausted
                auto exhausted = RouteResultCandidate::NewExhaustedAs(dest,expiretime);
                exhausted.result_cash_begin_ = Simulator::Now().GetSeconds();
                exhausted.result_cash_end_ = min(
                    exhausted.result_cash_begin_ + NS3DTNBIT_CGR_OPTIMAL_OPTION_REUSE_INTERVAL2
                    ,exhausted.cannot_arrive_until_);
                exhausted.seqpkt_ = out_app_.debug_cgr_that_seqno_;
                exhausted.Complete(seqno_for_rrc_++);
                NS_LOG_INFO(LogPrefixMacro<<"one exhausted-RRC pushed in" << exhausted.ToString());
                table_.push_back(exhausted);
                return vector<CGRRouting::RouteResultCandidate>{exhausted};
            } else {
                for (auto& rrc : this_route_) {
                    rrc.dest_ = dest;
                    rrc.result_cash_begin_ = Simulator::Now().GetSeconds();
                    rrc.result_cash_end_ = min(
                        rrc.result_cash_begin_ + NS3DTNBIT_CGR_OPTIMAL_OPTION_REUSE_INTERVAL
                        ,rrc.forfeit_time_);
                    rrc.Complete(seqno_for_rrc_++);
                    NS_LOG_INFO(LogPrefixMacro<<"one nonexhausted-RRC pushed in" << rrc.ToString());

                    table_.push_back(rrc);
                }
                return this_route_;
            }
        }

        vector<CGRRouting::RouteResultCandidate> CGRRouting::CashedRouteTable::ReuseOldResult
        (node_id_t dest, dtn_time_t expiretime) {
            NS_LOG_INFO(LogPrefixMacro<< "into reuseoldresult");
            auto nowtime = Simulator::Now().GetSeconds();
            // remove old rrc
            table_.erase(
                std::remove_if(
                    table_.begin(), 
                    table_.end(), 
                    [=](CGRRouting::RouteResultCandidate rrc){
                        if (rrc.result_cash_end_ < nowtime) {
                            return true;
                        } else {
                            return false;
                        }
                    }
                ), 
                table_.end()
            );
            vector<CGRRouting::RouteResultCandidate> rerrc;
            NS_LOG_INFO(LogPrefixMacro<< "RRC table size=" << table_.size());
            for (auto const &rrc : table_) {
                assert(rrc.IsComplete());
                assert(rrc.result_cash_end_ > nowtime);
                if (rrc.dest_ == dest && rrc.is_exhausted_ && rrc.cannot_arrive_until_ > expiretime) {
                    assert(rerrc.empty());
                    rerrc.push_back(rrc);
                    break;

                }
                if (rrc.dest_ == dest && !rrc.is_exhausted_) {
                    rerrc.push_back(rrc);
                }
            }

            if (rerrc.size() == 1&& rerrc.back().is_exhausted_) {
                // reuse exhausted result
                NS_LOG_INFO(LogPrefixMacro<< "exhausted rrc.ToString()=" << rerrc.back().ToString());
                NS_LOG_DEBUG(LogPrefixMacro << "may make bad routing-1 exhausted");
            } else if (rerrc.size() == 0) {
                // no reuse
            } else if (rerrc.size() >= 0 && !rerrc.back().is_exhausted_) {
                // reuse cashed result
                NS_LOG_DEBUG(LogPrefixMacro << "may make bad routing-1 cashed");
            } else {
                NS_LOG_ERROR(LogPrefixMacro);
                std::abort();
            }

            return rerrc;
            NS_LOG_INFO(LogPrefixMacro<< "end of reuseoldresult");
        }

        void CGRRouting::CashedRouteTable::PushFinalXmit(CgrXmit cgrxmit,dtn_time_t local_forfeit_time, dtn_time_t local_best_delivery_time) {
            NS_LOG_INFO(" cur_rrc.seq=" << cur_candidate_.seqpkt_);
            cur_candidate_.PushXmit(cgrxmit);
            NS_LOG_INFO(LogPrefixMacro<<"after push final xmit, rrc.vec_ is " << cur_candidate_.VecString());
            this_route_.push_back(cur_candidate_);
            this_route_.back().forfeit_time_ = local_forfeit_time;
            this_route_.back().nexthop_ = cgrxmit.node_id_of_to_;
            this_route_.back().is_exhausted_ = false;
        }

        void CGRRouting::CashedRouteTable::PopOneXmit() {
            cur_candidate_.PopXmit();
        }
        void CGRRouting::CashedRouteTable::PushOneXmit(CgrXmit cgrxmit) {
            cur_candidate_.PushXmit(cgrxmit);
        }

        void CGRRouting::CashedRouteTable::InitCC(dtn_seqno_t seq) {
            cur_candidate_ = RouteResultCandidate();    // empty one
            NS_LOG_INFO(LogPrefixMacro<<"[initcc] seqno=" << seq);
            cur_candidate_.seqpkt_ = seq;
            this_route_ = vector<RouteResultCandidate>();
        }

    }

    namespace ns3dtnbit {
        
        static string vecstr(vector<node_id_t> const & vec) {
            stringstream ss;
            for (auto const & i : vec) {ss << " " << i << " ";}
            ss << endl;
            return ss.str();
        }

        int CGRRouting::DoRoute(int s, int d) {
            NS_LOG_INFO(LogPrefixMacro<< "DEBUG_CGR:" << " ====== Into DoRoute ===== "<< ";seqno=" << debug_cgr_that_seqno_);
            Init();
            assert(d == destination_id_);
            assert(s == own_id_);
            int result;
            bool reuse_flag = NS3DTNBIT_CGR_OPTIMAL_OPTION;
            // search exhausted_list and abandon
            if (reuse_flag) {
                NS_LOG_INFO(LogPrefixMacro);
                vector<RouteResultCandidate> result_vec = CRT_.ReuseOldResult(destination_id_,expired_time_);
                if (result_vec.empty()) {
                    NS_LOG_INFO(LogPrefixMacro<< "into CRP");
                    debugFunc01(destination_id_, "push");
                    ContactReviewProcedure(destination_id_, forfeit_time_, best_delivery_time_);
                    debugFunc01(-1,"pop");
                    result_vec = CRT_.RecordCRPResult("out of CRP", 
                    destination_id_, expired_time_, proximate_vec_.empty());
                    NS_LOG_INFO(LogPrefixMacro<< "DEBUG_CGR:" << "before ForwardDecision()" << "BundleTrace:entertimes=" << debug_crp_enter_count_ << "times");
                }
                result = ForwardDecision(result_vec);
            } else { 
                debugFunc01(destination_id_, "push");
                ContactReviewProcedure(destination_id_, forfeit_time_, best_delivery_time_);
                debugFunc01(-1,"pop");
                auto result_vec = CRT_.RecordCRPResult("out of CRP", 
                destination_id_, expired_time_, proximate_vec_.empty());
                result = ForwardDecision(result_vec);
            }
            NS_LOG_INFO(LogPrefixMacro<< "DEBUG_CGR:" << " ======= outof DoRoute ==== result=" << result << ";seqno=" << debug_cgr_that_seqno_ <<"\n");
            return result; 
        }

        void CGRRouting::GetInfo(node_id_t destination_id, 
                node_id_t from_id, std::vector<node_id_t> vec_of_current_neighbor, 
                node_id_t own_id, dtn_time_t expired_time, 
                uint32_t bundle_size, map<node_id_t, vector<node_id_t>> id2cur_exclude_vec_of_id, 
                dtn_time_t local_time, dtn_seqno_t that_seqno) {
            id_of_d2cur_excluded_vec_of_d_ =id2cur_exclude_vec_of_id; 
            local_time_ = local_time;
            destination_id_ = destination_id;
            NS_LOG_INFO(LogPrefixMacro<<"from_id="<<from_id);

            node_id_transmit_from_ = from_id;
            expired_time_ = expired_time;
            own_id_ = own_id;
            debug_cgr_that_seqno_ = that_seqno;
            ecc_ = bundle_size;
            id_of_current_neighbor_ = vec_of_current_neighbor;
        }

        void CGRRouting::Init() {
            debug_recurrsive_deep_ = 0;
            debug_crp_enter_count_ = 0;
            debug_cgr_this_exhausted_search_not_found_ = false;
            debug_node_access_count_map_ = map<int, int>();
            debug_recurrsive_path_stack_ = stack<int>();
            cgr_find_proximate_count_ = 0;
            forfeit_time_ = expired_time_;
            best_delivery_time_ = local_time_;
            excluded_vec_ = vector<int>();
            proximate_vec_ = vector<int>();
            // app tells which nodeid should not in route path

            auto cur_excluded = id_of_d2cur_excluded_vec_of_d_[destination_id_];
            for (auto nei : id_of_current_neighbor_) {
                auto found = find(cur_excluded.begin(), cur_excluded.end(), nei);
                if (found != cur_excluded.end()) {
                    excluded_vec_.push_back(nei);
                }
            }
            // exclude for transmit from id
            excluded_vec_.push_back(node_id_transmit_from_);
            NS_LOG_INFO(LogPrefixMacro<<"exclude_vec_="<<vecstr(excluded_vec_));
            CRT_.InitCC(debug_cgr_that_seqno_);
        }

        void CGRRouting::DebugPrintXmit(vector<CgrXmit> const & cgr_xmit_vec_ref, int cur_d) const{
            {
                cout << "DEBUG_CGR:" << " ====== for node-" << cur_d 
                    << " xmit is :" << endl;
                for (auto const & m : cgr_xmit_vec_ref) {

                    cout << "\n m ==> m.contact_start_time_ =" << m.contact_start_time_
                        << ";m.contact_end_time_=" << m.contact_end_time_
                        << ";m.node_id_of_from_=" << m.node_id_of_from_
                        << ";m.node_id_of_to_=" << m.node_id_of_to_
                        << ";m.data_transmission_rate_=" << m.data_transmission_rate_ << endl;
                }
                cout << "end of DEBUG_CGR" << endl;
            }

        }

        void CGRRouting::debugFunc01(node_id_t cur_d, string str) {
            if (str == "push") {
                if (cgr_debug_flag_1) {
                    debug_crp_enter_count_ += 1;

                    debug_recurrsive_path_stack_.push(cur_d);
                    debug_recurrsive_deep_ += 1;
                    auto found = debug_node_access_count_map_.find(cur_d);
                    if (found != debug_node_access_count_map_.end()) {
                        debug_node_access_count_map_[cur_d] += 1;
                    } else {
                        debug_node_access_count_map_[cur_d] = 1;
                    }
                    if (debug_recurrsive_deep_ > 7) {
                        if (debug_crp_enter_count_ > 5000) {
                            cout << "Error: this CGR is missing in deep recurrsive, we would print all access map and abort, enter this function " << debug_crp_enter_count_ << " times, check readme.md to know what happens" << endl;
                            while (!debug_recurrsive_path_stack_.empty()) {
                                cout << "node-" << debug_recurrsive_path_stack_.top() << " to" << endl;
                                debug_recurrsive_path_stack_.pop();
                            }
                            for (auto mv : debug_node_access_count_map_) {
                                int idn = get<0>(mv);
                                int count = get<1>(mv);
                                cout << "node-" << idn << " access " << count << "times" << endl;
                            }
                            abort();
                        }
                    }
                }
            } else if (str == "pop") { 
                if (cgr_debug_flag_1) {
                    debug_recurrsive_deep_ -= 1;
                    debug_recurrsive_path_stack_.pop();
                }
            } else {
                cout << __FILE__ << __LINE__ << endl;
                    std::abort();
            }
        }

        void CGRRouting::debugFunc02(node_id_t cur_d, vector<CgrXmit> const & cgr_xmit_vec_ref) const{

            if (cgr_debug_flag_0) {
                if (debug_cgr_that_seqno_ == NS3DTNBIT_CGR_DEBUG_SEQ_1 || debug_cgr_that_seqno_ == NS3DTNBIT_CGR_DEBUG_SEQ_2) {
                    cout << "CGR_DEBUG:temporary debug use, deleteme when you don't need me, xmitdebug-seqno-" << debug_cgr_that_seqno_ << __FILE__ << ":" <<  __LINE__ 
                        << ";own_id_ = " << own_id_ 
                        << ";local_time_ = " << local_time_ 
                        << endl;
                    DebugPrintXmit(cgr_xmit_vec_ref, cur_d);
                }
            }
        }

        void CGRRouting::debugFunc03(string str) const{
            if (cgr_debug_flag_1) {
                if (str == "tail of recurrsive") {
                cout << "DEBUG_CGR:" << "in tail of recursive, and, if we find a proximate one,"
                    << "we would break the search, because the CGR algorithm in RFC would force to find all possible pathes,"
                    << " which is too big for some senario, mainly the group moving senario " << endl;
                } else if (str == "body of recurrsive") {
                    cout << "DEBUG_CGR:" << "in body of recursive" << endl;
                } else {
                    cout << __FILE__ << __LINE__ << endl;
                    std::abort();
                }
            }
        }

         void CGRRouting::debugFunc04(CgrXmit const & m,dtn_time_t cur_deadline,dtn_time_t local_forfeit_time, double next_deadline, string str) const{

             if (str == "tail") {
                if (cgr_debug_flag_0) {
                    if (debug_cgr_that_seqno_ == NS3DTNBIT_CGR_DEBUG_SEQ_1 || debug_cgr_that_seqno_ == NS3DTNBIT_CGR_DEBUG_SEQ_2) {
                        cout << "\nCGR_DEBUG:\nthis is the xmit we finally choose" 
                            << ";forfeit_time_ = " << local_forfeit_time
                            << ";cur_deadline = " << cur_deadline << endl;
                        cout << "\n m ==> m.contact_start_time_ =" << m.contact_start_time_
                            << ";m.contact_end_time_=" << m.contact_end_time_
                            << ";m.node_id_of_from_=" << m.node_id_of_from_
                            << ";m.node_id_of_to_=" << m.node_id_of_to_
                            << ";m.data_transmission_rate_=" << m.data_transmission_rate_ << endl;
                    }
                }
            } else if (str == "body") {
                if (cgr_debug_flag_0) {
                    if (debug_cgr_that_seqno_ == NS3DTNBIT_CGR_DEBUG_SEQ_1 || debug_cgr_that_seqno_ == NS3DTNBIT_CGR_DEBUG_SEQ_2) {
                        cout << "\nCGR_DEBUG:\nthis is the xmit we enter" 
                            << ";cur_deadline = " << cur_deadline
                            << ";forfeit_time_ = " << local_forfeit_time
                            << ";next_deadline = " << next_deadline << endl;
                        cout << "\n m ==> m.contact_start_time_ =" << m.contact_start_time_
                            << ";m.contact_end_time_=" << m.contact_end_time_
                            << ";m.node_id_of_from_=" << m.node_id_of_from_
                            << ";m.node_id_of_to_=" << m.node_id_of_to_
                            << ";m.data_transmission_rate_=" << m.data_transmission_rate_ << endl;
                    }
                }
            } else {
                cout << __FILE__ << __LINE__ << endl;
                    std::abort();
            }
        }
        // Please read this link, if you want to know about this. https://tools.ietf.org/html/draft-burleigh-dtnrg-cgr-00
        //  ---- cur_d 
        //  ---- cur_deadline
        //  ---- proximate_vec_
        //  ---- excluded_vec_
        //  ---- forfeit_time_
        //  ---- best_delivery_time_
        void CGRRouting::ContactReviewProcedure(node_id_t cur_d, dtn_time_t cur_deadline, dtn_time_t best_deli) {
            NS_LOG_INFO(LogPrefixMacro<<"in new CRP,cur_d="<< cur_d <<";cur_deadline="<< cur_deadline);
            assert(debug_crp_enter_count_ < 300);
            if (cgr_find_proximate_count_ >= NS3DTNBIT_CGR_OPTIMAL_DECISION_AMOUNT) {

                return;
            }
            // 1.
            excluded_vec_.push_back(cur_d);
            const Adob& ref_adob = RoutingMethodInterface::get_adob();
            auto const & cgr_xmit_vec_ref = ref_adob.node_id2cgr_xmit_vec_map_[cur_d];
            debugFunc02(cur_d, cgr_xmit_vec_ref);
            // 2.
            for (auto const & m : cgr_xmit_vec_ref) {
                NS_LOG_INFO(LogPrefixMacro<<"<$$$>"
                << "cur_d=" << cur_d << ";cur_deadline=" << cur_deadline
                << "cur_path=<finaldest>"<< destination_id_ << CRT_.CCPathStr()
                << ";loop for xmit:" << m.ToString());
                bool localcheck = local_time_ < cur_deadline - NS3DTNBIT_CGR_CRP_REMAIN_INNODE;
                bool start_before_deadline = (m.contact_start_time_ + NS3DTNBIT_CGR_CRP_REMAIN_INNODE ) < cur_deadline ;
                bool end_after_now = local_time_ < (m.contact_end_time_ - NS3DTNBIT_CGR_CRP_REMAIN_INNODE);
                bool last_moment_check = localcheck && end_after_now && start_before_deadline;

                dtn_time_t local_forfeit_time = cur_deadline;
                dtn_time_t local_best_delivery_time = best_deli;
                if (!last_moment_check) {
                    // 2.A
                    NS_LOG_INFO(LogPrefixMacro<<"contine-1:" 
                    <<"localcheck="<<localcheck
                    <<"start_before_deadline="<<start_before_deadline
                    <<"end_after_now="<<end_after_now);

                    continue;
                } else {
                    // 2.B
                    node_id_t s = m.node_id_of_from_;
                    bool s_is_local_node_with_own_id = s == own_id_;
                    if (s_is_local_node_with_own_id) {
                        debugFunc03("tail of recurrsive");
                        // 2.B.1
                        // compute ECC for this bundle
                        int ecc = ecc_;
                        assert(m.contact_end_time_ - m.contact_start_time_ < 5000);
                        // not accurate, just a hypothetic value
                        int ecc_of_other_bundles = NS3DTNBIT_CGR_CRP_ECC_OF_OTHERS;

                        int residual_capacity = ((m.contact_end_time_ - m.contact_start_time_) * m.data_transmission_rate_) - ecc_of_other_bundles; 
                        assert(residual_capacity > 10000);
                        auto found_1 = find(proximate_vec_.begin(), proximate_vec_.end(), cur_d);
                        bool d_is_in_proximate = found_1 != proximate_vec_.end();
                        if (residual_capacity < ecc) {
                            NS_LOG_INFO(LogPrefixMacro<<"contine-2:");
                            continue;
                        } else if (d_is_in_proximate) {
                            NS_LOG_INFO(LogPrefixMacro<<"contine-3:");

                            continue;
                        } else {
                            if (m.contact_end_time_ < local_forfeit_time) {
                                local_forfeit_time = m.contact_end_time_;
                            } 
                            if (m.contact_start_time_ > local_best_delivery_time) {
                                local_best_delivery_time = m.contact_start_time_;
                            }
                            // this line won't let searching end imediately, still would find the available one in this branch.
                            {
                                cgr_find_proximate_count_ += 1;
                            }
                            // Note the computed forfeit time and best-case delivery time in the event that the bundle is queued for transmission to D.
                            assert(local_time_ < local_forfeit_time);
                            proximate_vec_.push_back(cur_d);
                            debugFunc04(m,cur_deadline,local_forfeit_time,-1, "tail"); 
                            CRT_.PushFinalXmit(m, local_forfeit_time, local_best_delivery_time);
                            CRT_.PopOneXmit();

                        }
                    } else {
                        debugFunc03("body of recurrsive");
                        // 2.B.2
                        auto found_2 = find(excluded_vec_.begin(), excluded_vec_.end(), s); // excluded_vec_ would include from_node but not own_node
                        bool s_is_in_excluded = found_2 != excluded_vec_.end();
                        if (s_is_in_excluded) {
                            NS_LOG_INFO(LogPrefixMacro<<"contine-4:current excluded_vec=" << vecstr(excluded_vec_));

                            continue;
                        } else {
                            if (m.contact_end_time_ < local_forfeit_time) {
                                local_forfeit_time = m.contact_end_time_;
                            }
                            if (m.contact_start_time_ > local_best_delivery_time) {
                                local_best_delivery_time = m.contact_start_time_;
                            }
                            double forwarding_latency = (2 * ecc_) / m.data_transmission_rate_;     // it's a very small account
                            double next_deadline = min((m.contact_end_time_ - forwarding_latency), cur_deadline) - NS3DTNBIT_CGR_CRP_REMAIN_INNODE;
                            if (next_deadline < local_time_) {
                                NS_LOG_INFO(LogPrefixMacro<<"contine-5:");

                                continue;
                            }
                            debugFunc04(m,cur_deadline,local_forfeit_time,next_deadline,"body");
                            assert(local_time_ < next_deadline);
                            debugFunc01(cur_d, "push");
                            CRT_.PushOneXmit(m);
                            ContactReviewProcedure(s, next_deadline, local_best_delivery_time);
                            NS_LOG_INFO(LogPrefixMacro<<"back to upper CRP,cur_d="<< cur_d <<";cur_deadline="<< cur_deadline);
                            debugFunc01(-1,"pop");
                            CRT_.PopOneXmit();

                        }
                    }
                }
            }
            // 3.
            excluded_vec_.erase(find(excluded_vec_.begin(), excluded_vec_.end(), cur_d));
        }

        CGRRouting::CGRRouting(DtnApp& dp) : RoutingMethodInterface(dp), CRT_(CashedRouteTable(*this)) {
#ifdef CGR_DEBUG_0
            cgr_debug_flag_0 = true;
#endif
#ifdef CGR_DEBUG_1
            cgr_debug_flag_1 = true;
#endif
        }

        int CGRRouting::ForwardDecision(vector<RouteResultCandidate> & rrc_vec) {
            for (auto rrc:rrc_vec) {
                if (rrc.is_exhausted_ && rrc_vec.size() != 1) {
                    NS_LOG_ERROR(LogPrefixMacro << rrc.ToString());

                    std::abort();
                }
            }
            if (rrc_vec.size() >= 2) {
                NS_LOG_INFO(LogPrefixMacro<<"It's random . we need to implement a network configuration matter decision!");
                return rrc_vec[NCMDecision(rrc_vec)].nexthop_;

            } else if (rrc_vec.size() == 1) {
                if (rrc_vec[0].is_exhausted_) {
                    return -2;
                } else {
                    return rrc_vec[0].nexthop_;
                }
            } else {
                NS_LOG_ERROR(LogPrefixMacro);
                std::abort();
            }
        }

        // check https://tools.ietf.org/html/draft-burleigh-dtnrg-cgr-00 -------- 2.5.3
        int CGRRouting::NCMDecision(vector<RouteResultCandidate> const & rrc_vec) {
            assert(rrc_vec.size() >= 2);
            int ret = std::rand();
            return ret % rrc_vec.size();

        }

    }
} /* ns3  */ 