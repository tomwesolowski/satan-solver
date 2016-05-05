#include <bits/stdc++.h>

using namespace std;

#include "Constants.h"
#include "Helpers.h"
#include "Literal.h"
#include "Clause.h"
#include "VarDatabase.h"
#include "Solver.h"

Literal VarDatabase::GetNext(Solver* solver) {
	int var_free = free_vars_.rbegin()->second;
	Literal lit_pos(var_free, kPositive);
	Literal lit_neg(var_free, kNegative);
	Literal lit(var_free, (rand()%2 ? kPositive : kNegative));

	if(solver->watchers_[lit_neg.index()].size() > 
			solver->watchers_[lit_pos.index()].size()) {
		lit = lit_pos;
	}
	else if(solver->watchers_[lit_pos.index()].size()) {
		lit = lit_neg;
	}
	return lit;
}

void VarDatabase::AddToFreeVars(Solver* solver, int var) {
	//var_db_.activity_[var] = rand(); //FOR DEBUG
	Assert(!IsVarFree(solver, var), "AddToFreeVars: Trying to add to free vars again");
	free_vars_.insert({activity_[var], var});
	is_free_[var] = 1;
	num_free_++;
}

void VarDatabase::PrepareFreeVars(Solver* solver) {
	for(int i = 0; i < num_vars_; i++) {
		AddToFreeVars(solver, i);
	}
}

void VarDatabase::Init(Solver* solver) {
	num_vars_ = solver->vars_.size();
	activity_.resize(num_vars_, 0);
	is_free_.resize(num_vars_, 0);
	PrepareFreeVars(solver);
}

void VarDatabase::RemoveFree(Solver* solver, int var) {
	//Cerr << "rem " << var << ":" << var_db_.activity_[var] << endl;
	Assert(IsVarFree(solver, var), "Variable was not free");
	free_vars_.erase({activity_[var], var});
	is_free_[var] = 0;
	num_free_--;
}

void VarDatabase::BumpActivity(Solver* solver, int var, int value) {
	if(IsVarFree(solver, var)) {
		RemoveFree(solver, var);
		activity_[var] += value;
		AddToFreeVars(solver, var);
	}
	else {
		activity_[var] += value;
	}
	//activity_[var] += value;
}

bool VarDatabase::IsVarFree(Solver* solver, int var) {
	return is_free_[var] == 1;
}


int VarDatabase::GetNumFree(Solver* solver) {
	return num_free_;
}

void VarDatabase::DecayActivities(Solver* solver) {
	//return;
	for(int var = 0; var < num_vars_; var++) {
		if(IsVarFree(solver, var)) {
			RemoveFree(solver, var);
			activity_[var] /= decay;
			AddToFreeVars(solver, var);
		}
		else {
			activity_[var] /= decay;
		}
	}
}