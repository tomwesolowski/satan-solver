#ifndef CLAUSE_H
#define CLAUSE_H

class Solver;

struct Clause {
  vector<Literal> lits_;

  static shared_ptr<Clause> Create(Solver* solver, vector<Literal>& lits) { 
  	return make_shared<Clause>(lits);
  }

  Clause(vector<Literal>& lits) {
  	lits_ = lits;
  }

};

#endif