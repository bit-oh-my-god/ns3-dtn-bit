set val(chan)   Channel/WirelessChannel    ;# channel type
set val(prop)   Propagation/TwoRayGround   ;# radio-propagation model
set val(netif)  Phy/WirelessPhy            ;# network interface type
set val(mac)    Mac/802_11                 ;# MAC type
set val(ifq)    Queue/DropTail/PriQueue    ;# interface queue type
set val(ll)     LL                         ;# link layer type
set val(ant)    Antenna/OmniAntenna        ;# antenna model
set val(ifqlen) 50                         ;# max packet in ifq
set val(rp)     AODV                       ;# routing protocol
source scenario.ns_params
set ns [new Simulator]

set topo       [new Topography]
$topo load_flatgrid $val(x) $val(y)
create-god $val(nn)

set tracefile [open out.tr w]
$ns trace-all $tracefile

set namfile [open out.nam w]
$ns namtrace-all $namfile
$ns namtrace-all-wireless $namfile $val(x) $val(y)
set chan [new $val(chan)];#Create wireless channel

#===================================
#     Mobile node parameter setup
#===================================
$ns node-config -adhocRouting  $val(rp) \
                -llType        $val(ll) \
                -macType       $val(mac) \
                -ifqType       $val(ifq) \
                -ifqLen        $val(ifqlen) \
                -antType       $val(ant) \
                -propType      $val(prop) \
                -phyType       $val(netif) \
                -channel       $chan \
                -topoInstance  $topo \
                -agentTrace    ON \
                -routerTrace   ON \
                -macTrace      ON \
                -movementTrace ON

#Create 10 nodes
set n0 [$ns node]
$ns initial_node_pos $n0 20
set n1 [$ns node]
$ns initial_node_pos $n1 20
set n2 [$ns node]
$ns initial_node_pos $n2 20
set n3 [$ns node]
$ns initial_node_pos $n3 20
set n4 [$ns node]
$ns initial_node_pos $n4 20
set n5 [$ns node]
$ns initial_node_pos $n5 20
set n6 [$ns node]
$ns initial_node_pos $n6 20
set n7 [$ns node]
$ns initial_node_pos $n7 20
set n8 [$ns node]
$ns initial_node_pos $n8 20
set n9 [$ns node]
$ns initial_node_pos $n9 20
source scenario.ns_movements
#===================================
#        Agents Definition        
#===================================
#Setup a TCP connection
set tcp0 [new Agent/TCP]
$ns attach-agent $n0 $tcp0
set sink1 [new Agent/TCPSink]
$ns attach-agent $n2 $sink1
$ns connect $tcp0 $sink1
$tcp0 set packetSize_ 1500


#===================================
#        Applications Definition        
#===================================
#Setup a FTP Application over TCP connection
set ftp0 [new Application/FTP]
$ftp0 attach-agent $tcp0
$ns at 1.0 "$ftp0 start"
$ns at 2.0 "$ftp0 stop"


#===================================
#        Termination        
#===================================
#Define a 'finish' procedure
proc finish {} {
    global ns tracefile namfile
    $ns flush-trace
    close $tracefile
    close $namfile
    exec nam out.nam &
    exit 0
}
for {set i 0} {$i < $val(nn) } { incr i } {
    $ns at $val(duration) "\$n$i reset"
}
$ns at $val(duration) "$ns nam-end-wireless $val(duration)"
$ns at $val(duration) "finish"
$ns at $val(duration) "puts \"done\" ; $ns halt"
$ns run
