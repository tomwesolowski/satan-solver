#ifndef CLAUSE_H
#define CLAUSE_H

#include "Literal.h"

class Solver;
class Clause;

typedef shared_ptr<Clause> RefClause;

class Clause {
public:
	int id_;
	bool active = true;
	bool learnt = false;

  vector<Literal> lits_;

  static RefClause Create(Solver* solver, vector<Literal>& lits);

  Clause(vector<Literal>& lits, int id);

  int FindWatcher(Solver* solver);

  bool locked(Solver* solver);

  bool operator<(const Clause& clause) const {
  	return id_ < clause.id_;
  }
};


#endif