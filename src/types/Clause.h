#ifndef CLAUSE_H
#define CLAUSE_H

#include "Literal.h"

class Solver;
class Clause;

typedef shared_ptr<Clause> RefClause;

// Contains all information we have about single clause.
class Clause {
 public:
  Clause(vector<Literal>& lits, int id, bool active, bool learnt);

  static RefClause Create(
      Solver* solver, vector<Literal>& lits, bool active, bool learnt);

  int FindWatcher(Solver* solver);

  int GetNumFreeLiterals(Solver* solver);

  double GetFulfillment(Solver* solver);

  int GetLBD(Solver* solver);

  bool IsLocked(Solver* solver);

  bool IsLearnt();

  bool IsActive();

  void Deactive();

  bool operator<(const Clause& clause) const {
    return id_ < clause.id_;
  }

 private:
  int id_;
  bool active_;
  bool learnt_;
  vector<Literal> lits_;

 friend class Solver;
 friend class VarDatabase;
};


#endif