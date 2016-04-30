#include <bits/stdc++.h>
#include "Constants.h"
#include "Literal.h"
#include "Helpers.h"
#include "Clause.h"
#include "Solver.h"

using namespace std;

shared_ptr<Clause> NullClause = nullptr;

void Solver::AddToFreeVars(int var) {
	//Cerr << "add " << var+1 << ":" << var_db_.activeness_[var] << endl;
	//var_db_.activeness_[var] = rand(); //FOR DEBUG
	Assert(!IsVarFree(var), "AddToFreeVars: Trying to add to free vars again");
	var_db_.free_vars_.insert({var_db_.activeness_[var], var});
}

void Solver::PrepareFreeVars() {
	for(int i = 0; i < vars_.size(); i++) {
		AddToFreeVars(i);
	}
}

void Solver::InitVars(int num_vars) { 
	vars_.resize(num_vars, kUndefined);
	reason_.resize(num_vars, nullptr);
	watchers_.resize(2*num_vars);
	level_.resize(num_vars);
	var_db_.activeness_.resize(num_vars, 0);
	PrepareFreeVars();
}

bool Solver::AddClause(vector<Literal> lits, bool learnt) { 

	if(!learnt) {
		sort(lits.begin(), lits.end());
		lits.resize(distance(lits.begin(), unique(lits.begin(), lits.end())));	
	
		int last_var = -1;
		for(Literal& lit : lits) {
			if(last_var == lit.var()) {
				return true; // <xi or -xi - always satisfiable>
			}
			last_var = lit.var();
		}
	}

		//TODO
	for(Literal& lit : lits) {
		if(IsVarFree(lit.var())) {
			RemoveFree(lit.var());
			var_db_.activeness_[lit.var()] += 50 - lits.size();
			AddToFreeVars(lit.var());
		}
		else {
			var_db_.activeness_[lit.var()] += 50 - lits.size();
		}
	}

	assert(lits.size() > 0);

	if(learnt) {
		//cerr << "Learnt size: " << lits.size() << endl;
		assert(lits.size() > 1 || CurrentDecisionLevel() == 0);
	}

	if(lits.size() <= 1) {
		if(!Enqueue(lits[0])) {
			return false;
		}
		return true;
	}

	if(learnt) {
		for(int i = 2; i < lits.size(); i++) {
			//Cerr << level_[lits[0].var()] << endl;
			Assert(level_[lits[0].var()] == -1, 
				"Lit[0] of learnt clause has to be the most recent.");
			Assert(level_[lits[i].var()] != -1, 
				"Lit[i] of learnt clause has to be assigned.");
			if(level_[lits[i].var()] > level_[lits[1].var()]) {
				swap(lits[1], lits[i]);
			}
		}
	}

	RefClause clause = Clause::Create(this, lits);
	clause->learnt = learnt;

	if(learnt) {
		Assert(Enqueue(lits[0], clause), "Enqueuing learn literal failed");
	}

	clauses_.push_back(clause); 
	for(int i = 0; i < 2; i++) {
		watchers_[(-lits[i]).index()].push_back(clause);
	}
	
	return true;
}

void Solver::Decide() {
	// get random free variable
	Assert(GetNumFree(), "No free vars to decide.");

	int var_free = var_db_.free_vars_.rbegin()->second;
	Literal lit_pos(var_free, kPositive);
	Literal lit_neg(var_free, kNegative);
	Literal lit;
	RefClause clause;

	if(watchers_[lit_neg.index()].size() > 
			watchers_[lit_pos.index()].size()) {
		clause = watchers_[lit_neg.index()][0];
		lit = lit_pos;
	}
	else if(watchers_[lit_pos.index()].size()) {
		clause = watchers_[lit_pos.index()][0];
		lit = lit_neg;
	}
	else {
		lit = Literal(var_free, kPositive);
	}

	//int idlit = (clause->lits_[1].var() == var_free);
	//Assert((clause->lits_[idlit].var() == var_free), 

	decision_levels_.push_back(trail_.size());
	Assert(Enqueue(lit), "Enqueue failed while deciding new var");

	Cerr << "Decide " << lit.var() << 
					" val: " << lit.sign() << 
					"free: " << GetNumFree() << endl;
}

