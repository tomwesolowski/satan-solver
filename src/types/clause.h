#ifndef CLAUSE_H
#define CLAUSE_H

class Solver;
class Clause;

typedef shared_ptr<Clause> RefClause;

class Clause {
public:
  vector<Literal> lits_;
  int watch_pointer_ = 2;

  bool learnt = false;

  int id_;

  static RefClause Create(Solver* solver, vector<Literal>& lits);

  Clause(vector<Literal>& lits, int id);

  int FindWatcher(Solver* solver);
};


#endif