//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package darknetsim;


import inet.nodes.inet.StandardHost;
import inet.nodes.inet.Router;
import inet.networklayer.autorouting.FlatNetworkConfigurator;
import ned.DatarateChannel;


network TestNetChain1
{
    @display("bgb=424,130");
    submodules:
        configurator: FlatNetworkConfigurator {
            @display("p=54,90;i=block/cogwheel");
            networkAddress = "23.42.1.0";
            netmask = "255.255.255.0";
        }
        router: Router {
            @display("p=170,90");
        }
        host0: StandardHost {
            @display("p=35,30");
        }
        host1: StandardHost {
            @display("p=125,30");
        }
        host2: StandardHost {
            @display("p=215,30");
        }
        host3: StandardHost {
            @display("p=300,30");
        }
        host4: StandardHost {
            @display("p=385,30");
        }
    connections:
        router.pppg++ <--> DatarateChannel <--> host0.pppg++;
        router.pppg++ <--> DatarateChannel <--> host1.pppg++;
        router.pppg++ <--> DatarateChannel <--> host2.pppg++;
        router.pppg++ <--> DatarateChannel <--> host3.pppg++;
        router.pppg++ <--> DatarateChannel <--> host4.pppg++;

}
