#include <bits/stdc++.h>

using namespace std;

#include "constants.h"
#include "helpers.h"
#include "literal.h"
#include "clause.h"
#include "solver.h"

shared_ptr<Clause> Clause::Create(Solver* solver, vector<Literal>& lits) { 
	return make_shared<Clause>(lits);
}

Clause::Clause(vector<Literal>& lits) {
	lits_ = lits;
}

// new watcher always at index 0
int Clause::FindWatcher(Solver* solver) {

	Assert(solver->GetLitValue(lits_[0]) != kUndefined, "First literal is not assigned!");

	int free_vars = 0;

	if(solver->GetLitValue(lits_[0]) == kPositive || 
		solver->GetLitValue(lits_[1]) == kPositive) {
		return kHoldWatcher;
	}

	for(int j = 2; j < lits_.size(); j++) {
		if(solver->GetLitValue(lits_[j]) == kUndefined && 
			 solver->GetLitValue(lits_[1]) == kNegative) {
				Assert(false, "Undefined value among nonwatcher while second watcher = Negative");
		}
	}

	/*
	auto& watchers = solver->watchers_[lits[i].var()];
	bool found = false;

	//removing old watcher
	for(int j = 0; j < watchers.size(); j++) {
		if(watchers[j].get() == this) {
			swap(watchers[j], watchers.back());
			watchers.pop_back();
			found = true;
			break;
		}
	}

	Assert(found, "Old watcher not found");
	*/

	/* only for debug, it should not be comment
	if(solver->GetLitValue(lits_[0]) == kNegative) {
  	return kUnsatisfiableClause;	
  }
  */

	bool found = false;

	for(int j = 2; j < lits_.size(); j++) {
    if(solver->GetLitValue(lits_[j]) != kNegative) {
      swap(lits_[0], lits_[j]);
      found = true;
      break;
    }
  }

  if(solver->GetLitValue(lits_[0]) == kNegative
  	&& solver->GetLitValue(lits_[1]) == kNegative) {

  	return kUnsatisfiableClause;	
  }

  if(!found) {
  	Assert(solver->GetLitValue(lits_[1]) == kUndefined, "Returns unit while there is no unit");
  	return kUnitClause;
  }

  return kReleaseWatcher;

  //Cerr << solver->GetLitValue(lits_[0]) << " " << solver->GetLitValue(lits_[1]) << endl;

	//return (free_vars == 1 ? kUnitClause : kUnsatisfiableClause);
}