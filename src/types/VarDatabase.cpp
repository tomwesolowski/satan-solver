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
	return lit;
}

void VarDatabase::AddToFreeVars(Solver* solver, int var) {
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

void VarDatabase::DecayAppearances(Solver* solver) {
	if(true) return; // temporary disabled.
	for(int& app : apps_) {
		app *= 0.8;
	}
}

void VarDatabase::RemoveFree(Solver* solver, int var) {
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
	const int kMaxDiffsQueueSize = 10;
	const int kMaxAvgDiff = 7;
	
	const double kDecayFactorSlow = 0.99;
	const double kDecayFactorFast = 0.75;

	diffs_queue_.push(diff_levels);
	sum_diffs_ += diff_levels;
	if(diffs_queue_.size() > kMaxDiffsQueueSize) {
		sum_diffs_ -= diffs_queue_.front();
		diffs_queue_.pop();
	}
	avg_diffs_ = sum_diffs_ / diffs_queue_.size();

	if(avg_diffs_ <= kMaxAvgDiff) {
		kDecayFactor = kDecayFactorSlow;
	}
	else {
		kDecayFactor = kDecayFactorFast;
	}
}