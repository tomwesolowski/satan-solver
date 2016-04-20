#ifndef SOLVER_H
#define SOLVER_H

const int DEBUG = 0;
#define Cerr if(DEBUG) cerr

class Solver {
 public:
	std::default_random_engine generator_;
	
	vector<int> var_level_;
  vector<int> free_vars_;
  vector< vector<RefClause > > watchers_;
  vector<RefClause> clauses_;
  queue<Literal> prop_queue_; // <var, value>
  vector<int> in_prop_queue_;

  vector<VarAssignment> trail_; //<var, both_ways>
  vector<int> decision_levels_; // <var> - parent of level

  int state_ = kUnknown;

 	vector<int> vars_;

  void InitVars(int num_vars);
  
  void AddClause(vector<Literal> lits);

  void Decide();

  RefClause Propagate();

  //level - the latest level to NOT be erased
  bool Backtrack(int level);

  int Analyze(RefClause conflict);

  bool Search();

  bool AddToPropagateQueue(Literal lit);

  Literal RemoveFromPropagateQueue();

  void AddToTrail(int var) ;

  bool Verify();

  bool Solve();

  bool UpdateWatchers(RefClause clause, bool forceAdd = false);

  bool Enqueue(Literal lit);

  int FlipValue(int value);

  void FlipVar(int var) ;

  void SetVar(int var, int val);

  void UnsetVar(int var);

  void RemoveFree(int var);

  int GetNumFree();

  bool IsUnitClause(RefClause clause);

  Literal GetUnitLiteral(RefClause clause);

  bool IsClauseSatisified(RefClause clause);

  bool IsClauseUnsatisified(RefClause clause);

  int GetVarValue(int var);

  int GetLitValue(Literal& lit) ;

  bool IsLitPositive(Literal& lit);

  bool IsLitNegative(Literal& lit);

  bool IsVarPositive(int var);

  bool IsVarNegative(int var);

  bool IsVarUndefined(int var);

  void SetSat();

  void SetUnsat();

  int IsSat();

  int IsUnsat();

  int IsUnsolved();

  void Print();
  
};

#endif
// SOLVER_H