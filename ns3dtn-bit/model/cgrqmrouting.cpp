
#include "cgrqmrouting.h"

namespace ns3 {
//
    // =================================== CGRQM
    //
    namespace ns3dtnbit {

        CGRQMRouting::CGRQMRouting(DtnApp& dp) : RoutingMethodInterface(dp) {
#ifdef CGR_DEBUG_0
            cgr_debug_flag_0 = true;
#endif
#ifdef CGR_DEBUG_1
            cgr_debug_flag_1 = true;
#endif
        }

        // maintain storageinfo_maintained_ and storage_max_ in this method
        // CGRQM TODO
        void CGRQMRouting::StorageinfoMaintainInterface(string action
                ,map<int, pair<int, int>> parsed_storageinfo_from_neighbor
                ,map<int, pair<int, int>>& move_storageinfo_to_this
                ,map<int, int> storagemax
                ,vector<int> path_of_route
                ,pair<int, int> update_storage_from_hello
                ) {
            {
                // CGRQM TODO
                // release storage
            }
            if (action == "route answer is made, add queue usage, and time to decay") {
                // Do route answer is made means that pkt is acked? FIXME
                for (auto nodeinpath : path_of_route) {
                    storageinfo_maintained_[nodeinpath] = {storageinfo_maintained_[nodeinpath].first, storageinfo_maintained_[nodeinpath].second + 1};
                }
                release_queue_[Simulator::Now().GetSeconds()] = path_of_route;
            } else if (action == "receive neighbor storageinfo") {
                for (auto pp : parsed_storageinfo_from_neighbor) {
                    auto nodeid = pp.first;
                    auto belive = pp.second.first;
                    auto storevalue = pp.second.second;
                    if (storageinfo_maintained_.count(nodeid)) {
                        int be, st;
                        int b1 = storageinfo_maintained_[nodeid].first;
                        int s1 = storageinfo_maintained_[nodeid].second;
                        be = (b1 + belive) / 2;
                        st = (b1 * s1 + belive * storevalue) / (b1 + belive);
                        storageinfo_maintained_[nodeid] = {be, st};
                    } else {
                        storageinfo_maintained_[nodeid] = {belive + 1, storevalue};
                    }
                }
            } else if (action == "to send storageinfo to neighbor") {
                move_storageinfo_to_this = storageinfo_maintained_;
            } else if (action == "give storage_max_") {
                storage_max_ = storagemax;
            } else if (action == "update storage info from hello") {
                storageinfo_maintained_[update_storage_from_hello.first] = {1, storage_max_[update_storage_from_hello.first] - update_storage_from_hello.second};
            } else {
                cout << "StorageinfoMaintainInterface: can't find action, action str is :" << action << "\n would fatal." << endl;
                std::abort();
            }
        }

