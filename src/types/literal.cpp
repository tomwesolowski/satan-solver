#include "literal.h"

Literal::Literal() {}

Literal::Literal(int v, int s) {
  varid = abs(v);
  sgn = s;
}

int Literal::var() const { 
	return varid;
}

int Literal::sign() const { 
	return sgn;
}

Literal Literal::operator-() {
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