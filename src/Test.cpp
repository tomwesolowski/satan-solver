#include <bits/stdc++.h>

using namespace std;

#include "Constants.h"
#include "Literal.h"
#include "Helpers.h"
#include "Clause.h"
#include "Solver.h"
#include "Parser.h"
#include "gtest.h"

int z, n;

/*
TEST(f0, Sample) {
	ifstream ifs("./tests/simple.cnf");

	Parser parser;
	Solver solver;
	Cerr << "init ok" << endl;
	parser.Parse(ifs, solver);
	Cerr << "parse ok" << endl;
  solver.Solve();
  Cerr << "solve ok" << endl;
  solver.Verify();
  solver.Print();
}

TEST(f1, Sample) {
	ifstream ifs("./tests/simple_unsat.cnf");

	Parser parser;
	Solver solver;
	Cerr << "init ok" << endl;
	parser.Parse(ifs, solver);
	Cerr << "parse ok" << endl;
  solver.Solve();
  Cerr << "solve ok" << endl;
  solver.Verify(false);
  solver.Print();
}


TEST(f2, Sample) {
	for(int i = 2; i <= 8; i++) {
		stringstream ss;
		ss << i; string s; ss >> s;
		ifstream ifs("./tests/pret/pret" + s + ".cnf");

		Parser parser;
		Solver solver;

		parser.Parse(ifs, solver);
		
	  solver.Solve();
	  solver.Verify(i>4);
	  //solver.Print();	
	  cerr << ".";
	}
	
}


*/

TEST(f5, Sample) {
	for(int i = 1; i < 100; i++) {
		stringstream ss;
		ss << i; string s; ss >> s;
		ifstream ifs("./tests/uf125/uf125-0" + s + ".cnf");

		Solver solver;
		Parser parser;

		parser.Parse(ifs, solver);
		
	  solver.Solve();
	  solver.Verify();
	  //solver.Print();	
	  cerr << ".";
	}
	
}

TEST(Parity, PAR16) {
	for(int i = 1; i <= 1; i++) {
		stringstream ss;
		ss << i; string s; ss >> s;
		ifstream ifs("./tests/parity/par16-" + s + ".cnf");

		Parser parser;
		Solver solver;

		parser.Parse(ifs, solver);
		
	  solver.Solve();
	  solver.Verify();
	  //solver.Print();	
	  cerr << ".";
	}
	
}




TEST(SSA, SSA) {
	for(int i = 1; i < 9; i++) {
		stringstream ss;
		ss << i; string s; ss >> s;
		ifstream ifs("./tests/ssa/ss" + s + ".cnf");

		Parser parser;
		Solver solver;

		parser.Parse(ifs, solver);
		
	  solver.Solve();
	  solver.Verify(i>4);
	  //solver.Print();	
	  cerr << ".";
	}
	
}


/*



TEST(f4, Sample) {
	for(int i = 1; i <= 99; i++) {
		stringstream ss;
		ss << i; string s; ss >> s;
		ifstream ifs("./tests/uf75/uf75-0" + s + ".cnf");

		Parser parser;
		Solver solver;

		parser.Parse(ifs, solver);
		
	  solver.Solve();
	  solver.Verify();
	  //solver.Print();	
	  cerr << ".";
	}
	
}




TEST(f6, Sample) {
	for(int i = 1; i < 100; i++) {
		stringstream ss;
		ss << i; string s; ss >> s;
		ifstream ifs("./tests/unsat50/uuf50-0" + s + ".cnf");

		Solver solver;
		Parser parser;

		parser.Parse(ifs, solver);
		
	  solver.Solve();
	  solver.Verify(false);
	  //solver.Print();	
	  cerr << ".";
	}
	
}


TEST(f3, Sample) {
	for(int i = 1; i < 100; i++) {
		stringstream ss;
		ss << i; string s; ss >> s;
		ifstream ifs("./tests/uf50/uf50-0" + s + ".cnf");

		Parser parser;
		Solver solver;

		parser.Parse(ifs, solver);
		
	  solver.Solve();
	  solver.Verify();
	  //solver.Print();	
	  cerr << ".";
	}
	
}


TEST(f4, Sample) {
	for(int i = 5; i < 9; i++) {
		stringstream ss;
		ss << i; string s; ss >> s;
		ifstream ifs("./tests/ssa/ss" + s + ".cnf");

		Parser parser;
		Solver solver;

		parser.Parse(ifs, solver);
		
	  solver.Solve();
	  solver.Verify(i>4);
	  //solver.Print();	
	  cerr << ".";
	}
	
}



TEST(f6, Sample) {
	for(int i = 1; i <= 7; i++) {
		cerr << i;
		stringstream ss;
		ss << i; string s; ss >> s;
		ifstream ifs("./tests/crafted/SAT09/CRAFTED/edgematching/all/" + s + ".cnf");

		Solver solver;
		Parser parser;

		parser.Parse(ifs, solver);
		
	  solver.Solve();
	  solver.Verify();
	  //solver.Print();	
	  cerr << "\r";
	}
	
}


TEST(f5, Sample) {
	for(int i = 1; i < 100; i++) {
		stringstream ss;
		ss << i; string s; ss >> s;
		ifstream ifs("./tests/unsat50/uuf50-0" + s + ".cnf");

		Solver solver;
		Parser parser;

		parser.Parse(ifs, solver);
		
	  solver.Solve();
	  solver.Verify(false);
	  //solver.Print();	
	  cerr << ".";
	}
	
}









TEST(f3, Sample) {
	for(int i = 1; i < 100; i++) {
		stringstream ss;
		ss << i; string s; ss >> s;
		ifstream ifs("./tests/unsat50/uuf50-0" + s + ".cnf");

		Parser parser;
		Solver solver;

		parser.Parse(ifs, solver);
		
	  solver.Solve();
	  solver.Verify(false);
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
		Parser parser;

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
		Parser parser;

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
		Parser parser;

		parser.Parse(ifs, solver);	
		
	  solver.Solve();
	  solver.Verify();
	  //solver.Print();	
	  cerr << ".";
	}
	
}
*/

/*

int main() {
  ios_base::sync_with_stdio(0);

  parser.Parse(solver);
  solver.Solve();
  solver.Print();
}

*/