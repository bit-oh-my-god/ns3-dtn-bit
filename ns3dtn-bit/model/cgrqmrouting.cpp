#include "cgrqmrouting.h"

namespace ns3 {
//
    // =================================== CGRQM
    //
    namespace ns3dtnbit {

        NS_LOG_COMPONENT_DEFINE ("DtnCGRQMRouting");
        #define LogPrefixMacro LogPrefix()<<"[DtnCGRQMRouting]line-"<<__LINE__<<"]"

        CGRQMRouting::CGRQMRouting(DtnApp& dp) : CGRRouting(dp) { 
            // CGRQM didn't mean anything unless 
            assert(NS3DTNBIT_CGR_OPTIMAL_DECISION_AMOUNT > 1);
        }
        CGRQMRouting::~CGRQMRouting() {}

        void CGRQMRouting::DebugUseScheduleToDoSome() {
            //NS_LOG_DEBUG(LogPrefixMacro << "in DebugUseSchedule");
            for (auto const & me : storageinfo_maintained_) {
                NS_LOG_DEBUG(LogPrefixMacro<<"[Trace]storageinfo:" 
                << "nodeid=" << get<0>(me) 
                << "belive="<< get<1>(me).first 
                << "storage_v="<< get<1>(me).second);
            }
        }

        bool CGRQMRouting::ShouldForwardSI(Ipv4Address ip) {
            auto nowtime = Simulator::Now().GetSeconds();
            if (storageinfo_forward_map_.count(ip)) {
                if (storageinfo_forward_map_[ip] + NS3DTNBIT_CGR_QM_ALGORITHM_SI_FORWARD_INTERVAL > nowtime) {
                    storageinfo_forward_map_[ip] = nowtime;
                    return true;
                } else {
                    return false;
                }
            } else {
                storageinfo_forward_map_[ip] = nowtime;
                return true;
            }
        }

        void CGRQMRouting::LoadCurrentStorageOfOwn(node_id_t node, size_t usage) {
            NS_LOG_INFO("[loadcurrentstorageofown]update local storage usage" << usage << ":"<< int (usage));
            storageinfo_maintained_[node] = make_pair(int(1), int(usage));
            //for (auto itt: storageinfo_maintained_) {
            //    if (itt.second.second < 0) {
            //        NS_LOG_ERROR(itt.first << ":" << itt.second.first << ";" << itt.second.second);
            //        NS_ASSERT(itt.second.second >= 0);
            //    }
            //}
        }

