CFSS: a Branch and Bound algorithm for GCCF
===================
CFSS is a Branch and Bound algorithm for Graph-Constrained Coalition Formation (GCCF). CFSS has been presented by Filippo Bistaffa, Alessandro Farinelli, Jesús Cerquides, Juan Rodríguez-Aguilar, and Sarvapali D. Ramchurn in “[Anytime Coalition Structure Generation on Synergy Graphs](http://www.aamas-conference.org/Proceedings/aamas2014/aamas/p13.pdf)”, Proceedings of the 13th International Conference on Autonomous Agents and Multiagent Systems (AAMAS), pages 13−20, 2014, IFAAMAS.

Download
----------
CFSS requires `g++` to compile, and does not require any external library to execute. In order to employ Twitter as network topology, `java` must be installed on the system, and the [Twitter GitHub repository](https://github.com/filippobistaffa/twitter) must be `git clone`'d inside CFSS's root directory.

Execution
----------
CFSS must be executed by means of the [`cfss.sh`](cfss.sh) script, i.e.,
```
./cfss.sh -t <scalefree|twitter> -n <#agents> -s <seed> [-m <barabasi_m>]

-t	Network topology (either scalefree or twitter)
-n	Number of agents
-s	Seed
-m	Parameter m of the Barabasi-Albert model (optional, default m = 1)
```

Collective energy purchasing dataset
----------
The dataset with the energy profiles used in the above paper is not publicly available. Instead, we provide a sample [`data.csv`](data.csv) file with 10 profiles to show the format of the input data. Specifically, each profile is specified in one line, and it refers to one day of energy consumption, with a measure every 30 minutes (i.e., each line contains 48 measures).
