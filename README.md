# simulRaft
A Stand-alone Simulation of RAFT Cluster with Qt C++

## Overview
All the servers are running in different threads, using a responsive mode so that you can running more servers(If you want the UI to work, no more than 50. Also you can modify the code to close the UI and observe the work flow through qDebug()'s report. The qDebug()'s report uses two macro switches defined in Settings.hh.) when consuming less resources. Servers can crash and recover.

The class Server is implemented basically as the Raft paper recommends, but given the simulation of network, class simulNet, cannot perfectly simulate the actual network, I made some subtle modifications. The Client interacts with the cluster to append entries and to attain the real-time information at a fixed interval (1000ms).

The client, cluster and the simulNet are running in the main thread. When simulNet, receives a broadcast request, it will start a timer to simulate the network delay, which conform to a normal distrbution with default 95% confidence interval set between 1ms and 20ms. Unfortunately, due to the single-thread property of simulNet and the precision of timer, the log replication procedure may get stuck after executing for a period of time, especially when the leader crashes. When a timer timeout, it triggers an event, which can finish the delay of only one package according with the timerID and send it. So if several packages are due to be sent at very close moment or even the same time, only one package can be delivered as scheduled, and the others' delivery would be put off, even permanently when the packages are in a large amount.

Above all, this is not a perfect stand-alone simulation of Raft by a noob sophomore programmer, mostly due to the design of the simulNet (or other bugs I'm not aware of). If you have a better solution or find some bugs, please commit your issue or email me at rossil@sjtu.edu.cn.
