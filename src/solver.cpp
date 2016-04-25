#include <bits/stdc++.h>
#include "constants.h"
#include "literal.h"
#include "helpers.h"
#include "clause.h"
#include "solver.h"

using namespace std;

shared_ptr<Clause> NullClause = nullptr;

void Solver::AddToFreeVars(int var) {
	//Cerr << "add " << var+1 << ":" << activeness_[var] << endl;
	//activeness_[var] = rand(); //DEBUG
	Assert(free_vars_.count({activeness_[var], var}) == 0, "AddToFreeVars: Trying to add to free vars again");
	free_vars_.insert({activeness_[var], var});
}

void Solver::PrepareFreeVars() {
	for(int i = 0; i < vars_.size(); i++) {
		AddToFreeVars(i);
	}
}


void Solver::InitVars(int num_vars) { 
	vars_.resize(num_vars, kUndefined);
	watchers_.resize(num_vars);
	var_level_.resize(num_vars);
	in_prop_queue_.resize(num_vars, kUndefined);
	activeness_.resize(num_vars, 0);
}

void Solver::AddClause(vector<Literal> lits) { 

	sort(lits.begin(), lits.end());
	lits.resize(distance(lits.begin(), unique(lits.begin(), lits.end())));

	RefClause clause = Clause::Create(this, lits);

	//unset_clauses_.push_back(clauses_.back()); 

	int last_var = -1;
	for(Literal& lit : lits) {
		if(last_var == lit.var()) return; // <xi and -xi - always satisfiable>
		last_var = lit.var();
	}

	for(Literal& lit : lits) {
		activeness_[lit.var()]++;
	}

	assert(lits.size() > 0);

	if(lits.size() <= 1) {
		AddToPropagateQueue(lits[0]);
	}
	else {
		clauses_.push_back(clause); 
		Assert(InitWatchers(clause) != kUnsatisfiableClause, "AddClause: Could not intialize watchers");	
	}
	
}

void Solver::Decide() {
	// get random free variable
	Assert(GetNumFree(), "No free vars to decide.");

	//std::uniform_int_distribution<int> randFree(0, GetNumFree()-1);
	//std::uniform_int_distribution<int> randVal(0, 1);
	int var_free = free_vars_.rbegin()->second;
	int value = 1;

	if(watchers_[var_free].size()) {
		RefClause clause = watchers_[var_free][0];
		int idlit = (clause->lits_[1].var() == var_free);
		Assert((clause->lits_[idlit].var() == var_free), 
			"Decide: Picked var watch error");
		value = clause->lits_[idlit].sign();
	}

	Literal next(var_free, value);

	decision_levels_.push_back(next.var());
	AddToPropagateQueue(next);

	Cerr << "Decide " << next.var()+1 << 
					" val: " << next.sign() << 
					"free: " << GetNumFree() << endl;

	int sum_watchers = 0;
	for(int i = 0; i < watchers_.size(); i++) sum_watchers += watchers_[i].size();
	//cerr << "sum: " << sum_watchers << endl;
}

RefClause Solver::Propagate() {
  while (prop_queue_.size()) {
  	Literal lit = RemoveFromPropagateQueue();
  	Cerr << "Prop " << lit.var()+1 << " val: " << GetVarValue(lit.var()) << endl;
  	Cerr << "watching: " << watchers_[lit.var()].size() << endl;
  	if(trail_.size() && trail_.back().first == lit.var()) {
  		trail_.back().second = 1;
  	}
  	else {
  		AddToTrail(lit.var());
  	}

  	vector<RefClause> watchers = watchers_[lit.var()];
  	watchers_[lit.var()].clear();
  	RefClause conflict = nullptr;
  	//watchers_[lit.var()].clear();

  	assert(trail_.back().second || GetVarValue(lit.var()) == kUndefined);
  	SetVar(lit.var(), lit.sign());
  	Cerr << "Free: " << GetNumFree() << endl;
  	Cerr << "Set: " << lit.var() << " to " << lit.sign() << endl;
  	Cerr << "Lit value: " << GetLitValue(lit) << endl;
  	while(watchers.size()) {

  		RefClause clause = watchers.back();
  		watchers.pop_back();

  		int status = UpdateWatcher(clause, lit);

  		Cerr << "status:" << status << " unit:" << IsUnitClause(clause) << endl;
  		Assert( (status == kUnitClause && IsUnitClause(clause)) || 
  			      (status != kUnitClause && !IsUnitClause(clause)), 
  			      "(Not)Unitary clause status is wrong!");

			if(status == kUnsatisfiableClause) {
  			Cerr << "conflict!" << endl;
  			conflict = clause;
  			break;
  		}

  		if(status == kUnitClause) {
  			Cerr << "unit!" << endl;
  			Literal unit = GetUnitLiteral(clause);
  			if(!AddToPropagateQueue(unit)) {
  				Cerr << "and conflict" << endl;
  				conflict = clause;
  				break;
  			}
  		}

  	}

  	if(conflict != nullptr) {
	  	//add remaining watching clauses
	  	for(RefClause clause: watchers) {
				watchers_[lit.var()].push_back(clause);
			}
	  	return conflict;
	  }

  }

  return NullClause;
}

