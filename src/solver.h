#ifndef SOLVER_H
#define SOLVER_H

const int DEBUG = 0;
#define Cerr if(DEBUG) cerr

class Solver {
	std::default_random_engine generator_;
	
	vector<int> var_level_;
  vector<int> free_vars_;
  vector< vector<RefClause > > watchers_;
  vector<RefClause> clauses_;
  //vector<RefClause> unset_clauses_;
  queue<Literal> prop_queue_; // <var, value>
  vector<int> in_prop_queue_;

  vector<VarAssignment> trail_; //<var, both_ways>
  vector<int> decision_levels_; // <var> - parent of level

  int state_ = kUnknown;

 public:

 	vector<int> vars_;

  void InitVars(int num_vars) { 
  	vars_.resize(num_vars, kUndefined);
  	for(int i = 0; i < (int)vars_.size(); i++) {
  		free_vars_.push_back(i);
  	}
  	watchers_.resize(num_vars);
  	var_level_.resize(num_vars);
  	in_prop_queue_.resize(num_vars, 0);
  }

  // vector < vector<RefClause > watches_;
  
  void AddClause(vector<Literal> lits) { 
  	clauses_.push_back(Clause::Create(this, lits)); 
  	//unset_clauses_.push_back(clauses_.back()); 

  	sort(lits.begin(), lits.end());
  	lits.resize(distance(lits.begin(), unique(lits.begin(), lits.end())));

  	int last_var = -1;
  	for(Literal& lit : lits) {
  		if(last_var == lit.var()) return; // <xi and -xi - always satisfiable>
  		last_var = lit.var();
  	}

  	for(Literal& lit : lits) {
  		//if(lit.sign() == kNegative) {
  			watchers_[lit.var()].push_back(clauses_.back());
  		/*}
  		else {
  			assert(lit.sign() == kPositive);
  		}*/
  	}
  }

  void Decide() {
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

  RefClause& Propagate() {
    while (prop_queue_.size()) {
    	Literal lit = RemoveFromPropagateQueue();
    	Cerr << "Prop " << lit.var()+1 << " val: " << GetVarValue(lit.var()) << endl;

    	if(trail_.size() && trail_.back().first == lit.var()) {
    		trail_.back().second = 1;
    	}
    	else {
    		AddToTrail(lit.var());
    	}

    	SetVar(lit.var(), lit.sign());
    	Cerr << "Free: " << GetNumFree() << endl;
    	for(RefClause& clause : watchers_[lit.var()]) {
    		if(IsClauseUnsatisified(clause)) {
    			Cerr << "conflict!" << endl;
    			return clause;
    		}
    		if(IsUnitClause(clause)) {
    			Literal unit = GetUnitLiteral(clause);
    			if(!AddToPropagateQueue(unit)) {
    				return clause;
    			}
    		}
    	}
    }
    return NullClause;
  }

  //level - the latest level to NOT be erased
  bool Backtrack(int level) {
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

  int Analyze(RefClause& conflict) {
    // returns the most recent level causing problem
    int max_level = -1;
  	for(Literal& lit : conflict->lits_) {
  		max_level = max(max_level, var_level_[lit.var()]);
  	}
  	Cerr << "analyze " << max_level << endl;
  	return max_level;
  }

  bool Search() {
    while (IsUnsolved()) {
      RefClause& conflict = Propagate();
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

  bool AddToPropagateQueue(Literal lit) {
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

  Literal RemoveFromPropagateQueue() {
  	Assert(prop_queue_.size(), "Trying to pop from empty prop queue.");
  	Literal lit = prop_queue_.front();
  	prop_queue_.pop();
  	in_prop_queue_[lit.var()] = kUndefined;
  	return lit;
  }

  void AddToTrail(int var) {
  	trail_.push_back({var, 0});
  }

  bool Verify() {
  	for(RefClause& clause : clauses_) {
  		if(IsClauseUnsatisified(clause)) return false;
  	}
  	return true;
  }

  bool Solve() {
  	state_ = (Search() ? kSatisfiable : kUnsatisfiable);
    return IsSat();
  }

  bool Enqueue(Literal lit) {
  	return true;
  }

  int FlipValue(int value) {
  	Assert(value != kUndefined, "Trying to flip undefined value");
  	return (value == kPositive ? kNegative : kPositive);
  }

  void FlipVar(int var) {
  	SetVar(var, FlipValue(GetVarValue(var)));
  }

  void SetVar(int var, int val) { 
  	if(vars_[var] == kUndefined) {
  		RemoveFree(var);
  	}
  	vars_[var] = val; 
  	var_level_[var] = decision_levels_.size();
  	Cerr << var+1 << " val: " << val << " " << "lvl: " << decision_levels_.size() << endl;
  }

  void UnsetVar(int var) { 
  	if(vars_[var] != kUndefined) {
  		free_vars_.push_back(var);
  	}
  	vars_[var] = kUndefined; 
  	var_level_[var] = -1;
  }

  void RemoveFree(int var) {
  	for(int i = 0; i < (int)free_vars_.size(); i++) {
  		if(free_vars_[i] == var) {
  			swap(free_vars_[i], free_vars_.back());
  			free_vars_.pop_back();		
  			return;
  		}
  	}
  	Assert(false, "Variable was not free");
  }

  int GetNumFree() {
  	return free_vars_.size();
  }

  bool IsUnitClause(RefClause& clause) { 
  	int free_vars = 0;
  	for(Literal& lit : clause->lits_) {
  		free_vars += IsVarUndefined(lit.var());
  		if(IsLitPositive(lit)) return false;
  	}
  	return (free_vars == 1);
  }

  Literal GetUnitLiteral(RefClause& clause) { 
  	for(Literal& lit : clause->lits_) {
  		if(IsVarUndefined(lit.var())) {
  			return lit;
  		}
  	}
  	Assert(false, "Unit literal not found.");
  	assert(false); // just to get rid of warning
  } 

  bool IsClauseSatisified(RefClause& clause) { 
  	for(Literal& lit : clause->lits_) {
  		if(GetLitValue(lit) == kPositive) {
  			return true;
  		}
  	}
  	return false;
  }

  bool IsClauseUnsatisified(RefClause& clause) { 
  	for(Literal& lit : clause->lits_) {
  		if(GetLitValue(lit) != kNegative) {
  			return false;
  		}
  	}
  	return true;
  }

  int GetVarValue(int var) { return vars_[var]; }

  int GetLitValue(Literal& lit) { 
  	return vars_[lit.var()] * lit.sign(); 
  }

  bool IsLitPositive(Literal& lit) { return GetLitValue(lit) == kPositive; }

  bool IsLitNegative(Literal& lit) { return GetLitValue(lit) == kNegative; }

  bool IsVarPositive(int var) { return GetVarValue(var) == kPositive; }

  bool IsVarNegative(int var) { return GetVarValue(var) == kNegative; }

  bool IsVarUndefined(int var) { return GetVarValue(var) == kUndefined; }

  void SetSat() { state_ = kSatisfiable; }

  void SetUnsat() { state_ = kUnsatisfiable; }

  int IsSat() { return state_ == kSatisfiable; }

  int IsUnsat() { return state_ == kUnsatisfiable; }

  int IsUnsolved() { return state_ == kUndefined; }

  void Print() {
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
  
};

#endif
// SOLVER_H