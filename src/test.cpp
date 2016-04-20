#include <bits/stdc++.h>

using namespace std;

#include "constants.h"
#include "literal.h"
#include "helpers.h"
#include "clause.h"
#include "solver.h"
#include "parser.h"
#include "gtest.h"

int z, n;

Parser parser;
Solver solver;

TEST(f0, Sample) {
	ifstream ifs("./tests/simple.cnf");

	Solver solver;
	cerr << "init ok" << endl;
	parser.Parse(ifs, solver);
	cerr << "parse ok" << endl;
  solver.Solve();
  cerr << "solve ok" << endl;
  solver.Verify();
  solver.Print();
}

TEST(f1, Sample) {
	for(int i = 1; i < 100; i++) {
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

TEST(f2, Sample) {
	for(int i = 1; i < 100; i++) {
		stringstream ss;
		ss << i; string s; ss >> s;
		ifstream ifs("./tests/uf125/uf125-0" + s + ".cnf");

		Solver solver;

		parser.Parse(ifs, solver);
		
	  solver.Solve();
	  solver.Verify();
	  //solver.Print();	
	  cerr << ".";
	}
	
}

TEST(f3, Sample) {
	for(int i = 1; i < 100; i++) {
		stringstream ss;
		ss << i; string s; ss >> s;
		ifstream ifs("./tests/uf150/uf150-0" + s + ".cnf");

		Solver solver;

		parser.Parse(ifs, solver);
		
	  solver.Solve();
	  solver.Verify();
	  //solver.Print();	
	  cerr << ".";
	}
	
}

TEST(f4, Sample) {
	for(int i = 1; i < 100; i++) {
		stringstream ss;
		ss << i; string s; ss >> s;
		ifstream ifs("./tests/uf250/uf250-0" + s + ".cnf");

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