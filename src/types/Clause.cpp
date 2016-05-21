#include <bits/stdc++.h>

using namespace std;

#include "Constants.h"
#include "Helpers.h"
#include "Literal.h"
#include "Clause.h"
#include "VarDatabase.h"
#include "Solver.h"

RefClause Clause::Create(Solver* solver, vector<Literal>& lits) {
	static int id = 0; 
	return make_shared<Clause>(lits, id++);
}

Clause::Clause(vector<Literal>& lits, int id) {
	id_ = id;
	lits_ = lits;
}

bool Clause::locked(Solver* solver) {
	return solver->reason_[lits_[1].var()].get() == this; 
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

int Clause::GetNumFreeLiterals(Solver* solver) {
	int fr = 0;
	for(Literal& lit : lits_) {
		fr += solver->IsVarUndefined(lit.var());
	}
	return fr;
}

double Clause::GetFulfillment(Solver* solver) {
	return (double)GetNumFreeLiterals(solver) / lits_.size();
}

int Clause::GetLBD(Solver* solver) {
	set<int> diff_levels;
	for(Literal& lit : lits_) {
		diff_levels.insert(solver->level_[lit.var()]);
	}
	return diff_levels.size();
}