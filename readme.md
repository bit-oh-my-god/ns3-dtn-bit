# Preface
This Project is a simulate kit for dtn on ns-3 environment, easy to use for research purples.
This project is highly inspirited by [Lakkakorpi](https://www.netlab.tkk.fi/tutkimus/dtn/ns/)

# structure of this project

* editor tool :
editor Vim do I use, if you are using YCM for code complete, remember add pull path include path to ~/.ycm_extra_config.py
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
there is one tutorial that I think may help you [ref](https://www.youtube.com/watch?v=HW29067qVWk&t=1568s)
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

            ack--->
            bundel---->     SocketSendDetail()
            anti-pkt----> 

            hello---->  CreateHelloBundleAndSendDetail()

# TODO list

    * <s>wifi delay </s> done by 
            wifiChannel.AddPropagationLoss ("ns3::RangePropagationLossModel",  
                    "MaxRange", DoubleValue (4000.0));
    * <s>note that ns2mobilityhelper only have ability to parse velocity 2d, extend it to support 3d </s> done by modify it, copy /box/ns2-mobilityhelp to src/mobility/helper to use it.
    would support '$ns at $time $node setdest x2 y2 z2 speed' format, and this format only.
        
    * <s>relative path </s> done by 
            config.txt
    * transmit session
    * time extented graph like that [paper]()

# Develpment Annoucement & Publish Log

    * First edition, ns3dtnbit-1.0, we are going to read [this](https://www.netlab.tkk.fi/tutkimus/dtn/ns/), then rename variables methods and do code refactoring.
    * Second edition, run a real example, and parse the log.
        * get trace file done 29.Feb
        * get jupyter to have an animation 5.Mar
            <s>fine, we got a idle loop, and I can't fix it</s>
            abstract example-interface
        * debug a bunch
            abstract routing-method interface
