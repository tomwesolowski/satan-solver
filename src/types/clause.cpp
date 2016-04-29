#include <bits/stdc++.h>

using namespace std;

#include "constants.h"
#include "helpers.h"
#include "literal.h"
#include "clause.h"
#include "solver.h"

shared_ptr<Clause> Clause::Create(Solver* solver, vector<Literal>& lits) {
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

	///FOR DEBUG
	for(int j = 2; j < lits_.size(); j++) {
		if(solver->GetLitValue(lits_[j]) == kUndefined && 
			 solver->GetLitValue(lits_[1]) == kNegative) {
				//Assert(false, "Undefined value among nonwatcher while second watcher = Negative");
		}
	}
	for(int j = 2; j < lits_.size(); j++) {
    if(solver->GetLitValue(lits_[j]) != kNegative) {
    	swap(lits_[0], lits_[j]);
      return kNewWatcher | kNormalClause;
    }
  }

  return kHoldWatcher | kUnitClause;

  /*if(solver->GetLitValue(lits_[0]) == kPositive) {
  	return kNewWatcher | kNormalClause;
  }

  if(solver->GetLitValue(lits_[0]) == kNegative
  	&& solver->GetLitValue(lits_[1]) == kNegative) {
  	return kHoldWatcher | kUnsatisfiableClause;	
  }

  if(found == 1 && solver->GetLitValue(lits_[1]) == kNegative) {
  	return kNewWatcher | kUnitClause;
  }
  if(found == 0 && solver->GetLitValue(lits_[1]) == kUndefined) {
  	return kHoldWatcher | kUnitClause;
  }*/

  //Cerr << solver->GetLitValue(lits_[0]) << " " << solver->GetLitValue(lits_[1]) << endl;

	//return (free_vars == 1 ? kUnitClause : kUnsatisfiableClause);
}