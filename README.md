CFSS: a Branch and Bound algorithm for Graph-Constrained Coalition Formation
===================
CFSS is a Branch and Bound algorithm for Graph-Constrained Coalition Formation. CFSS has been presented by Filippo Bistaffa, Alessandro Farinelli, Jesús Cerquides, Juan Rodríguez-Aguilar, and Sarvapali D. Ramchurn in “[Anytime Coalition Structure Generation on Synergy Graphs](http://www.aamas-conference.org/Proceedings/aamas2014/aamas/p13.pdf)”, Proceedings of the 13th International Conference on Autonomous Agents and Multiagent Systems (AAMAS), pages 13−20, 2014, IFAAMAS.

Download
----------
CFSS does not require any external library to compile and execute. In order to employ Twitter as network topology, `git clone` the [Twitter GitHub repository](https://github.com/filippobistaffa/twitter) inside CFSS's root directory.

Execution
----------
CFSS must be executed by means of the [`cfss.sh`](https://github.com/filippobistaffa/CFSS/blob/master/cfss.sh) script, i.e.,
```
./cfss.sh -t <scalefree|twitter> -n <#agents> -s <seed> [-m <barabasi_m>]

-t	Network topology (either scalefree or twitter)
-n	Number of agents
-s	Seed
-m	Parameter m of the Barabasi-Albert model (optional, default m = 1)
```
