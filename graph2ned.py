#!/usr/bin/python3

#version 0.2

import fileinput
import re
import string
import sys
import random

pattern = re.compile("^(?P<node>\d+):(?P<edges>(\d+(;\d+)*)?)")
default_port = 8080

nodes = {}
ignored_lines = []

if len(sys.argv) > 1:
    netname = sys.argv[1]
    if len(sys.argv) > 2:
        targetCount = int(sys.argv[2])
    else:
        targetCount = 0
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
            networkAddress = "23.42.0.0";
            netmask = "255.255.0.0";
        }
        router: Router {
            @display("p=34,150");
        }
%s
    connections:
%s
}"""

tpl_ininetfile="""[General]
network = %s
**udpApp[0].localPort = %s

output-vector-file = ${resultdir}/${network}-%s-${configname}-${runnumber}.vec
output-scalar-file = ${resultdir}/${network}-%s-${configname}-${runnumber}.sca

%s


include general.ini
"""

tpl_inifile="""[General]

%s

include %s

[Config Randomwalk]
extends = General
**.udpAppType = "RandomwalkNode"

[Config Random2]
extends = Randomwalk
**.requestFanout = 2

[Config Random4]
extends = Randomwalk
**.requestFanout = 4

[Config Random8]
extends = Randomwalk
**.requestFanout = 8


[Config Flooding]
extends = General
sim-time-limit = 3600s

**.udpAppType = "FloodingNode"
**.defaultTTL = 1
**.udpApp[0].requestIntervalMean = 600s
**.udpApp[0].requestIntervalVariance = 60

[Config Flood2]
extends = Flooding
**defaultTTL = 2

[Config Flood3]
extends = Flooding
**defaultTTL = 3

[Config Flood4]
extends = Flooding
**defaultTTL = 4

[Config Flood5]
extends = Flooding
**defaultTTL = 5

[Config Flood6]
extends = Flooding
**defaultTTL = 6

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

def ini_conf_net(nodes):
    ret = []
    for node in nodes.keys():
        ret.append("**.host"+node+".udpApp[0].nodeID = \"host"+node+"\"")
        peers=[]
        for peer in nodes[node]:
            peers.append("host"+peer+":"+str(default_port))
        ret.append("**.host"+node+".udpApp[0].destinations = \""+(" ".join(peers))+"\"")
    return ret

def ini_conf_hosts(nodes):
    ret = []
    for node in nodes.keys():
        targets=[]
        for x in range(0,targetCount):
            targets.append("host"+random.choice(nodes.keys()))
        ret.append("**.host"+node+".udpApp[0].requestTargets = \""+(" ".join(targets))+"\"")
    return ret




if(targetCount==0):
	nedfile = open(netname+".ned","w")
	nedfile.write(tpl_nedfile % (netname,"\n".join(ned_make_hosts(nodes)),"\n".join(ned_make_router_connections(nodes))))
	nedfile.close()
	ininetfile = open(netname+".ini","w")
	ininetfile.write(tpl_ininetfile % (netname,default_port,targetCount,targetCount,"\n".join(ini_conf_net(nodes))))
	ininetfile.close()
else:
	inifile = open(netname+"-"+str(targetCount)+".ini","w")
	inifile.write(tpl_inifile % ("\n".join(ini_conf_hosts(nodes)),netname+".ini"))
	inifile.close()

