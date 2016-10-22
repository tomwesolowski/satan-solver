#include <bits/stdc++.h>

using namespace std;

#include "Constants.h"
#include "Literal.h"
#include "Helpers.h"
#include "Clause.h"
#include "Solver.h"
#include "Parser.h"
#include "gtest.h"

TEST(TestSample, Sample) {
	ifstream ifs("./tests/simple.cnf");

	Parser parser;
	Solver solver;
	parser.Parse(ifs, solver);
  solver.Solve();
  solver.Verify();
  solver.Print();
}