#include <bits/stdc++.h>

using namespace std;

#include "helpers.h"
#include "constants.h"
#include "literal.h"
#include "clause.h"
#include "model.h"
#include "solver.h"
#include "parser.h"
#include "verificator.h"

#include "gtest.h"

const int inf = 1e9 + 1;
const int N = 100123;

int z, n;

Parser parser;
Solver solver;

TEST(f0, Sample) {
	ifstream ifs("./tests/simple.cnf");

	Solver solver;
	parser.Parse(ifs, solver);
  solver.Solve();
  solver.Verify();
  solver.Print();
}

TEST(f1, Sample) {
	for(int i = 1; i < 5; i++) {
		stringstream ss;
		ss << i; string s; ss >> s;
		ifstream ifs("./tests/uf75/uf75-0" + s + ".cnf");

		Solver solver;
		parser.Parse(ifs, solver);
	  solver.Solve();
	  solver.Verify();
	  //solver.Print();	
	  cerr << ".";
	}
	
}

/*

int main() {
  ios_base::sync_with_stdio(0);

  parser.Parse(solver);
  solver.Solve();
  solver.Print();
}

*/