//level - the latest level to NOT be erased
bool Solver::Backtrack(int level) {
  // todo

  while(prop_queue_.size()) RemoveFromPropagateQueue();

  Cerr << "Backtrack lvl: " << level << endl;

  if(level <= 0) return false;

  while(true) {

  	Assert(trail_.size(), "Trying to backtrack on top-level");
  	int var = trail_.back().first;
  	int both_ways = trail_.back().second;

  	Cerr << "backtrack " << var+1 << ": " << GetVarValue(var) << "both: " << both_ways << endl;
  	Cerr << "level " << decision_levels_.size() << endl;
  	if(decision_levels_.size()) Cerr << "father level " << decision_levels_.back() << endl;

  	int flipped_value = FlipValue(GetVarValue(var));
  	UnsetVar(var);

  	if(var == decision_levels_.back()) {
  		if((int)decision_levels_.size() <= level) {
    		if(both_ways <= 0) {
    			Assert(trail_.back().first == var, "Trail != var when flipping");
    			Assert(AddToPropagateQueue(Literal(var, flipped_value)), "flipping value must be correct");
    			break;
    		}
    	}
    	decision_levels_.pop_back();
    }

  	trail_.pop_back();

		if(trail_.size() <= 0) {
			return false;
		}
  }

  Assert(trail_.size(), "Backtrack left nothing assigned");
  return true;
}

int Solver::Analyze(RefClause conflict) {
  // returns the most recent level causing problem
  int max_level = -1;
	for(Literal& lit : conflict->lits_) {
		max_level = max(max_level, var_level_[lit.var()]);
	}
	Cerr << "analyze " << max_level << endl;
	return max_level;
}

bool Solver::Search() {
  while (IsUnsolved()) {
    RefClause conflict = Propagate();
    if (conflict == NullClause) {
      if(!GetNumFree()) {
      	return true;
      }
      Decide();
    } else {
      // conflict!
      int level = Analyze(conflict);
      if(!Backtrack(level)) {
      	return false;
      }
    }
  }
  Assert(false, "Solved instance has not been caught.");
  return true;
}

bool Solver::AddToPropagateQueue(Literal lit) {
	if(in_prop_queue_[lit.var()] != kUndefined) {
		if(in_prop_queue_[lit.var()] != lit.sign()) {
			return false;
		}
		return true;
	}
	in_prop_queue_[lit.var()] = lit.sign();
	prop_queue_.push(lit);
	return true;
}

Literal Solver::RemoveFromPropagateQueue() {
	Assert(prop_queue_.size(), "Trying to pop from empty prop queue.");
	Literal lit = prop_queue_.front();
	prop_queue_.pop();
	in_prop_queue_[lit.var()] = kUndefined;
	return lit;
}

void Solver::AddToTrail(int var) {
	trail_.push_back({var, 0});
}

bool Solver::Verify(bool solvable) {
	if(IsUnsat()) {
		Assert(!solvable, "Assignment not found to solvable instance");
		return true;
	}
	for(RefClause clause : clauses_) {
		bool satisfied = false;
		for(Literal& lit : clause->lits_) {
  		if(GetLitValue(lit) == kPositive) {
  			satisfied = true;
  		}
  		Assert(GetLitValue(lit) != kUndefined, "Variable has not been assigned.");
  	}
  	Assert(satisfied, "Clause is not satisfied");
	}
	return true;
}

bool Solver::Solve() {
	PrepareFreeVars();
	state_ = (Search() ? kSatisfiableState : kUnsatisfiableState);
  return IsSat();
}

int Solver::InitWatchers(RefClause clause) {
	// false, when unit or unsatisfied
	Assert(clause->lits_.size() >= 2, "InitWatchers: Clause has less than 2 literals");

	for(int i = 0; i < 2; i++) {
		//Assert(clause->FindWatcher(this) == kNormalClause, 	"InitWatchers: Clause is unit or unsat.");
		watchers_[clause->lits_[i].var()].push_back(clause);
		//swap(clause->lits_[0], clause->lits_[1]);
	}

	return kHoldWatcher;
}

