#ifndef SOLVER_H
#define SOLVER_H


const int DEBUG = 0;
#define Cerr if(DEBUG) cerr

class Solver {
 public:
	std::default_random_engine generator_;
	
	vector<int> level_;
  set< pair<int, int> > free_vars_; // <activeness, var>
  vector<int> activeness_; // number of clauses with literal i

  vector< vector<RefClause > > watchers_;

  vector<RefClause> reason_;
  vector<RefClause> assumptions_;
  vector<RefClause> clauses_;
  queue<Literal> prop_queue_; // <var, value>

  vector<Literal> trail_; //<var, both_ways>
  vector<int> decision_levels_; // <var> - parent of level

  int state_ = kUnknownState;

 	vector<int> vars_;

  void InitVars(int num_vars);

  void AddToFreeVars(int var);

  void PrepareFreeVars();

  void MakeAssumptions();
  
  bool AddClause(vector<Literal> lits, bool learnt = false);

  void Decide();

  RefClause Propagate();

  //level - the latest level to NOT be erased
  bool Backtrack(int level);

  int Analyze(RefClause conflict, vector<Literal>& learnt_clause);

  bool Search();

  Literal RemoveFromPropagateQueue();

  bool Verify(bool solvable = true);

  bool Solve();

  int UpdateWatcher(RefClause clause, Literal lit);

  bool Enqueue(Literal lit, RefClause from = nullptr);

  int FlipValue(int value);

  void UnsetVar();

  void RemoveFree(int var);

  void Simplify();

  int GetNumFree();

  int CurrentDecisionLevel();

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

  bool IsVarFree(int var);

  void SetSat();

  void SetUnsat();

  int IsSat();

  int IsUnsat();

  int IsUnsolved();

  void Print();
  
};

#endif
// SOLVER_H