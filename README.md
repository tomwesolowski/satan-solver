<h2>SATan Solver</h2>

![alt tag](http://cs1924.vk.me/u31123286/a_ad325910.jpg)

<h3> The goal </h3>

Our goal is to create fast (as hell!) and still simple SAT solver that could bravely compete with modern state-of-the-art solvers. This is the first time we create our own SAT solver - we want to learn from the others, benefit from the experiences of our <s>older</s> more experienced friends and build up our project on them. 

For this reason, the primary version of our solver is mainly based on MINISAT and its implementation (comprehensively described here: http://www.decision-procedures.org/handouts/MiniSat.pdf ). We hope our project will outperform its ancestor very soon.

We would like to thank Dr Michał Wrona for inspiring us to create this project. The project is a part of 'SAT Solvers' course conducted by Jagiellonian University.

<h3> What has been done? </h3>

+ DIMACS parser (reading input) - our solver can communicate with the world!
+ Variable, clauses management - simple, but powerful! (for now)
+ Backtracking (with backjumping) - when everything goes wrong, sometimes the best option is just to give up or give up-up or give up-up-up
+ VDIDS heuristic for variable decision and its new version based on paper: https://ece.uwaterloo.ca/~vganesh/Publications_files/vg2015-UnderstandingVSIDS-HVC15.pdf
+ Unit propagation in linear time - using two-watched literals with custom little tweaks
+ Inferred/learnt clauses mechanism - our solver can learn (smart beast!) from its mistake to avoid similar ones in the future by analyzing implication graph and finding causes of encountered conflicts
+ Simplifying clauses - when we're sure how literal has to be assigned, we don't want process it over and over again
+ Smart restarts policy
+ Verification of returned model - just for testing purposes

<h3> More to come </h3>

+ Smarter restarts policy
+ Better management of learnt clauses
+ Deriving and solving subproblems with fast, polynomial algorithms (for 2-SAT, Horn-SAT etc.)
+ Modularity - we want our solver to be flexible and consist of disjoint and easy-to-swap modules for different heuristics. Furthermore, solver should be able to evaluate efficiency of heuristics and pick the best one for specific instance.
+ Efficient use of cache
+ Much more heuristics coming from papers and our minds
+ Many small programming tweaks that improve performance
+ Analyzing MINSAT 2.2 and GLUCOSE solvers and pilfering good ideas
+ Experimenting, experimenting, experimenting...

We test efficiency our solver with benchmarks provided by:
http://www.cs.ubc.ca/~hoos/SATLIB/benchm.html

At present time [30.04.16], SATan solver outperforms GRASP on big part of benchmarks and CHAFF on a few (and counting!).
