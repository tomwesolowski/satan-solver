#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <bits/stdc++.h>

using namespace std;

const int kUndefined = 0;
const int kPositive = 1;
const int kNegative = -1;

const int kUnsatisfiableState = -1;
const int kSatisfiableState = 1;
const int kUnknownState = 0;

const int kUnitClause = 0;
const int kUnsatisfiableClause = 1;
const int kReleaseWatcher = 2;
const int kHoldWatcher = 3;

typedef pair<int, int> VarAssignment; // <var, both_ways>

#endif 
//CONSTANTS_H