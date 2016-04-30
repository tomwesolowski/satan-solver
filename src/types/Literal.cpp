#include "Literal.h"

Literal::Literal() {
	sgn = kUndefined;
}

Literal::Literal(int v, int s) {
  varid = v;
  sgn = s;
}

int Literal::var() const { 
	return varid;
}

int Literal::sign() const { 
	return sgn;
}

int Literal::index() const { 
	return 2 * varid + (sign() != kPositive);
}

Literal Literal::operator-() {
	assert(sign() != kUndefined);
  return Literal(var(), (sign() == kNegative ? kPositive : kNegative));
}

bool Literal::operator<(const Literal& lit) const {
	if(var() != lit.var()) {
		return var() < lit.var();
	}
	return sign() < lit.sign();
}

bool Literal::operator==(const Literal& lit) const {
	return var() == lit.var() && sign() == lit.sign();
}