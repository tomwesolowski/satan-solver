const int kUndefined = 0;
const int kPositive = 1;
const int kNegative = -1;

const int kUnsatisfiable = -1;
const int kSatisfiable = 1;
const int kUnknown = 0;

struct Clause;

shared_ptr<Clause> NullClause = nullptr;
typedef pair<int, int> VarAssignment; // <var, both_ways>
typedef shared_ptr<Clause> RefClause;