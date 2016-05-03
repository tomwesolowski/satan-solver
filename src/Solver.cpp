#include <bits/stdc++.h>
#include "Constants.h"
#include "Literal.h"
#include "Helpers.h"
#include "Clause.h"
#include "Solver.h"

using namespace std;

shared_ptr<Clause> NullClause = nullptr;

bool Solver::Solve() {
	srand(time(NULL));

	kLearntLimit = clauses_.size();
	kConflictLimit = 450;

	int status;
	do {
		num_fixed_clauses = clauses_.size();
		num_conflicts_ = 0;
		Backtrack(0);
		//cerr << "@";
		cerr << "_";
		kLearntLimit *= 1.8;
		kConflictLimit *= 1.5;
		status = Search();
	}
	while(status == kForceRestart);
	state_ = status;

	cerr << "clauses: " << clauses_.size() << " learnt: " << num_learnt_ << " conf: " << num_conflicts_ << endl;
  return IsSat();
}

int Solver::Search() {
  while (IsUnsolved()) {
  	//Cerr << clauses_.size() << endl;
    RefClause conflict = Propagate();
    if (conflict == NullClause) {
      if(!GetNumFree()) {
      	return kSatisfiableState;
      }
      if(++num_conflicts_ > kConflictLimit) {
      	return kForceRestart;
      }
      if(CurrentDecisionLevel() == 0) {
    		Simplify();
    	}
    	Reduce();
      Decide();
    } else {
    	if(CurrentDecisionLevel() <= 0) {
    		return kUnsatisfiableState;
    	}
      // conflict!
		  ClearPropagationQueue();

      vector<Literal> learnt_clause;
      int level = Analyze(conflict, learnt_clause);
      Backtrack(level);
      assert(AddClause(learnt_clause, true));
    }
  }
  Assert(false, "Solved instance has not been caught.");
  return kUnsatisfiableState;
}

void Solver::InitVars(int num_vars) { 
	vars_.resize(num_vars, kUndefined);
	reason_.resize(num_vars, nullptr);
	watchers_.resize(2*num_vars);
	level_.resize(num_vars);
	var_db_.Init(this);
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
		var_db_.BumpActivity(this, lit.var(), 50-lits.size());
	}

	assert(lits.size() > 0);
	assert(!learnt || lits.size() > 1 || CurrentDecisionLevel() == 0);

	num_learnt_ += learnt;

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

	Literal lit = var_db_.GetNext(this);

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

  		if(!clause->active) {
  			continue;
  		}

  		int status = UpdateWatcher(clause, -lit);

  		if(status == kUnitClause) {

  			Literal unit = GetUnitLiteral(clause);

  			if(!Enqueue(unit, clause)) {
  				Cerr << "and conflict " << unit.var() << " " << GetVarValue(unit.var()) << " " << unit.sign() << " " << clause->lits_.size() << endl;

  				conflict = clause;

  				//todo
  				for(Literal& lit : conflict->lits_) {
  					var_db_.BumpActivity(this, lit.var());
  				}
  				break;
  			}
  		}

  	}

  	if(conflict != nullptr) {
	  	//add remaining watching clauses
	  	for(RefClause rem_clause: watchers) {
	  		if(rem_clause->active) {
	  			watchers_[lit.index()].push_back(rem_clause);	
	  		}
				
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
  	Assert(p.sign() != kUndefined, "p is undefined");

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

bool Solver::Enqueue(Literal lit, RefClause from) {
	Assert(lit.sign() != kUndefined, "Enqueuing undefined var.");
	Cerr << "enqueue " << lit.var() << " " << lit.sign() << endl;
	if(vars_[lit.var()] != kUndefined) {
		if(vars_[lit.var()] != lit.sign()) {
			return false;
		}
		return true;
	}

	// enqueuing...
	var_db_.RemoveFree(this, lit.var());
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

void Solver::UnsetVar() { 
	int var = trail_.back().var();
	Assert(vars_[var] != kUndefined, "Un-assigning non-undefined var.");
	
	var_db_.AddToFreeVars(this, var);
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

void Solver::ClearPropagationQueue() {
	while(prop_queue_.size()) {
  	RemoveFromPropagateQueue();
  }
}

struct Solver::CompActivity {
	Solver* solver_;

	CompActivity(Solver* solver) :
		solver_(solver) {}

	bool operator() (const RefClause& a, const RefClause& b) const {
		assert(a.get() != nullptr);
		assert(b.get() != nullptr);
		//cerr << ">" << (b.get() != nullptr) << endl;
		if(a->learnt < b->learnt) {
			return true;
		}
		if(a->locked(solver_) > b->locked(solver_)) {
			return true;
		}
		return solver_->activity_[a] > solver_->activity_[b];
	}
};

/*
void Solver::SortClausesByActivity() {
	//sort(clauses_.begin(), clauses_.end(), CompActivity(this));
	for(int i = 0; i < clauses_.size(); i++) {
		for(int j = i; j > 0; j--) {
			RefClause a = clauses_[j-1];
			RefClause b = clauses_[j];
			if(a->learnt > b->learnt || activity_[a] > activity_[b]) {
				swap(clauses_[j-1], clauses_[j]);
			}
			else break;
		}
	}
}
*/

void Solver::SortClausesByActivity(int a, int b) {
	if(a > b) {
		cerr << a << " " << b << endl;
		assert(false);
	}
	if(a == b) {
		return;
	}
	int mid = (a+b)/2;

	SortClausesByActivity(a, mid);
	SortClausesByActivity(mid+1, b);

	int i = a, j = mid+1, d = 0;
	vector<RefClause> tmp(b-a+1);

	while(i <= mid || j <= b) { // O(d), d = b-a+1
		RefClause& x = clauses_[i];
		RefClause& y = clauses_[j];
		if(i <= mid && 
				(j > b || (x->learnt > y->learnt || x->locked(this) > y->locked(this) || activity_[x] > activity_[y]))) {
			tmp[d] = clauses_[i];
			d++; i++;
		}
		else { 
			tmp[d] = clauses_[j];
			d++; j++;
		}
	}
	for(int i = a; i <= b; i++) { // O(d), d = b-a+1
		clauses_[i] = tmp[i-a];
	}
}

void Solver::Reduce() {
	if((int)clauses_.size()-num_fixed_clauses <= kLearntLimit) {
		return;
	}
	//cerr << num_fixed_clauses << " : " << clauses_.size() << endl;
	SortClausesByActivity(num_fixed_clauses, clauses_.size()-1);
	while(clauses_.size() - num_fixed_clauses > kLearntLimit) {
		if(!clauses_.back()->learnt) break;
		if(clauses_.back()->locked(this)) break;
		clauses_.back()->active = 0;
		clauses_.pop_back();
	}
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
	Assert(solvable, "Assignment found but it's not solvable :D");
	return true;
}

int Solver::FlipValue(int value) {
	Assert(value != kUndefined, "Trying to flip undefined value");
	return (value == kPositive ? kNegative : kPositive);
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