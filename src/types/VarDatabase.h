#ifndef VAR_DATABASE_H
#define VAR_DATABASE_H

#include <bits/stdc++.h>

class VarDatabase {
public:
	set< pair<int, int> > free_vars_; // <activeness, var>
  vector<int> activeness_; // number of clauses with literal i
};

#endif