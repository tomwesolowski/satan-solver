#ifndef VAR_DATABASE_H
#define VAR_DATABASE_H

#include <bits/stdc++.h>

class Solver;

class VarDatabase {
public:
	int num_vars_ = 0;
	int num_free_ = 0;

	double kDecayFactor = 0.9;
  const double kMaxActivity = 1e8;
  const double kRescaleFactor = 0.0001;

  queue<double> diffs_queue_;
  double sum_diffs_ = 0;
  double avg_diffs_ = 0;

	set< pair<double, int> > free_vars_; // <activeness, var>
  vector<int> activity_;
  vector<int> is_free_;

  void Init(Solver* solver);

  void AddToFreeVars(Solver* solver, int var);

  void PrepareFreeVars(Solver* solver);

  void RemoveFree(Solver* solver, int var);

  void BumpActivity(Solver* solver, int var, int value = 1);

  bool IsVarFree(Solver* solver, int var);

  int GetNumFree(Solver* solver);

  void DecayActivities(Solver* solver);

  void AdjustDecay(Solver* solver, double diff_level);

 	Literal GetNext(Solver* solver);
};

#endif