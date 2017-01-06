# readme


* Acknoeledgement Annoucement 
    following torminologies are used to name variables in this project
            source_node         from_node           forward_node           destination_node
                |                   |                       |                   |
                |                   |                       |                   |
                |                   |                       |                   |
                ()                  ()                      ()                  ()
                
                *-------------------------------------------------------------> *
                        this is called "send"
                *-----------------> * --------------------> * ----------------> *
                        this is called "transmition & retransmition"
                        note : "store and forward" is just semantic
                
                a. every node have a daemon(or agent)
                    every daemon keep some state : 1. //TODO
                b. bundle can be fragment, which is also called bundle, so a bundle can be forward as one or multiple "bundle".
                    and packet which owns particular meaning as 'bytes in link', is only scoped in 'sending and receiving scenes',
                    while one utility name of ns-3 is also called 'packet'.
                    bp fragment specification[ref](https://tools.ietf.org/html/rfc5050#page-32)
                c. we should implement prophet router which is dependent on dynamic neighbor discovery //TODO
                d. size of pkt header, bytes of buffer queue, 
                e. baq means bundle and antipacket queue
                f. every header has a source unique id, which can be used to identify one header, under the assitance of source addr
                g. how 'dtn.cc' updates 'neighbor_sent_aps'? why? //TODO
                j. antipacket should be sent to 'source_node' every bundle arrive at 'dst_node', this antipacket is sth like 'standard dtn bundle report'
                k. when talk about seqnof in neighbor info context, negative means this bpheader is sent from this node and received at neighbor, but in original code, the negative sign aways come with antipacket? // TODO
                l. we don't need congestion control
                m. we should use 'app header payload' or we do not need it?
                n. how should we do with packet payload? TODO
                z. we don't need packet tag anymore TODO
                x. for simulator, every 'send movement' is called flow, and you can use flowmoniter to collect the performance of the simulation

* Develpment Annoucement

    * First edition, ns3dtnbit-1.0, we are going to read [this](https://www.netlab.tkk.fi/tutkimus/dtn/ns/), then rename variables methods and do code refactoring.