int Solver::UpdateWatcher(RefClause clause, Literal lit) {
	// false, when unit or unsatisfied
	//Cerr << clause->lits_[0].var()+1 << " " << clause->lits_[1].var()+1 << 
	//" " << clause->lits_.size() << " lit: " << GetLitValue(lit) << endl;

	//temporrary check only var, because of flipped values
	if(lit.var() == clause->lits_[1].var()) {
		swap(clause->lits_[0], clause->lits_[1]);
	}

	Assert(GetLitValue(lit) != kUndefined, "UpdateWatcher: updating when lit = undefined");

	Assert(lit.var() == clause->lits_[0].var(), 
		"UpdateWatcher: Old watcher != lits[0]");

	int lit_value = GetLitValue(clause->lits_[0]);

	//cerr << lit.var()+1 << " " << frozen_watchers_[lit.var()].size() << endl;

	int status = clause->FindWatcher(this);
	if(status == kReleaseWatcher) {
		watchers_[clause->lits_[0].var()].push_back(clause);
	}
	else {
		watchers_[lit.var()].push_back(clause);
	}

	return status;
}

bool Solver::Enqueue(Literal lit) {
	return true;
}

int Solver::FlipValue(int value) {
	Assert(value != kUndefined, "Trying to flip undefined value");
	return (value == kPositive ? kNegative : kPositive);
}

void Solver::FlipVar(int var) {
	SetVar(var, FlipValue(GetVarValue(var)));
}

void Solver::SetVar(int var, int val) { 
	if(vars_[var] == kUndefined) {
		RemoveFree(var);
	}
	vars_[var] = val; 
	var_level_[var] = decision_levels_.size();
	//Cerr << var+1 << " val: " << val << " " << "lvl: " << decision_levels_.size() << endl;
}

void Solver::UnsetVar(int var) { 
	if(vars_[var] != kUndefined) {
		AddToFreeVars(var);
	}
	vars_[var] = kUndefined; 
	var_level_[var] = -1;
}

void Solver::RemoveFree(int var) {
	//Cerr << "rem " << var << ":" << activeness_[var] << endl;
	Assert(free_vars_.erase({activeness_[var], var}), "Variable was not free");
}

int Solver::GetNumFree() {
	return free_vars_.size();
}

bool Solver::IsUnitClause(RefClause clause) { 
	int free_vars = 0;
	int i = 0;
	for(Literal& lit : clause->lits_) {
		if(IsVarUndefined(lit.var())) {
			free_vars++;
			Cerr << "free var at " << i << endl;
		}
		if(IsLitPositive(lit)) {
			return false;
		}
		i++;
	}
	return (free_vars == 1);
}

Literal Solver::GetUnitLiteral(RefClause clause) { 
	Assert(GetLitValue(clause->lits_[1]) == kUndefined, "Unit literal not found.");
	return clause->lits_[1];
} 

bool Solver::IsClauseSatisified(RefClause clause) { 
	for(Literal& lit : clause->lits_) {
		if(GetLitValue(lit) == kPositive) {
			return true;
		}
	}
	return false;
}

bool Solver::IsClauseUnsatisified(RefClause clause) { 
	for(Literal& lit : clause->lits_) {
		if(GetLitValue(lit) != kNegative) {
			return false;
		}
	}
	return true;
}

int Solver::GetVarValue(int var) { 
	return vars_[var]; 
}

int Solver::GetLitValue(Literal& lit) { 
	return GetVarValue(lit.var()) * lit.sign(); 
}

bool Solver::IsLitPositive(Literal& lit) { return GetLitValue(lit) == kPositive; }

bool Solver::IsLitNegative(Literal& lit) { return GetLitValue(lit) == kNegative; }

bool Solver::IsVarPositive(int var) { return GetVarValue(var) == kPositive; }

bool Solver::IsVarNegative(int var) { return GetVarValue(var) == kNegative; }

bool Solver::IsVarUndefined(int var) { return GetVarValue(var) == kUndefined; }

void Solver::SetSat() { state_ = kSatisfiableState; }

void Solver::SetUnsat() { state_ = kUnsatisfiableState; }

int Solver::IsSat() { return state_ == kSatisfiableState; }

int Solver::IsUnsat() { return state_ == kUnsatisfiableState; }

int Solver::IsUnsolved() { return state_ == kUnknownState; }

void Solver::Print() {
	assert(!IsUnsolved());
  if (IsUnsat()) {
    cout << "UNSAT" << endl;
    return;
  }
  Assert(Verify(), "Assignment is not correct.");
  cout << "SAT" << endl;
  for (int i = 0; i < (int)vars_.size(); i++) {
    cout << vars_[i] * (i+1) << " ";
  }
  cout << endl;
}