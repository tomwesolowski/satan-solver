#ifndef VAR_DATABASE_H
#define VAR_DATABASE_H

#include <bits/stdc++.h>

class Solver;

class VarDatabase {
public:
	int num_vars_;

	set< pair<int, int> > free_vars_; // <activeness, var>
  vector<int> activity_; // number of clauses with literal i

  void Init(Solver* solver);

  void AddToFreeVars(Solver* solver, int var);

  void PrepareFreeVars(Solver* solver);

  void RemoveFree(Solver* solver, int var);

  void BumpActivity(Solver* solver, int var, int value = 1);

  bool IsVarFree(Solver* solver, int var);

 	Literal GetNext(Solver* solver);
};

#endif