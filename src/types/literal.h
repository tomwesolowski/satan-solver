#ifndef LITERAL_H
#define LITERAL_H

struct Literal {
  int varid;
  int sgn;

  Literal() {}

  Literal(int v, int s = kUndefined) {
    varid = abs(v);
    sgn = s;
  }

  int var() const { 
  	return varid;
  }

  int sign() const { 
  	return sgn;
  }

  Literal operator-() {
    return Literal(var(), (sign() == kNegative ? kPositive : kNegative));
  }

  bool operator<(const Literal& lit) const {
  	if(var() != lit.var()) {
  		return var() < lit.var();
  	}
  	return sign() < lit.sign();
  }

  bool operator==(const Literal& lit) const {
  	return var() == lit.var() && sign() == lit.sign();
  }
};

#endif