/*class Model {
  vector<int> vars_;
  vector<int> free_vars_;
  vector< vector<shared_ptr<Clause> > > watchers;
  vector<shared_ptr<Clause>> clauses;
  int state_ = kUnknown;

 public:
  Model(int num_vars) { 
  	vars_.resize(num_vars, kUndefined);
  	for(int i = 0; i < (int)vars_.size(); i++) {
  		free_vars_.push_back(i);
  	}
  	SetUndefined();
  }

  Model(vector<int>& v) {
    vars_ = v;
    SetSat();
  }

  void Set(int var, int val) { 
  	if(vars_[var] == kUndefined) {
  		for(int i = 0; i < (int)free_vars_.size(); i++) {
  			if(free_vars_[i] == var) {
  				swap(free_vars_[i], free_vars_.back());
  				free_vars_.pop_back();
  				break;
  			}
  		}
  	}
  	vars_[var] = val; 
  	if(free_vars_.size() <= 0) {
  		SetSat();
  	}
  }

  void Unset(int var) { 
  	if(vars_[var] != kUndefined) {
  		free_vars_.push_back(var);
  	}
  	vars_[var] = kUndefined; 
  	if(free_vars_.size() > 0) {
  		SetUndefined();
  	}
  }

  int GetNumFree() {
  	return free_vars_.size();
  }

  int Get(int var) { return vars_[var]; }

  bool IsVarPositive(int var) { return Get(var) == kPositive; }

  bool IsVarNegative(int var) { return Get(var) == kNegative; }

  bool IsVarUndefined(int var) { return Get(var) == kUndefined; }

  void SetSat() { state_ = kSatisfiable; }

  void SetUndefined() { state_ = kUndefined; }

  void SetUnsat() { state_ = kUnsatisfiable; }

  int IsSat() { return state_ == kSatisfiable; }

  int IsUnsat() { return state_ == kUnsatisfiable; }

  int IsUnsolved() { return state_ == kUndefined; }

  void Print() {
    if (IsUnsat()) {
      cout << "Problem is UNSATISFIABLE." << endl;
    }
    for (int i = 0; i < (int)vars_.size(); i++) {
      cout << "x_" + (i + 1) << " = " << vars_[i] << endl;
    }
  }
};*/