        // maintain storageinfo_maintained_ and storage_max_ in this method
        // CGRQM TODO
        void CGRQMRouting::StorageinfoMaintainInterface(string action
                ,map<node_id_t, pair<int, int>> parsed_storageinfo_from_neighbor
                ,map<node_id_t, pair<int, int>>& move_storageinfo_to_this
                ,map<node_id_t, size_t> storagemax
                ,pair<vector<node_id_t>, dtn_time_t> path_of_route_and_decaytime
                ,pair<node_id_t, int> update_storage_from_hello
                ) {
            NS_LOG_INFO(LogPrefixMacro<<"Into storageMaintainInterface");
            auto nowtime = Simulator::Now().GetSeconds();
            if (last_release_check_time_ + NS3DTNBIT_CGR_QM_ALGORITHM_DECAY_CHECK_TIME < nowtime) {
                // local-pkt in bundle-queue would release when acked, if routing-method = QM in dtnapp
                last_release_check_time_ = nowtime;
                NS_LOG_INFO(LogPrefixMacro<<"to release storage and don't do it hot");
                while (!release_queue_.empty() && get<0>(*release_queue_.begin())<nowtime) {
                    for (auto nodeinpathtodecaystorage : (*release_queue_.begin()).second) {
                        if (storageinfo_maintained_[nodeinpathtodecaystorage].second < 1) {
                            // storage usage is too small
                        } else {
                            // release storage
                            //auto old = get<1>(storageinfo_maintained_[nodeinpathtodecaystorage]);
                            get<1>(storageinfo_maintained_[nodeinpathtodecaystorage]) -= 1;
                            //assert(old != storageinfo_maintained_[nodeinpathtodecaystorage].second);
                        }
                    }
                    // map is ordered, so the begin() points to is the smallest one
                    release_queue_.erase(release_queue_.begin());
                }
            } else {
                // don't release too hot
            }
            if (last_belive_decay_ + NS3DTNBIT_CGR_QM_ALGORITHM_DECAY_DO_TIME < nowtime) {
                last_belive_decay_ = nowtime;
                // increase belive
                for (auto & me : storageinfo_maintained_) {
                    get<0>(get<1>(me)) += 1;
                }
            }
            if (action == "route answer is made, add queue usage, and time to decay") {
                NS_LOG_INFO("route answer is made, add queue usage, and time to decay");
                //if (path_of_route_and_decaytime.second < nowtime - 200) {
                    //NS_LOG_INFO( " fuckyou!!"
                    //<< ";decay=" << path_of_route_and_decaytime.second 
                    //<< ";nowtime=" << nowtime
                    //);
                    //NS_ASSERT_MSG(path_of_route_and_decaytime.second > nowtime - 50, 
                    //"decay time should be contact_start_time of (last hop to dst)");
                //}
                auto sizeofdecaypath = path_of_route_and_decaytime.first.size();
                node_id_t nexthopindecaypath;
                if (sizeofdecaypath >= 1) {
                    nexthopindecaypath = path_of_route_and_decaytime.first[sizeofdecaypath - 1];
                    for (auto nodeinpath : path_of_route_and_decaytime.first) {
                        if (!storageinfo_maintained_.count(nodeinpath)) {
                            storageinfo_maintained_[nodeinpath] = {10, 0};
                        }
                        //auto old = get<1>(storageinfo_maintained_[nodeinpath]);
                        get<1>(storageinfo_maintained_[nodeinpath]) += 1;
                        NS_ASSERT(get<1>(storageinfo_maintained_[nodeinpath]) >= 0);
                        //assert(get<1>(storageinfo_maintained_[nodeinpath]) == old + 1);
                        //{storageinfo_maintained_[nodeinpath].first, storageinfo_maintained_[nodeinpath].second + 1};
                    }
                    NS_LOG_DEBUG(LogPrefixMacro << " route pkts is acked"
                    << ";add decaytime=" << path_of_route_and_decaytime.second 
                    << ";next hop=" << nexthopindecaypath);
                    release_queue_[path_of_route_and_decaytime.second] = path_of_route_and_decaytime.first;
                }
            } else if (action == "receive neighbor storageinfo") {
                NS_LOG_INFO(LogPrefixMacro<< "receive neighbor storageinfo");
                for (auto pp : parsed_storageinfo_from_neighbor) {
                    auto nodeid = pp.first;
                    auto belive = pp.second.first;
                    auto storevalue = pp.second.second;
                    assert(belive>=1);
                    assert(nodeid>=0);
                    assert(storevalue>=0);
                    if (storageinfo_maintained_.count(nodeid)) {
                        int be, st;
                        int b1 = storageinfo_maintained_[nodeid].first;
                        int s1 = storageinfo_maintained_[nodeid].second;
                        be = (b1 + belive) / 2;
                        // belive-value is bigger, the right is lower 
                        st = double(((belive * s1) + (b1 * storevalue))) /  double(b1 + belive);
                        //NS_LOG_INFO(";st=" << st << ";belive="<< belive << ";s1=" << s1 << ";b1=" << b1 << ";storevalue=" << storevalue);
                        assert(st <= s1 || st <= storevalue);
                        if (storage_max_[nodeid] < st) {
                            NS_LOG_INFO(LogPrefixMacro << ":"<<st << ":" << storage_max_[nodeid]);
                            assert(storage_max_[nodeid] >= st);
                        }
                        //NS_LOG_INFO("update storageinfo,nodeid=" << nodeid<< ";beli=" << be<<";stor=" << st);
                        NS_ASSERT(st >= 0);
                        storageinfo_maintained_[nodeid] = {be, st};
                    } else {
                        if (storage_max_[nodeid] < storevalue) {
                            NS_LOG_WARN(LogPrefixMacro << " storage max should >= storevalue"
                            <<";nodeid="<< nodeid << ";max=" <<storage_max_[nodeid] <<";sto="<< storevalue);
                            NS_ASSERT_MSG(storevalue < 1000000 ,"overflow?");
                        }
                        NS_LOG_INFO("update storageinfo,nodeid=" << nodeid<< ";beli=" << belive + 1<<";stor=" << storevalue);
                        NS_ASSERT(storevalue >= 0);
                        storageinfo_maintained_[nodeid] = {belive + 1, storevalue};
                    }
                }
            } else if (action == "to send storageinfo to neighbor") {
                NS_LOG_INFO("to send storageinfo to neighbor");
                move_storageinfo_to_this = storageinfo_maintained_;
            } else if (action == "give storage_max_") {
                NS_LOG_INFO("give storage_max_");
                storage_max_ = storagemax;
                for (auto it : storage_max_) {
                    NS_LOG_INFO(LogPrefixMacro<< "storagemax=" << it.second << "at node=" << it.first);
                }
            } else if (action == "update storage info from hello") {
                NS_LOG_INFO("update storage info from hello");
                // max - avai = usage
                NS_LOG_INFO("fuck128967" << ";nodeid=" << update_storage_from_hello.first << ";avai=" << update_storage_from_hello.second << ";max=" << storage_max_[update_storage_from_hello.first]);
                NS_ASSERT(update_storage_from_hello.second>=0);
                int usage = storage_max_[update_storage_from_hello.first] - (update_storage_from_hello.second);
                NS_ASSERT(usage >= 0);
                //NS_LOG_INFO(LogPrefixMacro<< "update storage usage from hello, node="<< update_storage_from_hello.first<<"usage=" << usage);
                NS_LOG_INFO("update storageinfo,nodeid=" << update_storage_from_hello.first<< ";beli=" << 1<<";stor=" << usage);
                storageinfo_maintained_[update_storage_from_hello.first] = {1, usage};  
                //update_storage_from_hello.second};
            } else {
                NS_LOG_ERROR(LogPrefixMacro<< "StorageinfoMaintainInterface: can't find action, action str is :" << action << "\n would fatal." );
                std::abort();
            }
            //NS_ASSERT((!storageinfo_maintained_.count(4)) || storageinfo_maintained_[0].second != 66);

            //for (auto itt: storageinfo_maintained_) {
            //    NS_LOG_INFO("fuckyou"<<itt.first << ":" << itt.second.first << ";" << itt.second.second);
            //    if (itt.second.second < 0) {
            //        NS_ASSERT(itt.second.second >= 0);
            //    }
            //}
            NS_LOG_INFO(LogPrefixMacro<<"outof storageMaintainInterface");
        }