        int CGRQMRouting::DoRoute(int s, int d) {
            cout << "DEBUG_CGR:" << " ============== A new DoRoute ==========\n" << endl;
            Init();
            assert(d == destination_id_);
            assert(s == own_id_);
            // once exhausted search, next time won't search, because timeout won't change. Is this colollary correct? FIXME
            auto founds = find(exhausted_search_target_list_.begin(), exhausted_search_target_list_.end(), make_pair(d, debug_cgr_that_seqno_));
            // if is found in routed_table_, we would reuse the result.
            auto foundsx = find_if(routed_table_.begin(), routed_table_.end(), 
                    [this](tuple<node_id_t, dtn_time_t, dtn_time_t, node_id_t, dtn_seqno_t, dtn_time_t, vector<int>> rtele){
                    // get<0>(rtele) == destination_id_ 
                    //&& (get<4>(rtele) == debug_cgr_that_seqno_ ||  (local_time_ - get<5>(rtele)) < (NS3DTNBIT_BUFFER_CHECK_INTERVAL * 15)) 
                    //&& (get<1>(rtele) - (NS3DTNBIT_BUFFER_CHECK_INTERVAL * 3)) > local_time_) 
                        if ( get<0>(rtele) == destination_id_
                                && (get<1>(rtele) - NS3DTNBIT_BUFFER_CHECK_INTERVAL * 2) > local_time_
                                && (local_time_ - get<5>(rtele)) < NS3DTNBIT_CGR_OPTIMAL_OPTION_REUSE_INTERVAL
                           ) { return true; } else { return false; }
                    });
            bool reuse_flag = NS3DTNBIT_CGR_OPTIMAL_OPTION;
            if (founds != exhausted_search_target_list_.end() && reuse_flag) {
                cout << "DEBUG_CGR:" << "WARN:may make bad routing-1. this dest is searched before, and no available one is found, " 
                    << "so this time it would exhausted search possiblity too. we would return a -1" 
                    << " .This won't affect routing result, but can reduce simulation time." 
                    << ";local_time_ = " << local_time_ 
                    << ";debug_cgr_that_seqno_ = " << debug_cgr_that_seqno_
                    << endl;
                return -1;
            } else if (foundsx != routed_table_.end() && reuse_flag) {
                tuple<node_id_t, dtn_time_t, dtn_time_t, node_id_t, dtn_seqno_t, dtn_time_t, vector<int>> tmpfsx = *foundsx;
                cout << "DEBUG_CGR:" << "WARN:may make bad routing-2. reuse routing result, reduce routing time" 
                    << ";local_time_ = " << local_time_ 
                    << ";debug_cgr_that_seqno_ = " << debug_cgr_that_seqno_
                    << ";reused result = " << get<3>(tmpfsx) 
                    << endl;
                // CGRQM TODO
                map<int, pair<int, int>> qm_empty01;
                map<int, pair<int, int>> qm_empty02;
                map<int, int> qm_empty03;
                StorageinfoMaintainInterface("route answer is made, add queue usage, and time to decay", qm_empty01, qm_empty02, qm_empty03, get<6>(tmpfsx)); 
                return get<3>(tmpfsx);
            }
            ContactReviewProcedure(destination_id_, forfeit_time_, best_delivery_time_, vector<int>());
            cout << "DEBUG_CGR:" << "before ForwardDecision()" 
                << "BundleTrace:entertimes=" << debug_crp_enter_count_ << "times" << endl;
            int result_index = ForwardDecision();
            int result = result_index >= 0 ? proximate_vec_[result_index] : -1;
            if (debug_cgr_this_exhausted_search_not_found_) {
                assert(result == -1);
                exhausted_search_target_list_.push_back(make_pair(d, debug_cgr_that_seqno_));
            } else {
                tuple<node_id_t, dtn_time_t, dtn_time_t, node_id_t, dtn_seqno_t, dtn_time_t, vector<int>> tmprvec = 
                    make_tuple(
                            destination_id_, 
                            final_forfeit_time_[result_index], 
                            final_best_delivery_time_[result_index], 
                            result, 
                            debug_cgr_that_seqno_, 
                            local_time_,
                            proximate_path_vec_[result_index]);
                assert(local_time_ < final_forfeit_time_[result_index]);
                if (cgr_debug_flag_0) {
                    cout << "CGR_DEBUG:"
                        << "push one route result"
                        << ";destination_id_=" << destination_id_
                        << ";forfeit_time_=" << final_forfeit_time_[result_index]
                        << ";final_best_delivery_time_=" << final_best_delivery_time_[result_index]
                        << ";result=" << result
                        << ";debug_cgr_that_seqno_=" << debug_cgr_that_seqno_
                        << ";local_time_=" << local_time_ 
                        << ";vec path=" << " not yet!" << endl;
                }
                routed_table_.push_back(tmprvec);
                // CGRQM TODO
                map<int, pair<int, int>> qm_empty01;
                map<int, pair<int, int>> qm_empty02;
                map<int, int> qm_empty03;
                StorageinfoMaintainInterface("route answer is made, add queue usage, and time to decay", qm_empty01, qm_empty02, qm_empty03, proximate_path_vec_[result_index]); 
            }
            assert(result < 100);
            cout << "DEBUG_CGR:"  << " ================== end this DoRoute() =========\n"
                << "result= " << result 
                << ";CGR-" << s << "->" << d << endl;
            return result;
        }

        void CGRQMRouting::GetInfo(node_id_t destination_id, 
                node_id_t from_id, std::vector<node_id_t> vec_of_current_neighbor, 
                node_id_t own_id, dtn_time_t expired_time, 
                uint32_t bundle_size, map<node_id_t, vector<node_id_t>> id2cur_exclude_vec_of_id, 
                dtn_time_t local_time, dtn_seqno_t that_seqno) {
            id_of_d2cur_excluded_vec_of_d_=id2cur_exclude_vec_of_id;
            local_time_ = local_time;
            destination_id_ = destination_id;
            node_id_transmit_from_ = from_id;
            expired_time_ = expired_time;
            own_id_ = own_id;
            debug_cgr_that_seqno_ = that_seqno;
            ecc_ = bundle_size;
            id_of_current_neighbor_ = vec_of_current_neighbor;
        }

        void CGRQMRouting::Init() {
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
            proximate_path_vec_ = vector<vector<int>>();
            final_forfeit_time_.clear();
            final_best_delivery_time_.clear();
            auto cur_excluded = id_of_d2cur_excluded_vec_of_d_[destination_id_];
            for (auto nei : id_of_current_neighbor_) {
                auto found = find(cur_excluded.begin(), cur_excluded.end(), nei);
                if (found != cur_excluded.end()) {
                    excluded_vec_.push_back(nei);
                }
            }
            excluded_vec_.push_back(node_id_transmit_from_);
        }

