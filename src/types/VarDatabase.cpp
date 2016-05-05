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

	if(apps_[lit_pos.index()] - solver->watchers_[lit_neg.index()].size() 
			> apps_[lit_neg.index()] - solver->watchers_[lit_pos.index()].size()) {
		lit = lit_pos;
	}
	else {
		lit = lit_neg;
	}
	/*
	if(solver->watchers_[lit_neg.index()].size() > 
			solver->watchers_[lit_pos.index()].size()) {
		lit = lit_pos;
	}
	else if(solver->watchers_[lit_pos.index()].size()) {
		lit = lit_neg;
	}
	*/
	//cerr << free_vars_.rbegin()->first << endl;
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
	apps_.resize(2*num_vars_, 0);
	PrepareFreeVars(solver);
}

void VarDatabase::ChangeAppearance(Solver* solver, Literal lit, int val) {
	apps_[lit.index()] += val;	
}

void VarDatabase::ChangeAppearance(Solver* solver, RefClause clause, int val) {
	for(Literal& lit : clause->lits_) {
		ChangeAppearance(solver, lit, val);
	}
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

void VarDatabase::BumpActivity(Solver* solver, RefClause clause, int value) {
	for(Literal& lit : clause->lits_) {
		BumpActivity(solver, lit.var(), value);
	}
}

bool VarDatabase::IsVarFree(Solver* solver, int var) {
	return is_free_[var] == 1;
}


int VarDatabase::GetNumFree(Solver* solver) {
	return num_free_;
}

void VarDatabase::DecayActivities(Solver* solver) {
	//return;
	bool rescale = false;

	for(int var = 0; var < num_vars_; var++) {
		if(IsVarFree(solver, var)) {
			RemoveFree(solver, var);
			activity_[var] *= kDecayFactor;
			AddToFreeVars(solver, var);
		}
		else {
			activity_[var] *= kDecayFactor;
		}
		if(activity_[var] > kMaxActivity) rescale = true;
	}

	if(rescale) {
		for(int var = 0; var < num_vars_; var++) {
			if(IsVarFree(solver, var)) {
				RemoveFree(solver, var);
				activity_[var] *= kRescaleFactor;
				AddToFreeVars(solver, var);
			}
			else {
				activity_[var] *= kRescaleFactor;
			}
		}		
	}
}

void VarDatabase::AdjustDecay(Solver* solver, int diff_levels) {
	diffs_queue_.push(diff_levels);
	sum_diffs_ += diff_levels;
	if(diffs_queue_.size() > 10) {
		sum_diffs_ -= diffs_queue_.front();
		diffs_queue_.pop();
	}
	avg_diffs_ = sum_diffs_ / diffs_queue_.size();
	//cerr << avg_diffs_ << endl;

	if(avg_diffs_ <= 7) {
		kDecayFactor = 0.99;
	}
	else {
		kDecayFactor = 0.75;
	}
}