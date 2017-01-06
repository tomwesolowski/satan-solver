// Responsible for parsing data from input.
class Parser {
 public:
	// Parse CNF file and initialize Solver.
	void Parse(ifstream& ifs, Solver &solver);

 private:
	string line_;
	string last_;
};
