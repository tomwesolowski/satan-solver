#ifndef CLAUSE_H
#define CLAUSE_H

class Solver;
class Clause;

typedef shared_ptr<Clause> RefClause;

class Clause {
public:
	int id_;
  vector<Literal> lits_;

  bool learnt = false;

  static RefClause Create(Solver* solver, vector<Literal>& lits);

  Clause(vector<Literal>& lits, int id);

  int FindWatcher(Solver* solver);
};


#endif