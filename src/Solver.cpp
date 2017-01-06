#include <bits/stdc++.h>

#include "Clause.h"
#include "Constants.h"
#include "Helpers.h"
#include "Literal.h"
#include "Solver.h"

const shared_ptr<Clause> NullClause = nullptr;

Solver::Solver(SolverParameters params) :
    var_db_(params) {
  learnt_limit_ = params.learnt_limit_;
  conflicts_limit_ = params.conflicts_limit_;
  G = params.G;
}

// Main function of algorithm.
// Restarts search and controls its parameters when needed.
bool Solver::Solve() {
  srand(time(NULL));

  int status;
  do {
    num_fixed_clauses = clauses_.size();
    num_conflicts_ = 0;
    Backtrack(0);
    learnt_limit_ *= 1.8;
    conflicts_limit_ *= 1.1;
    status = Search();
  }
  while(status == kForceRestart);
  state_ = status;

  return IsSat();
}

// Looks for correct assignment.
int Solver::Search() {
  int iteration = 1;
  avg_level = 0;
  agility = 1;
  long double sum_levels = 0;

  while (IsUnsolved()) {
    RefClause conflict = Propagate();

    sum_levels += trail_.size();
    avg_level = sum_levels / iteration;

    if (conflict == NullClause) {
      if(!GetNumFree()) {
        return kSatisfiableState;
      }
      if(++num_conflicts_ > conflicts_limit_ && agility < 0.3) {
        return kForceRestart;
      }
      if(CurrentDecisionLevel() == 0) {
        Simplify();
      }
      Reduce();
      if((iteration&7) <= 0) {
        var_db_.DecayActivities();
      }
      if((iteration&127) <= 0) {
        var_db_.DecayAppearances();
      }
      Decide();
    } else {
      if(CurrentDecisionLevel() <= 0) {
        return kUnsatisfiableState;
      }
      // Ooops! We have a conflict here!
      ClearPropagationQueue();

      vector<Literal> learnt_clause;
      int level = Analyze(conflict, learnt_clause);
      Backtrack(level);
      AddClause(learnt_clause, true);
    }
    iteration++;
  }
  // If we're here, something went wrong.
  return kUnsatisfiableState;
}

void Solver::InitVars(int num_vars) { 
  vars_.resize(num_vars, kUndefined);
  polarity_.resize(num_vars, kUndefined);
  reason_.resize(num_vars, nullptr);
  watchers_.resize(2*num_vars);
  level_.resize(num_vars);
  var_db_.Init(num_vars);
}

// Adds clause to SAT instance.
bool Solver::AddClause(vector<Literal> lits, bool learnt) { 
  if(!learnt) {
    sort(lits.begin(), lits.end());
    lits.resize(distance(lits.begin(), unique(lits.begin(), lits.end())));  
  
    int last_var = -1;
    for(Literal& lit : lits) {
      if(last_var == lit.var()) {
        return true; // <x_i or -x_i - always satisfiable>
      }
      last_var = lit.var();
    }
  }
  num_learnt_ += learnt;

  if(lits.size() <= 1) {
    var_db_.BumpActivity(lits[0].var(), 1);
    if(!Enqueue(lits[0])) {
      return false;
    }
    return true;
  }

  if(learnt) {
    for(int i = 2; i < lits.size(); i++) {
      if(level_[lits[i].var()] > level_[lits[1].var()]) {
        swap(lits[1], lits[i]);
      }
    }
  }

  RefClause clause = Clause::Create(this, lits);
  clause->learnt = learnt;

  BumpActivity(clause);
  var_db_.ChangeAppearance(clause, 1);
  var_db_.BumpActivity(clause, max(1, 100-(int)lits.size()));

  Enqueue(lits[0], clause);
  clauses_.push_back(clause); 
  for(int i = 0; i < 2; i++) {
    watchers_[(-lits[i]).index()].push_back(clause);
  }
  return true;
}