        void CGRQMRouting::NotifyRouteSeqnoIsAcked(dtn_seqno_t seq){
            if (seqno2ackedcb_.count(seq)) {
                seqno2ackedcb_[seq]();
            } else {
                NS_ABORT_MSG("not cb");
            }
        }

        int CGRQMRouting::ForwardDecision(vector<RouteResultCandidate> & rrc_vec) {
            for (auto rrc:rrc_vec) {
                if (rrc.is_exhausted_ && rrc_vec.size() != 1) {
                    NS_LOG_ERROR(LogPrefixMacro << rrc.ToString());
                    std::abort();
                }
            }
            NS_LOG_DEBUG(LogPrefixMacro<<" forwarddecision: rrc_vec=" << rrc_vec.size());
            auto storagefunc = [&](size_t index){
                map<node_id_t, pair<int, int>> qm_empty01;
                map<node_id_t, pair<int, int>> qm_empty02;
                map<node_id_t, size_t> qm_empty03;
                pair<vector<node_id_t>, dtn_time_t> path_of_route;
                pair<node_id_t, int> emptyupdate ={ -1, -1 };
                double decaytime = rrc_vec[index].GetArriveDestTimeIfGood();
                auto func = std::bind(&CGRQMRouting::StorageinfoMaintainInterface, this, "route answer is made, add queue usage, and time to decay", 
                qm_empty01, qm_empty02, qm_empty03, make_pair(std::move(rrc_vec[index].GetPath()), decaytime), emptyupdate); 
                NS_ASSERT_MSG(decaytime > 0.0, "decay time should be >= 0.0");
                if (!seqno2ackedcb_.count(debug_cgr_that_seqno_)) {
                    seqno2ackedcb_[debug_cgr_that_seqno_] = std::move(func);
                }
            };
            if (rrc_vec.size() >= 2) {
                // multiplt choice
                size_t index_of_rrc_vec = NCMDecision(rrc_vec);
                storagefunc(index_of_rrc_vec);
                return rrc_vec[index_of_rrc_vec].nexthop_;
            } else if (rrc_vec.size() == 1) {
                if (rrc_vec[0].is_exhausted_) {
                    // only one choice and is exhausted-route
                    return -2;
                } else {
                    // only one choice and is good-route
                    storagefunc(0);
                    return rrc_vec[0].nexthop_;
                }
            } else {
                NS_LOG_ERROR(LogPrefixMacro);
                std::abort();
            }
        }

