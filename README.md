CFSS: a Branch and Bound algorithm for GCCF
===================
CFSS is a Branch and Bound algorithm for Graph-Constrained Coalition Formation (GCCF). CFSS has been presented by Filippo Bistaffa, Alessandro Farinelli, Jesús Cerquides, Juan Rodríguez-Aguilar, and Sarvapali D. Ramchurn in “[Anytime Coalition Structure Generation on Synergy Graphs](http://www.aamas-conference.org/Proceedings/aamas2014/aamas/p13.pdf)”, Proceedings of the 13th International Conference on Autonomous Agents and Multiagent Systems (AAMAS), pages 13−20, 2014, IFAAMAS.

This branch contains the CFSS implementation that solves GCCF problems involving the <i>edge-sum with coordination cost</i> characteristic function.

Download
----------
CFSS requires `g++` to compile, and does not require any external library to execute. In order to employ Twitter as network topology, `java` must be installed on the system, and the [Twitter GitHub repository](https://github.com/filippobistaffa/twitter) must be `git clone`'d inside CFSS's root directory.

Execution
----------
CFSS must be executed by means of the [`cfss.sh`](https://github.com/filippobistaffa/CFSS/blob/master/cfss.sh) script, i.e.,
```
./cfss.sh -t <scalefree|twitter|filename> [-n <#agents>] [-s <seed>] [-m <barabasi_m>]

-t	Network topology (either scalefree, twitter, or the input filename)
-n	Number of agents (optional, default n = 10)
-s	Seed (optional, default s = 0)
-m	Parameter m of the Barabasi-Albert model (optional, default m = 2)
```

Input File Format
----------
The weighted graph <i>G = (V, E)</i> can be specified as a file via the `-t` parameter. Such file encodes the information about <i>G</i> according to the following format.

  * <i>n</i> lines containing the weights of the <i>n</i> auto-edges (i.e., one for each agent) in <i>G</i>. Auto-edges allow to have singletons with non-zero values and they are optional (i.e., their weights can be 0).
  * <i>e</i> lines containing, for each of the <i>e</i> edges, the indices of the incident vertices followed by the weight of the edge.
