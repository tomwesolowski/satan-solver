struct Parser {

	string line_;
	string last_;

	// Parse CNF file.
	void Parse(ifstream& ifs, Solver &solver) {
		Assert(ifs.good(), "Stream is not opened.");

		bool found_start = false;

		while(ifs >> line_) {
			if(last_ == "p" && line_ == "cnf") {
				found_start = true;
				break;
			}
			last_ = line_;
		}

		Assert(found_start, "CNF not found");
		
		int num_vars, num_clauses;
		ifs >> num_vars >> num_clauses;

		solver.InitVars(num_vars);

		// Clauses
		int var;
		while(num_clauses--) {
			vector<Literal> lits;
			while(ifs >> var) {
				if(var == 0) break;
				int sign = (var > 0 ? kPositive : kNegative);
				var = abs(var)-1;
				lits.push_back(Literal(var, sign));
			}	
			solver.AddClause(lits);
		}
	}
};