// Picks the next variable to assign.
void Solver::Decide() {
  int free_var = var_db_.GetVarCandidate();
  Literal lit_pos(free_var, kPositive);
  Literal lit_neg(free_var, kNegative);
  Literal lit = var_db_.GetNext(
      free_var, NumWatchers(lit_pos.index()), 
      NumWatchers(lit_neg.index()));
  decision_levels_.push_back(trail_.size());
  Enqueue(lit);
}

// Assigns all literals waiting in queue.
RefClause Solver::Propagate() {
  while (prop_queue_.size()) {
    Literal lit = RemoveFromPropagateQueue();
    vector<RefClause> watchers = watchers_[lit.index()];
    watchers_[lit.index()].clear();
    RefClause conflict = nullptr;

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
          conflict = clause;
          var_db_.BumpActivity(conflict);
          break;
        }
      }
    }

    if(conflict != nullptr) {
      // Add remaining watching clauses.
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

// Sometimes things don't in a way we want.
// And we have to give up (or even better give up-up or give up-up-up).
// Level - the latest level to NOT be erased.
bool Solver::Backtrack(int level) {
  while(CurrentDecisionLevel() > level) {
    while(decision_levels_.back() < trail_.size()) {
      UnsetVar();
    }
    decision_levels_.pop_back();
  }
  return true;
}

// Returns the most recent level causing problem.
int Solver::Analyze(RefClause conflict, vector<Literal>& learnt_clause) {
  vector<bool> seen(vars_.size(), 0);
  Literal p(-1, kNegative);
  vector<Literal> reason;
  int max_level = 0;
  int counter = 0;
  int currentLevel = CurrentDecisionLevel();

  do {
    reason.clear();
    for(Literal& lit : conflict->lits_) {
      if(lit.var() != p.var()) {
        reason.push_back(lit);
      }
    }

    for(Literal& lit : reason) {
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
      p = trail_.back();
      conflict = reason_[p.var()];
      UnsetVar();
    }
    while(!seen[p.var()]);
    counter--;
  } while(counter > 0);

  set<int> diff_levels;
  for(Literal& lit : learnt_clause) {
    diff_levels.insert(level_[lit.var()]);
  }

  var_db_.AdjustDecay(diff_levels.size());
  learnt_clause.push_back(-p);
  swap(learnt_clause.front(), learnt_clause.back());

  return max_level;
}

// Adds literal to propagation queue.
bool Solver::Enqueue(Literal lit, RefClause from) {
  if(vars_[lit.var()] != kUndefined) {
    if(vars_[lit.var()] != lit.sign()) {
      return false;
    }
    return true;
  }

  var_db_.RemoveFree(lit.var());
  trail_.push_back(lit);
  vars_[lit.var()] = lit.sign(); 
  level_[lit.var()] = decision_levels_.size();
  reason_[lit.var()] = from;
  prop_queue_.push(lit);

  if(polarity_[lit.var()] != lit.sign()) {
    agility += (1-G);
  }
  agility *= G;
  polarity_[lit.var()] = lit.sign();

  return true;
}

// Tries to simplify clause - 
// to remove all unnecessary variables and clauses.
void Solver::Simplify() {
  for(int i = 0; i < clauses_.size(); i++) {
    RefClause& clause = clauses_[i];
    bool deleted = false;
    int k = 0;
    for(int j = 0; j < clause->lits_.size(); j++) {
      Literal& lit = clause->lits_[j];
      if(IsLitPositive(lit)) {
        // Remove clauses as it's always satisfied.
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
    }
  }
}

int Solver::UpdateWatcher(RefClause clause, Literal lit) {
  if(lit == clause->lits_[1]) {
    swap(clause->lits_[0], clause->lits_[1]);
  }

  int status = clause->FindWatcher(this);
  if(status & kNewWatcher) {
    watchers_[(-clause->lits_[0]).index()].push_back(clause);
  }
  else if(status & kHoldWatcher) {
    watchers_[(-lit).index()].push_back(clause);
  }

  if(status & kUnitClause) {
    return kUnitClause;
  }
  return kNormalClause;
}

void Solver::UnsetVar() { 
  int var = trail_.back().var();  
  var_db_.AddToFreeVars(var);
  vars_[var] = kUndefined; 
  reason_[var] = nullptr;
  level_[var] = -1;
  trail_.pop_back();
}

void Solver::BumpActivity(RefClause clause, int value) {
  activity_[clause] += value;
}

Literal Solver::RemoveFromPropagateQueue() {
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
    if(a->learnt < b->learnt) {
      return true;
    }
    if(a->locked(solver_) > b->locked(solver_)) {
      return true;
    }
    return solver_->activity_[a] > solver_->activity_[b];
  }
};

void Solver::SortClausesByActivity(int a, int b) {
  if(a > b) {
    return;
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
          (j > b || 
              (x->learnt > y->learnt || 
              x->locked(this) > y->locked(this) || 
              x->GetFulfillment(this)*activity_[x] > 
                  y->GetFulfillment(this)*activity_[y] ))) {
      tmp[d] = clauses_[i];
      d++; i++;
    }
    else { 
      tmp[d] = clauses_[j];
      d++; j++;
    }
  }
  for(int i = a; i <= b; i++) {
    clauses_[i] = tmp[i-a];
  }
}

