#!/usr/bin/python3

#version 0.2

import fileinput
import re
import string
import sys

pattern = re.compile("^(?P<node>\d+):(?P<edges>(\d+(;\d+)*)?)")
default_port = 8080

nodes = {}
ignored_lines = []

if len(sys.argv) > 1:
    netname = sys.argv[1]
else:
    netname = 'defaultNet'

for line in sys.stdin:
    match = pattern.search(line)
    if match != None:
        nodes[match.group("node")] = match.group("edges").split(';')
    else:
        ignored_lines.append(line[:-1])


tpl_nedfile="""package darknetsim;

import inet.nodes.inet.StandardHost;
import inet.nodes.inet.Router;
import inet.networklayer.autorouting.FlatNetworkConfigurator;
import ned.DatarateChannel;

network %s
{
    @display("bgb=454,255");
    submodules:
        configurator: FlatNetworkConfigurator {
            @display("p=24,28;i=block/cogwheel");
            networkAddress = "23.42.1.0";
            netmask = "255.255.0.0";
        }
        router: Router {
            @display("p=34,150");
        }
%s
    connections:
%s
}"""

tpl_inifile="""[General]
network = %s
sim-time-limit = 8s

# UDP Applications
**.numUdpApps=1 # 0 means no UDP apps active.
**.udpAppType="FloodingNode"
#**.udpAppType="HotpotatoNode"

**.udpApp[0].localPort=%s
%s

**.host0.udpApp[0].pingID="host4"
**.udpApp[0].pingID=""

**.udpApp[0].resendTimer=0.5
**.udpApp[0].resendCounter=3

# IP settings
**.routingFile=""
**.ip.procDelay=10us

# ARP configuration
**.arp.retryTimeout = 1s
**.arp.retryCount = 3
**.arp.cacheTimeout = 100s
**.networkLayer.proxyARP = true  # Host's is hardwired "false"

# NIC configuration in hosts and routers
**.ppp[*].queueType = "DropTailQueue"
**.router.ppp[*].queue.frameCapacity = 100 # packets
**.ppp[*].queue.frameCapacity = 500 # packets
"""

def ned_make_hosts(nodes):
    ret = []
    for node in nodes.keys():
        ret.append("\thost"+node+": StandardHost {\n\t}")
    return ret

def ned_make_router_connections(nodes):
    ret = []
    for node in nodes.keys():
        ret.append("\trouter.pppg++ <--> DatarateChannel <--> host"+node+".pppg++;")
    return ret

def ini_conf_hosts(nodes):
    ret = []
    for node in nodes.keys():
        ret.append("**.host"+node+".udpApp[0].nodeID=\"host"+node+"\"")
        peers=[]
        for peer in nodes[node]:
            peers.append("host"+peer+":"+str(default_port))
        ret.append("**.host"+node+".udpApp[0].destID=\""+(" ".join(peers))+"\"")
    return ret


nedfile = open(netname+".ned","w")
nedfile.write(tpl_nedfile % (netname,"\n".join(ned_make_hosts(nodes)),"\n".join(ned_make_router_connections(nodes))))
nedfile.close()
inifile = open(netname+".ini","w")
inifile.write(tpl_inifile % (netname,default_port,"\n".join(ini_conf_hosts(nodes))))
inifile.close()

