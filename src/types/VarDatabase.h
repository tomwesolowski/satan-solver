#ifndef VAR_DATABASE_H
#define VAR_DATABASE_H

#include <bits/stdc++.h>

#include "Literal.h"
#include "Parameters.h"

// Class responsible for picking the best next variable to assign.
// Contains lots of parameters that keep track of "activity" of
// every variable.
class VarDatabase {
public:
  VarDatabase(SolverParameters params);

  void Init(int num_vars);

  void AddToFreeVars(int var);

  void RemoveFree(int var);

  void BumpActivity(int var, int value = 1);

  void BumpActivity(RefClause clause, int value = 1);

  bool IsVarFree(int var);

  int GetNumFree();

  void ChangeAppearance(Literal lit, int val = 1);

  void ChangeAppearance(RefClause clause, int val = 1);

  void DecayAppearances();

  void DecayActivities();

  void AdjustDecay(int diff_levels);

  int GetVarCandidate();

  Literal GetNext(
      int free_var, int num_watchers_pos, int num_watchers_neg);

 private:
  // Input parameters.
  double decay_factor_;
  double rescale_factor_;
  double max_activity_;

  int max_diff_queue_size_;
  int max_avg_diff_;
  
  double decay_factor_slow_;
  double decay_factor_fast_;

	int num_vars_ = 0;
	int num_free_ = 0;

  queue<int> diffs_queue_;
  int sum_diffs_ = 0;
  double avg_diffs_ = 0;

  // Variables properties.
	set< pair<int, int> > free_vars_; // <activeness, var>
  vector<int> activity_;
  vector<int> is_free_;
  vector<int> apps_;
};

#endif