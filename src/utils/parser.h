struct Parser {

	string line_;
	stringstream sstream_;

	string GetString() {
		string tmp;
		sstream_ >> tmp;
		return tmp;
	}

	int GetInt() {
		int tmp;
		sstream_ >> tmp;
		return tmp;
	}

	void Parse(ifstream& ifs, Solver &solver) {
		//comments
		while(getline(ifs, line_)) {
			if(line_[0] != 'c') break;
		}
		sstream_.clear();

		//parameters
		sstream_ << line_;
		assert(GetString() == "p");
		assert(GetString() == "cnf");
		
		int num_vars = GetInt();
		int num_clauses = GetInt();


		solver.InitVars(num_vars);

		//clauses
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