RefClause Solver::Propagate() {
  while (prop_queue_.size()) {
  	Literal lit = RemoveFromPropagateQueue();
  	Cerr << "Prop " << lit.var() << " val: " << lit.sign() << " litval: " << GetLitValue(lit) << endl;
  	//Cerr << "watching: " << watchers_[lit.var()].size() << endl;
  	//Cerr << "Free: " << GetNumFree() << endl;

  	vector<RefClause> watchers = watchers_[lit.index()];
  	watchers_[lit.index()].clear();
  	RefClause conflict = nullptr;
  	Assert(GetVarValue(lit.var()) != kUndefined, 
  		"Propagated var is undefined");

  	while(watchers.size()) {

  		RefClause clause = watchers.back();
  		watchers.pop_back();

  		int status = UpdateWatcher(clause, -lit);

  		if(status == kUnitClause) {

  			Literal unit = GetUnitLiteral(clause);

  			if(!Enqueue(unit, clause)) {
  				Cerr << "and conflict " << unit.var() << " " << GetVarValue(unit.var()) << " " << unit.sign() << " " << clause->lits_.size() << endl;

  				conflict = clause;

  				//todo
  				for(Literal& lit : conflict->lits_) {
  					var_db_.activeness_[lit.var()]++;
  				}

  				break;
  			}
  		}

  	}

  	if(conflict != nullptr) {
	  	//add remaining watching clauses
	  	for(RefClause rem_clause: watchers) {
				watchers_[lit.index()].push_back(rem_clause);
			}
	  	return conflict;
	  }


  }

  return NullClause;
}

//level - the latest level to NOT be erased
bool Solver::Backtrack(int level) {
  // todo

  Cerr << "Backtrack lvl: " << level << endl;

  Cerr << decision_levels_.back() << " " << trail_.size() << endl;

  assert(level >= 0);

  while(CurrentDecisionLevel() > level) {
  	//Assert(trail_.size(), "Trying to backtrack on top-level");

  	while(decision_levels_.back() < trail_.size()) {
  		UnsetVar();
  	}
  	decision_levels_.pop_back();
  }

  return true;
}

int Solver::Analyze(RefClause conflict, vector<Literal>& learnt_clause) {
  // returns the most recent level causing problem
	vector<bool> seen(vars_.size(), 0);
	Literal p(-1, kNegative);
	vector<Literal> reason;
  int max_level = 0;
  int counter = 0;
  int currentLevel = CurrentDecisionLevel();

  for(Literal lit : conflict->lits_) {
			Cerr << lit.var() << ":" << lit.sign() << ":" << GetLitValue(lit) << ":" << level_[lit.var()] << " ";
		}
		Cerr << "@ " << CurrentDecisionLevel() << endl;

  do {
  	reason.clear();
  	//Cerr << ":" << p.var() << " counter: " << counter << " " << (p.var() != -1 ? level_[p.var()] : -1) << endl;
  	Assert(conflict != nullptr, "Conflict is nullptr");

  	assert(p.sign() != kUndefined);

		if(p.var() >= 0) {
			Cerr << p.var() << " " << p.sign() << endl;
			int found = 0;
			for(Literal& lit : conflict->lits_) {
				Cerr << ">" << lit.var() << " " << lit.sign() << endl;
				if(lit == p) found = 1;
			}
			Assert(found, "p literal not found in conflict");
		}
		for(Literal& lit : conflict->lits_) {
			if(lit.var() != p.var()) {
				Assert(GetLitValue(lit) == kNegative, "Reason literal is not negative");
				reason.push_back(lit);
			}
		}

  	for(Literal& lit : reason) {
  		Cerr << lit.var() << " " << lit.sign() << " " << (reason_[lit.var()] != nullptr) << "lvl: " << level_[lit.var()] << endl;
  		int var_level = level_[lit.var()];
  		//Assert(GetLitValue(lit) != kUndefined, "Reason lit is undefined");
  		if(!seen[lit.var()]) {
  			seen[lit.var()] = 1;
  			if(var_level == currentLevel) {
  				counter++;
  			}
  			else if(var_level > 0) {
  				learnt_clause.push_back(lit);
  				max_level = max(max_level, var_level);
  			}
  		}
  	}

  	do {
  		assert(trail_.size());
  		p = trail_.back();
  		conflict = reason_[p.var()];
  		UnsetVar();
  	}
  	while(!seen[p.var()]);
  	counter--;
  } while(counter > 0);

	Assert(currentLevel == CurrentDecisionLevel(), "Dec. level has changed");

	learnt_clause.push_back(-p);
	swap(learnt_clause.front(), learnt_clause.back());

	int fr = 0;
	for(Literal lit : learnt_clause) {
		fr += (GetLitValue(lit) == kUndefined);
	}
	Assert(fr == 1, "Learnt clause must contain exactly one undef. literal. has: " + to_string(fr));
	Assert(GetVarValue(learnt_clause[0].var()) == kUndefined, 
			"Learnt clause must contain lit[0] == Undefined");

	Cerr << "analyze " << max_level << endl;
	return max_level;
}

