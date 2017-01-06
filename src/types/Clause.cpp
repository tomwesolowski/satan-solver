#include <bits/stdc++.h>

#include "Constants.h"
#include "Clause.h"
#include "Helpers.h"
#include "Literal.h"
#include "VarDatabase.h"
#include "Solver.h"

using namespace std;

RefClause Clause::Create(
    Solver* solver, vector<Literal>& lits, bool active, bool learnt) {
  static int id = 0; 
  return make_shared<Clause>(lits, id++, active, learnt);
}

Clause::Clause(vector<Literal>& lits, int id, bool active, bool learnt) 
    : id_(id),
      active_(active),
      learnt_(learnt),
      lits_(lits) {}

int Clause::FindWatcher(Solver* solver) {
  if(solver->GetLitValue(lits_[1]) == kPositive) {
    return kHoldWatcher | kNormalClause;
  }
  for(int j = 2; j < lits_.size(); j++) {
    if(solver->GetLitValue(lits_[j]) != kNegative) {
      swap(lits_[2], lits_[j]);
      swap(lits_[0], lits_[2]);
      return kNewWatcher | kNormalClause;
    }
  }
  return kHoldWatcher | kUnitClause;
}

int Clause::GetNumFreeLiterals(Solver* solver) {
  int free_literals = 0;
  for(Literal& lit : lits_) {
    free_literals += solver->IsVarUndefined(lit.var());
  }
  return free_literals;
}

double Clause::GetFulfillment(Solver* solver) {
  return (double)GetNumFreeLiterals(solver) / lits_.size();
}

int Clause::GetLBD(Solver* solver) {
  set<int> diff_levels;
  for(Literal& lit : lits_) {
    diff_levels.insert(solver->GetLevel(lit.var()));
  }
  return diff_levels.size();
}

bool Clause::IsLocked(Solver* solver) {
  return solver->GetReason(lits_[1].var()).get() == this; 
}

bool Clause::IsLearnt() {
  return learnt_;
}

bool Clause::IsActive() {
  return active_;
}

void Clause::Deactive() {
  active_ = false;
}