        void CGRQMRouting::DebugPrintXmit(vector<CgrXmit>& cgr_xmit_vec_ref, int cur_d) {
            {
                cout << "DEBUG_CGR:" << " ====== for node-" << cur_d 
                    << " xmit is :" << endl;
                for (auto m : cgr_xmit_vec_ref) {
                    cout << "\n m ==> m.contact_start_time_ =" << m.contact_start_time_
                        << ";m.contact_end_time_=" << m.contact_end_time_
                        << ";m.node_id_of_from_=" << m.node_id_of_from_
                        << ";m.node_id_of_to_=" << m.node_id_of_to_
                        << ";m.data_transmission_rate_=" << m.data_transmission_rate_ << endl;
                }
                cout << "end of DEBUG_CGR" << endl;
            }

        }

        /*
         * Please read this link, if you want to know about this. https://tools.ietf.org/html/draft-burleigh-dtnrg-cgr-00
         *  ---- cur_d 
         *  ---- cur_deadline
         *  ---- proximate_vec_
         *  ---- excluded_vec_
         *  ---- forfeit_time_
         *  ---- best_delivery_time_
         * */
        void CGRQMRouting::ContactReviewProcedure(node_id_t cur_d, dtn_time_t cur_deadline, dtn_time_t best_deli, vector<node_id_t> cur_d_path) {
            debug_crp_enter_count_ += 1;
            assert(debug_crp_enter_count_ < 300);
            if (cgr_debug_flag_1) {
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
            if (cgr_find_proximate_count_ >= NS3DTNBIT_CGR_OPTIMAL_DECISION_AMOUNT) {
                if (cgr_debug_flag_1) { debug_recurrsive_deep_ -= 1; } 
                return;
            }
            // 1.
            excluded_vec_.push_back(cur_d);
            const Adob& ref_adob = RoutingMethodInterface::get_adob();
            auto& cgr_xmit_vec_ref = ref_adob.node_id2cgr_xmit_vec_map_[cur_d];
            if (cgr_debug_flag_0) {
                if (debug_cgr_that_seqno_ == NS3DTNBIT_CGR_DEBUG_SEQ_1 || debug_cgr_that_seqno_ == NS3DTNBIT_CGR_DEBUG_SEQ_2) {
                    cout << "CGR_DEBUG:temporary debug use, deleteme when you don't need me, xmitdebug-seqno-" << debug_cgr_that_seqno_ << __FILE__ << ":" <<  __LINE__ 
                        << ";own_id_ = " << own_id_ 
                        << ";local_time_ = " << local_time_ 
                        << endl;
                    DebugPrintXmit(cgr_xmit_vec_ref, cur_d);
                }
            }
            // 2.
            for (auto& m : cgr_xmit_vec_ref) {
                bool last_moment_check = 
                    local_time_ < cur_deadline - NS3DTNBIT_BUFFER_CHECK_INTERVAL * 2
                    && m.contact_start_time_ + NS3DTNBIT_BUFFER_CHECK_INTERVAL * 2< cur_deadline 
                    && local_time_ < m.contact_end_time_ - NS3DTNBIT_BUFFER_CHECK_INTERVAL * 2;
                dtn_time_t local_forfeit_time = cur_deadline;
                dtn_time_t local_best_delivery_time = best_deli;
                if (!last_moment_check) {
                    // 2.A
                    continue;
                } else {
                    // 2.B
                    node_id_t s = m.node_id_of_from_;
                    bool s_is_local_node_with_own_id = s == own_id_;
                    if (s_is_local_node_with_own_id) {
                        if (cgr_debug_flag_1) {
                            cout << "DEBUG_CGR:" << "in tail of recursive, and, if we find a proximate one,"
                                << "we would break the search, because the CGR algorithm in RFC would force to find all possible pathes,"
                                << " which is too big for some senario, mainly the group moving senario " << endl;
                        }
                        // 2.B.1
                        // compute ECC for this bundle
                        int ecc = ecc_;
                        assert(m.contact_end_time_ - m.contact_start_time_ < 5000);
                        // not accurate, just a hypothetic value,  TODO
                        int ecc_of_other_bundles = 1800;
                        int residual_capacity = ((m.contact_end_time_ - m.contact_start_time_) * m.data_transmission_rate_) - ecc_of_other_bundles; 
                        assert(residual_capacity > 10000);
                        auto found_1 = find(proximate_vec_.begin(), proximate_vec_.end(), cur_d);
                        bool d_is_in_proximate = found_1 != proximate_vec_.end();
                        if (residual_capacity < ecc) {
                            continue;
                        } else if (d_is_in_proximate) {
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
                            cur_d_path.push_back(cur_d);
                            proximate_path_vec_.push_back(cur_d_path);
                            final_forfeit_time_.push_back(local_forfeit_time);
                            final_best_delivery_time_.push_back(local_best_delivery_time);
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
                        }
                    } else {
                        if (cgr_debug_flag_1) {
                            cout << "DEBUG_CGR:" << "in body of recursive" << endl;
                        }
                        // 2.B.2
                        auto found_2 = find(excluded_vec_.begin(), excluded_vec_.end(), s);
                        bool s_is_in_excluded = found_2 != excluded_vec_.end();
                        if (s_is_in_excluded) {
                            continue;
                        } else {
                            if (m.contact_end_time_ < local_forfeit_time) {
                                local_forfeit_time = m.contact_end_time_;
                            }
                            if (m.contact_start_time_ > local_best_delivery_time) {
                                local_best_delivery_time = m.contact_start_time_;
                            }
                            double forwarding_latency = (2 * ecc_) / m.data_transmission_rate_;     // it's a very small account
                            double next_deadline = min((m.contact_end_time_ - forwarding_latency), cur_deadline) - NS3DTNBIT_BUFFER_CHECK_INTERVAL;
                            if (next_deadline < local_time_) {
                                cout << "CGR_DEBUG:" << __LINE__
                                    << ";m.contact_end_time_=" << m.contact_end_time_
                                    << ";next_deadline=" << next_deadline
                                    << ";cur_deadline=" << cur_deadline
                                    << ";forwarding_latency=" << forwarding_latency
                                    << ";local_time_=" << local_time_ << endl;
                                continue;
                            }
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
                            assert(local_time_ < next_deadline);
                            auto cur_d_path_copy = cur_d_path;
                            cur_d_path_copy.push_back(cur_d);
                            ContactReviewProcedure(s, next_deadline, local_best_delivery_time,cur_d_path_copy);
                        }
                    }
                }
            }
            // 3.
            excluded_vec_.erase(find(excluded_vec_.begin(), excluded_vec_.end(), cur_d));
            if (cgr_debug_flag_1) {
                debug_recurrsive_deep_ -= 1;
                debug_recurrsive_path_stack_.pop();
            }
        }

        int CGRQMRouting::ForwardDecision() {
            if (proximate_vec_.empty()) {
                std::cout << "Warn: cgr can't find one !" << __LINE__ << endl;
                debug_cgr_this_exhausted_search_not_found_ = true;
                return -1;
            } else {
                if (proximate_vec_.size() >= 2) {
                    std::cout << "Warn : we need to implement a network configuration matter decision!" << endl;
                    return NCMDecision();
                } else if (proximate_vec_.size() == 1) {
                    return 0;
                } else {
                    std::cout << "Error:" << __FILE__ <<  __LINE__ << endl;
                    std::abort();
                }
            }
        }

        // check https://tools.ietf.org/html/draft-burleigh-dtnrg-cgr-00 -------- 2.5.3
        int CGRQMRouting::NCMDecision() {
            assert(proximate_vec_.size() >= 2);
            assert(proximate_path_vec_.size() == proximate_vec_.size());
            map<int, int> score_map;
            for (int i = 0; i < proximate_path_vec_.size(); i++) {
                auto route_path = proximate_path_vec_[i];
                auto prox_node = proximate_vec_[i];
                assert(score_map.count(prox_node)==0);  // overwrite? yes.
                int smallestinroutepath = INT_MAX;
                for (auto node_inpath : route_path) {
                    assert(storage_max_.count(node_inpath));
                    if (!storageinfo_maintained_.count(node_inpath)) {
                        storageinfo_maintained_[node_inpath] = {1, 0};
                    } 
                    auto storagemaintained = storageinfo_maintained_[node_inpath].second;
                    auto remain = storage_max_[node_inpath] - storagemaintained;
                    smallestinroutepath = smallestinroutepath < remain ? smallestinroutepath : remain;
                }
                score_map[prox_node] = smallestinroutepath;
            }
            int index_of_biggestsmall = 0;
            for (auto pair_v : score_map) {
                if (get<1>(pair_v) > score_map[proximate_vec_[index_of_biggestsmall]]) {
                    index_of_biggestsmall = find(proximate_vec_.begin(), proximate_vec_.end(), get<0>(pair_v)) - proximate_vec_.begin();
                }
            }
            return index_of_biggestsmall;
        }
    } /* ns3dtnbit */ 
} /* ns3  */ 