bool Solver::Search() {
  while (IsUnsolved()) {
  	//Cerr << clauses_.size() << endl;
    RefClause conflict = Propagate();
    if (conflict == NullClause) {
      if(!GetNumFree()) {
      	return true;
      }
      if(CurrentDecisionLevel() == 0) {
    		Simplify();
    	}
      Decide();
    } else {
    	if(CurrentDecisionLevel() <= 0) {
    		return false;
    	}
      // conflict!
		  while(prop_queue_.size()) {
		  	RemoveFromPropagateQueue();
		  }

      vector<Literal> learnt_clause;
      int level = Analyze(conflict, learnt_clause);
      Backtrack(level);
      assert(AddClause(learnt_clause, true));
    }
  }
  Assert(false, "Solved instance has not been caught.");
  return true;
}

bool Solver::Enqueue(Literal lit, RefClause from) {
	Assert(lit.sign() != kUndefined, "Enqueuing undefined var.");
	Cerr << "enqueue " << lit.var() << " " << lit.sign() << endl;
	if(vars_[lit.var()] != kUndefined) {
		if(vars_[lit.var()] != lit.sign()) {
			return false;
		}
		return true;
	}
	RemoveFree(lit.var());
	trail_.push_back(lit);
	vars_[lit.var()] = lit.sign(); 
	level_[lit.var()] = decision_levels_.size();
	reason_[lit.var()] = from;
	prop_queue_.push(lit);

	if(from != nullptr) {
		int found = 0;
		for(Literal x : from->lits_) {
			found += (x == lit);
		}
		Assert(found, "Literal not found in reason during enqueuing");
	}
	return true;
}

void Solver::Simplify() {
	for(int i = 0; i < clauses_.size(); i++) {
		RefClause& clause = clauses_[i];
		bool deleted = false;
		int k = 0;
		for(int j = 0; j < clause->lits_.size(); j++) {
			Literal& lit = clause->lits_[j];
			if(IsLitPositive(lit)) {
				//remove clauses as it's always satisfied
				swap(clauses_[i], clauses_.back());
				clauses_.pop_back();
				i--; deleted = true;
				break;
			}
			if(!IsLitNegative(lit)) {
				clause->lits_[k] = lit;
				k++;
			}
		}
		if(!deleted) {
			clause->lits_.resize(k);
			Assert(k, "Simplified clause is not satisfiable - conflict not found eariler");
		}
	}
}

void Solver::UnsetVar() { 
	int var = trail_.back().var();
	Assert(vars_[var] != kUndefined, "Un-assigning non-undefined var.");
	
	AddToFreeVars(var);
	vars_[var] = kUndefined; 
	reason_[var] = nullptr;
	level_[var] = -1;
	trail_.pop_back();
}


Literal Solver::RemoveFromPropagateQueue() {
	Assert(prop_queue_.size(), "Trying to pop from empty prop queue.");
	Literal lit = prop_queue_.front();
	prop_queue_.pop();
	return lit;
}

