# Preface
This project is highly inspirited by [Lakkakorpi](https://www.netlab.tkk.fi/tutkimus/dtn/ns/), we would call him "the author or Lakkakorpi"

# structure of this project

* bonnmotion 
it is a software depending on java used to generate location files
we just need the **.ns_movements** file

* bonnmotion + ns2 + nam + nsg2.1
[ref](http://www.nsnam.com/2015/03/bonnmotion-mobile-scenario-generator.html) [ref](http://chandra-ns2.blogspot.com/2009/01/how-to-run-bonnmotion-for-ns-2.html)
[how to use this combo](https://www.slideshare.net/manasGaur1/bonn-motion-traffic-generation-and-nam)

* CMU setdest
The [author](https://www.netlab.tkk.fi/tutkimus/dtn/ns/) Lakkakorpi used this CMU setdest program, or should I use CMU setdest program in ns3? [ref](http://www.isi.edu/nsnam/ns/tutorial/nsscript7.html)
but, I can't get 3d in "setdest", this problem aside, let's focus on how to use this setdest, in my machine, I did this "$sudo apt install ns2"
so I have the setdest in my "/usr/bin/setdest"

* ns3 and ns3-dtn-bit module

* jupyter and python-matplotlib
This combo is used to make graph and paper, check /box/jupyter
if you want use jupyter yourself, install it on org-web, in China, you may not be able to download 0.5 G big file from the web.
you would need to install miniconda first, and install full-package from miniconda, that's the way I did.

# develop log

* Acknoeledgement Annoucement 
    following graph are used to name variables in this project
            source_node         from_node               to_node           destination_node
                |                   |                       |                   |
                |                   |                       |                   |
                |                   |                       |                   |
                ()                  ()                      ()                  ()
                
                *-------------------------------------------------------------> *
                        this is called "send"
                *-----------------> * --------------------> * ----------------> *
                        this is called "transmition & retransmition"
                        note : "store and forward" is just semantic
                
* the post it note while write code, not useful for you.

            a. it's a very good practice to use p_pkt->Copy(), Ptr<> is boost::instrusive_ptr, 
                which is similar to shared_ptr, which canbe used under some situation like one object to multi-pointers. 
                And in this situation, it could be dangerous to use p_pkt directly without copy. 
            b. bundle can be fragment, which is also called bundle, so a bundle can be forward as one or multiple "bundle".
                and packet which owns particular meaning as 'bytes in link', is only scoped in 'sending and receiving scenes',
                while one utility name of ns-3 is also called 'packet'.
                bp fragment specification[ref](https://tools.ietf.org/html/rfc5050#page-32)
            c. we should implement prophet router which is dependent on dynamic neighbor discovery //TODO
            d. Important! the 'sendmore after ack is a bad design' for this scence of dtn TODO
            e. baq means bundle and antipacket queue
            f. every header has a source unique id, which can be used to identify one header, under the assitance of source addr
            g. how 'dtn.cc' updates 'neighbor_sent_aps', it doesn't update, it rollback.
            t. authoer use 'qostag' and 'flowidtag', what's the effect of 'flowidtag'?
                'qostag'specify the TID which will be used by a QoS-aware WifiMac for a given traffic flow //TODO
            j. antipacket should be sent to 'source_node' every bundle arrive at 'dst_node', this antipacket is sth like 'standard dtn bundle report'
            k. when talk about seqnof in neighbor info context, negative means this bpheader is sent from this node and received at neighbor, but in original code, the negative sign always come with antipacket? Now, we are not using any seqnof, because it's not safe to translate from int to uint TODO
            rt. use 'get_ip_and_seqno' to implement 'equation semantic' (done)
            l. we don't need congestion control
            m. we should use 'app header payload' or we do not need it?
            n. how should we do with packet payload? create packet with payload, then addheader to it.
            z. we don't need packet tag anymore TODO
            x. for simulator, every 'send movement' is called flow, and you can use flowmoniter to collect the performance of the simulation
            y. when talk about 'flowid' antipacket seqno is negative, bundle seqno is positive

# Develpment Annoucement & Publish Log

    * First edition, ns3dtnbit-1.0, we are going to read [this](https://www.netlab.tkk.fi/tutkimus/dtn/ns/), then rename variables methods and do code refactoring.
    * Second edition, run a real example, and parse the log.