        // check https://tools.ietf.org/html/draft-burleigh-dtnrg-cgr-00 -------- 2.5.3
        int CGRQMRouting::NCMDecision(vector<RouteResultCandidate> const & rrc_vec) {
            map<size_t, int> score_map;
            vector<vector<node_id_t>> proximate_path_vec_;
            {
                // construct proximate_path_vec_
                for (auto const & rrc : rrc_vec) {
                    vector<node_id_t> path = rrc.GetPath();
                    NS_LOG_DEBUG(LogPrefixMacro << " in CGRQM::NCMDecision, one rrc.vecpathstr()=" << rrc.VecPathStr());
                    proximate_path_vec_.push_back(std::move(path));
                }
            }
            for (size_t i = 0; i < proximate_path_vec_.size(); i++) {
                vector<node_id_t> route_path = proximate_path_vec_[i];
                assert(score_map.count(i)==0);  // overwrite? yes.
                int smallestinroutepath = INT_MAX;
                for (auto const & node_inpath : route_path) {
                    assert(storage_max_.count(node_inpath));
                    if (!storageinfo_maintained_.count(node_inpath)) {
                        storageinfo_maintained_[node_inpath] = {1, 0};
                    } 
                    auto storagemaintained = storageinfo_maintained_[node_inpath].second;
                    auto remain = storage_max_[node_inpath] - storagemaintained;
                    smallestinroutepath = smallestinroutepath < remain ? smallestinroutepath : remain;
                }
                score_map[i] = smallestinroutepath;
            }
            size_t index_of_biggestsmall = 0;
            for (auto pair_v : score_map) {
                NS_LOG_DEBUG(LogPrefixMacro<< "score of hop(" << rrc_vec[pair_v.first].VecPathStr() << ")=" << pair_v.second);
                if (pair_v.second > score_map[index_of_biggestsmall]) {
                    index_of_biggestsmall = pair_v.first;
                }
            }
            NS_LOG_DEBUG(LogPrefixMacro << " decision.rrc.vecpathstr()= " << rrc_vec[index_of_biggestsmall].VecPathStr());
            return index_of_biggestsmall;
        }
    } /* ns3dtnbit */ 
} /* ns3  */ 