void Solver::Reduce() {
  if((int)clauses_.size()-num_fixed_clauses <= learnt_limit_) {
    return;
  }
  SortClausesByActivity(num_fixed_clauses, clauses_.size()-1);
  while(clauses_.size() - num_fixed_clauses > learnt_limit_) {
    if(!clauses_.back()->learnt) break;
    if(clauses_.back()->locked(this)) break;

    clauses_.back()->active = 0;
    var_db_.ChangeAppearance(clauses_.back(), -1);
    smalls_reduced += (clauses_.back()->lits_.size() <= 3);
    clauses_.pop_back();
  }
}

bool Solver::Verify(bool solvable) {
  if(IsUnsat()) {
    return true;
  }
  for(RefClause clause : clauses_) {
    bool satisfied = false;
    for(Literal& lit : clause->lits_) {
      if(GetLitValue(lit) == kPositive) {
        satisfied = true;
      }
      if(GetLitValue(lit) == kUndefined) {
        std::cerr << "Variable has not been assigned." 
                  << std::endl;
        return false;
      }
    }
    if(!satisfied) {
      std::cerr << "Clause is not satisfied" << std::endl;
      return false;
    }
  }
  if(!solvable) {
    std::cerr << "Assignment found but model is not solvable." 
              << std::endl;
    return false;
  }
  return true;
}

int Solver::FlipValue(int value) {
  Assert(value != kUndefined, "Trying to flip undefined value");
  return (value == kPositive ? kNegative : kPositive);
}

int Solver::GetNumFree() {
  return var_db_.GetNumFree();
}

// -------------------------------------------------
// For debug purposes.
// -------------------------------------------------
bool Solver::IsUnitClause(RefClause clause) { 
  int free_vars = 0;
  int i = 0;
  for(Literal& lit : clause->lits_) {
    if(IsVarUndefined(lit.var())) {
      free_vars++;
    }
    if(IsLitPositive(lit)) {
      return false;
    }
    i++;
  }
  return (free_vars == 1);
}

Literal Solver::GetUnitLiteral(RefClause clause) { 
  return clause->lits_[1];
} 

RefClause Solver::GetReason(int var) {
  return reason_[var];
}

int Solver::GetLevel(int var) {
  return level_[var];
}

int Solver::NumWatchers(int index) {
  return watchers_[index].size();
}

int Solver::NumVars() {
  return vars_.size();
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
  Assert(!IsUnsolved(), "Did you run solver?");
  if (IsUnsat()) {
    cout << "UNSAT" << endl;
    return;
  }
  Assert(Verify(true), "Assignment is not correct.");
  cout << "SAT" << endl;
  for (int i = 0; i < (int)vars_.size(); i++) {
    cout << vars_[i] * (i+1) << " ";
  }
  cout << endl;
}