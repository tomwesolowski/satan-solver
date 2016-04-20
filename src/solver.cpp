#include <bits/stdc++.h>
#include "constants.h"
#include "literal.h"
#include "helpers.h"
#include "clause.h"
#include "solver.h"

using namespace std;

shared_ptr<Clause> NullClause = nullptr;

void Solver::InitVars(int num_vars) { 
	vars_.resize(num_vars, kUndefined);
	for(int i = 0; i < (int)vars_.size(); i++) {
		free_vars_.push_back(i);
	}
	watchers_.resize(num_vars);
	var_level_.resize(num_vars);
	in_prop_queue_.resize(num_vars, 0);
}

void Solver::AddClause(vector<Literal> lits) { 
	RefClause clause = Clause::Create(this, lits);

	clauses_.push_back(clause); 
	//unset_clauses_.push_back(clauses_.back()); 

	sort(lits.begin(), lits.end());
	lits.resize(distance(lits.begin(), unique(lits.begin(), lits.end())));

	int last_var = -1;
	for(Literal& lit : lits) {
		if(last_var == lit.var()) return; // <xi and -xi - always satisfiable>
		last_var = lit.var();
	}

	if(lits.size() <= 1) {
		AddToPropagateQueue(lits[0]);
	}

	Assert(UpdateWatchers(clause, true), "Could not intialize watchers");
}

void Solver::Decide() {
	// get random free variable
	Assert(GetNumFree(), "No free vars to decide.");

	std::uniform_int_distribution<int> randFree(0, GetNumFree()-1);
	std::uniform_int_distribution<int> randVal(0, 1);
	int var_free = randFree(generator_);
	
	Literal next(free_vars_[var_free],
	  					(randVal(generator_) ? kPositive : kNegative));

	decision_levels_.push_back(next.var());
	AddToPropagateQueue(next);

	Cerr << "Decide " << next.var()+1 << 
					" val: " << next.sign() << 
					"free: " << GetNumFree() << endl;
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

  	vector<RefClause> temp_watchers = watchers_[lit.var()];
  	watchers_[lit.var()].clear();

  	SetVar(lit.var(), lit.sign());
  	Cerr << "Free: " << GetNumFree() << endl;
  	for(RefClause clause : temp_watchers) {

  		if(!UpdateWatchers(clause)) {
  			if(IsClauseUnsatisified(clause)) {
    			Cerr << "conflict!" << endl;
    			return clause;
    		}	

    		if(IsUnitClause(clause)) {
    			Cerr << "unit!" << endl;
    			Literal unit = GetUnitLiteral(clause);
    			if(!AddToPropagateQueue(unit)) {
    				Cerr << "and conflict" << endl;
    				return clause;
    			}
    		}
  		}
  		temp_watchers.pop_back();
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

  	Cerr << "backtrack " << var+1 << ": " << GetVarValue(var) << endl;

  	int flipped_value = FlipValue(GetVarValue(var));
  	UnsetVar(var);

  	if(var == decision_levels_.back()) {
  		if((int)decision_levels_.size() <= level) {
    		if(both_ways <= 0) {
    			Assert(trail_.back().first == var, "Trail != var when flipping");
    			AddToPropagateQueue(Literal(var, flipped_value));
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

bool Solver::Verify() {
	for(RefClause clause : clauses_) {
		bool satisfied = false;
		for(Literal& lit : clause->lits_) {
  		if(GetLitValue(lit) == kPositive) {
  			satisfied = true;
  		}
  		Assert(GetLitValue(lit) != kUndefined, "Variable has not been assigned.");
  	}
  	if(!satisfied) {
  		return false;
  	}
	}
	return true;
}

bool Solver::Solve() {
	state_ = (Search() ? kSatisfiable : kUnsatisfiable);
  return IsSat();
}

bool Solver::UpdateWatchers(RefClause clause, bool forceAdd) {
	// false, when unit or unsatisfied
	Assert(clause->lits_.size() >= 2, "Lits < 2");
	Cerr << clause->lits_[0].var()+1 << " " << clause->lits_[1].var()+1 << 
	" " << watchers_.size() << " " << endl;
	for(int i = 0; i < 2; i++) {
		int status = clause->FindWatcher(this, i);
		if(status != 0 || forceAdd) {
			watchers_[clause->lits_[i].var()].push_back(clause);
		}
		if(status < 0) {
			return false;
		}
		
	}
	return true;
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
	Cerr << var+1 << " val: " << val << " " << "lvl: " << decision_levels_.size() << endl;
}

void Solver::UnsetVar(int var) { 
	if(vars_[var] != kUndefined) {
		free_vars_.push_back(var);
	}
	vars_[var] = kUndefined; 
	var_level_[var] = -1;
}

void Solver::RemoveFree(int var) {
	for(int i = 0; i < (int)free_vars_.size(); i++) {
		if(free_vars_[i] == var) {
			swap(free_vars_[i], free_vars_.back());
			free_vars_.pop_back();		
			return;
		}
	}
	Assert(false, "Variable was not free");
}

int Solver::GetNumFree() {
	return free_vars_.size();
}

bool Solver::IsUnitClause(RefClause clause) { 
	int free_vars = 0;
	for(Literal& lit : clause->lits_) {
		free_vars += IsVarUndefined(lit.var());
		if(IsLitPositive(lit)) return false;
	}
	return (free_vars == 1);
}

Literal Solver::GetUnitLiteral(RefClause clause) { 
	for(Literal& lit : clause->lits_) {
		if(IsVarUndefined(lit.var())) {
			return lit;
		}
	}
	Assert(false, "Unit literal not found.");
	assert(false); // just to get rid of warning
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

void Solver::SetSat() { state_ = kSatisfiable; }

void Solver::SetUnsat() { state_ = kUnsatisfiable; }

int Solver::IsSat() { return state_ == kSatisfiable; }

int Solver::IsUnsat() { return state_ == kUnsatisfiable; }

int Solver::IsUnsolved() { return state_ == kUndefined; }

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