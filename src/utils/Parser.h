class Parser {
 public:
	// Parse CNF file.
	void Parse(ifstream& ifs, Solver &solver);
	
 private:
	string line_;
	string last_;
};
