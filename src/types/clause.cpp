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

int Clause::FindWatcher(Solver* solver, int i) {
	if(solver->GetLitValue(lits_[i]) == kUndefined) {
		return 0;
	}

  if(lits_.size() <= 2) {
    return -1;
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

	for(int j = 2; j < lits_.size(); j++) {
    if(solver->GetLitValue(lits_[j]) == kUndefined) {
      swap(lits_[i], lits_[j]);
      return 1;
    }
  }
	return -1;
}

bool Clause::FindWatchers(Solver* solver) {
	for(int i = 0; i < 2; i++) {
		if(!FindWatcher(solver, i)) return false;
	}
	return true;
}