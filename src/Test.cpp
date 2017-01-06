#include <bits/stdc++.h>

#include "gtest.h"

#include "Constants.h"
#include "Clause.h"
#include "Helpers.h"
#include "Literal.h"
#include "Parser.h"
#include "Solver.h"

TEST(TestSample, Sample) {
	ifstream ifs("./tests/simple.cnf");
	SolverParameters params;

	Parser parser;
	Solver solver(params);
	parser.Parse(ifs, solver);
  solver.Solve();
  solver.Verify(true);
  solver.Print();
}

TEST(uf50, Sample) {
	for(int i = 1; i < 1000; i++) {
		std::ostringstream oss;
		oss << "./tests/uf50/uf50-0" << i << ".cnf";
		ifstream ifs(oss.str());

		SolverParameters params;
		Parser parser;
		Solver solver(params);

		parser.Parse(ifs, solver);	
	  solver.Solve();
	  oss.clear();
	  oss << "Test #" << i << " failed.";
	  Assert(solver.Verify(true), oss.str()); // all models are solvable.
	}	
}
