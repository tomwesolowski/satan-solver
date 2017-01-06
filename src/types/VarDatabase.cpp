#include <bits/stdc++.h>

#include "Constants.h"
#include "Clause.h"
#include "Helpers.h"
#include "Literal.h"
#include "VarDatabase.h"

using std::queue;
using std::pair;
using std::set;
using std::vector;

VarDatabase::VarDatabase(SolverParameters params) {
	decay_factor_ = params.decay_factor_;
	rescale_factor_ = params.rescale_factor_;
	max_activity_ = params.max_activity_;
	max_diff_queue_size_ = params.max_diff_queue_size_;
	max_avg_diff_ = params.max_avg_diff_;
	decay_factor_slow_ = params.decay_factor_slow_;
	decay_factor_fast_ = params.decay_factor_fast_;
}

int VarDatabase::GetVarCandidate() {
	return free_vars_.rbegin()->second;
}

Literal VarDatabase::GetNext(
		int free_var, int num_watchers_pos, int num_watchers_neg) {
	Literal lit_pos(free_var, kPositive);
	Literal lit_neg(free_var, kNegative);
	Literal lit(free_var, (rand()%2 ? kPositive : kNegative));
	if(apps_[lit_pos.index()] - num_watchers_neg
			> apps_[lit_neg.index()] - num_watchers_pos) {
		lit = lit_pos;
	}
	else {
		lit = lit_neg;
	}
	return lit;
}

void VarDatabase::AddToFreeVars(int var) {
	free_vars_.insert({activity_[var], var});
	is_free_[var] = 1;
	num_free_++;
}

void VarDatabase::Init(int num_vars) {
	num_vars_ = num_vars;
	activity_.resize(num_vars_, 0);
	is_free_.resize(num_vars_, 0);
	apps_.resize(2*num_vars_, 0);
	for(int i = 0; i < num_vars_; i++) {
		AddToFreeVars(i);
	}
}

void VarDatabase::ChangeAppearance(Literal lit, int val) {
	apps_[lit.index()] += val;	
}

void VarDatabase::ChangeAppearance(RefClause clause, int val) {
	for(Literal& lit : clause->lits_) {
		ChangeAppearance(lit, val);
	}
}

void VarDatabase::DecayAppearances() {
	if(true) return; // temporary disabled.
	for(int& app : apps_) {
		app *= 0.8;
	}
}

void VarDatabase::RemoveFree(int var) {
	free_vars_.erase({activity_[var], var});
	is_free_[var] = 0;
	num_free_--;
}

void VarDatabase::BumpActivity(int var, int value) {
	if(IsVarFree(var)) {
		RemoveFree(var);
		activity_[var] += value;
		AddToFreeVars(var);
	}
	else {
		activity_[var] += value;
	}
}

void VarDatabase::BumpActivity(RefClause clause, int value) {
	for(Literal& lit : clause->lits_) {
		BumpActivity(lit.var(), value);
	}
}

bool VarDatabase::IsVarFree(int var) {
	return is_free_[var] == 1;
}


int VarDatabase::GetNumFree() {
	return num_free_;
}

void VarDatabase::DecayActivities() {
	bool rescale = false;

	for(int var = 0; var < num_vars_; var++) {
		if(IsVarFree(var)) {
			RemoveFree(var);
			activity_[var] *= decay_factor_;
			AddToFreeVars(var);
		}
		else {
			activity_[var] *= decay_factor_;
		}
		if(activity_[var] > max_activity_) rescale = true;
	}

	if(rescale) {
		for(int var = 0; var < num_vars_; var++) {
			if(IsVarFree(var)) {
				RemoveFree(var);
				activity_[var] *= rescale_factor_;
				AddToFreeVars(var);
			}
			else {
				activity_[var] *= rescale_factor_;
			}
		}		
	}
}

void VarDatabase::AdjustDecay(int diff_levels) {
	diffs_queue_.push(diff_levels);
	sum_diffs_ += diff_levels;
	if(diffs_queue_.size() > max_diff_queue_size_) {
		sum_diffs_ -= diffs_queue_.front();
		diffs_queue_.pop();
	}
	avg_diffs_ = sum_diffs_ / diffs_queue_.size();

	if(avg_diffs_ <= max_avg_diff_) {
		decay_factor_ = decay_factor_slow_;
	}
	else {
		decay_factor_ = decay_factor_fast_;
	}
}