bool Solver::Verify(bool solvable) {
	if(IsUnsat()) {
		Assert(!solvable, "Assignment not found to solvable instance");
		return true;
	}
	Assert(solvable, "Assignment found but it's not solvable :D");
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
	state_ = (Search() ? kSatisfiableState : kUnsatisfiableState);
  return IsSat();
}

int Solver::UpdateWatcher(RefClause clause, Literal lit) {
	//temporrary check only var, because of flipped values
	if(lit == clause->lits_[1]) {
		swap(clause->lits_[0], clause->lits_[1]);
	}

	Assert(GetLitValue(lit) == kNegative, "UpdateWatcher: updating when lit != negative");

	Assert(lit == clause->lits_[0], 
		"UpdateWatcher: Old watcher != lits[0]");

	int status = clause->FindWatcher(this);

	if(status & kNewWatcher) {
		watchers_[(-clause->lits_[0]).index()].push_back(clause);
	}
	else if(status & kHoldWatcher) {
		watchers_[(-lit).index()].push_back(clause);
	}

	Assert((status & kNewWatcher) != (status & kHoldWatcher), 
		"Cannot hold and release watcher at the same time");

	Assert(__builtin_popcount((status & kUnitClause) + 
		(status & kNormalClause)) == 1, 
		"Status in incorrect");

	if(status & kUnitClause) {
		return kUnitClause;
	}
	return kNormalClause;
}

int Solver::FlipValue(int value) {
	Assert(value != kUndefined, "Trying to flip undefined value");
	return (value == kPositive ? kNegative : kPositive);
}

void Solver::RemoveFree(int var) {
	//Cerr << "rem " << var << ":" << var_db_.activeness_[var] << endl;
	Assert(IsVarFree(var), "Variable was not free");
	var_db_.free_vars_.erase({var_db_.activeness_[var], var});
}

int Solver::GetNumFree() {
	return var_db_.free_vars_.size();
}

// FOR DEBUG
bool Solver::IsUnitClause(RefClause clause) { 
	int free_vars = 0;
	int i = 0;
	for(Literal& lit : clause->lits_) {
		if(IsVarUndefined(lit.var())) {
			free_vars++;
			//Cerr << "free var at " << i << endl;
		}
		if(IsLitPositive(lit)) {
			//Cerr << "positive var at " << i << endl;
			return false;
		}
		i++;
	}
	return (free_vars == 1);
}

Literal Solver::GetUnitLiteral(RefClause clause) { 
	return clause->lits_[1];
	Assert(false, "Unit literal not found.");
	assert(false); // omit warning
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

int Solver::CurrentDecisionLevel() {
	return decision_levels_.size();
}

bool Solver::IsLitPositive(Literal& lit) { return GetLitValue(lit) == kPositive; }

bool Solver::IsLitNegative(Literal& lit) { return GetLitValue(lit) == kNegative; }

bool Solver::IsVarPositive(int var) { return GetVarValue(var) == kPositive; }

bool Solver::IsVarNegative(int var) { return GetVarValue(var) == kNegative; }

bool Solver::IsVarUndefined(int var) { return GetVarValue(var) == kUndefined; }

bool Solver::IsVarFree(int var) { return var_db_.free_vars_.count({var_db_.activeness_[var], var}) == 1; }

void Solver::SetSat() { state_ = kSatisfiableState; }

void Solver::SetUnsat() { state_ = kUnsatisfiableState; }

int Solver::IsSat() { return state_ == kSatisfiableState; }

int Solver::IsUnsat() { return state_ == kUnsatisfiableState; }

int Solver::IsUnsolved() { return state_ == kUnknownState; }

void Solver::Print() {
	assert(!IsUnsolved());
  if (IsUnsat()) {
    cerr << "UNSAT" << endl;
    return;
  }
  Assert(Verify(), "Assignment is not correct.");
  cerr << "SAT" << endl;
  for (int i = 0; i < (int)vars_.size(); i++) {
    cerr << vars_[i] * (i+1) << " ";
  }
  cerr << endl;
}