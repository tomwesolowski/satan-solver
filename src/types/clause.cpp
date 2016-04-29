#include <bits/stdc++.h>

using namespace std;

#include "constants.h"
#include "helpers.h"
#include "literal.h"
#include "clause.h"
#include "solver.h"

RefClause Clause::Create(Solver* solver, vector<Literal>& lits) {
	static int id = 0; 
	return make_shared<Clause>(lits, id++);
}

Clause::Clause(vector<Literal>& lits, int id) {
	id_ = id;
	lits_ = lits;
}

// new watcher always at index 0
int Clause::FindWatcher(Solver* solver) {

	Assert(solver->GetLitValue(lits_[0]) != kUndefined, "First literal is not assigned!");

  assert(solver->GetLitValue(lits_[0]) == kNegative);

	if(solver->GetLitValue(lits_[1]) == kPositive) {
		return kHoldWatcher | kNormalClause;
	}

	for(int j = 2; j < lits_.size(); j++) {
    if(solver->GetLitValue(lits_[j]) != kNegative) {
      swap(lits_[2], lits_[j]);
    	swap(lits_[0], lits_[2]);
      return kNewWatcher | kNormalClause;
    }
  }

  return kHoldWatcher | kUnitClause;
}