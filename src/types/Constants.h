#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <bits/stdc++.h>

using namespace std;

const int kUndefined = 0;
const int kPositive = 1;
const int kNegative = -1;

const int kUnsatisfiableState = 3;
const int kSatisfiableState = 5;
const int kUnknownState = 6;

const int kUnitClause = 1;
const int kUnsatisfiableClause = 2;
const int kNormalClause = 4;
const int kNewWatcher = 8;
const int kHoldWatcher = 16;

const int kSolutionFound = 1;
const int kSolutionNotFound = 2;
const int kForceRestart = 0;

#endif 
//CONSTANTS_H