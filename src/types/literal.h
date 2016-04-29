#ifndef LITERAL_H
#define LITERAL_H

#include "constants.h"

struct Literal {
  int varid;
  int sgn;

  Literal();

  Literal(int v, int s = kUndefined);

  int var() const;

  int sign() const;

  int index() const;

  Literal operator-();

  bool operator<(const Literal& lit) const;

  bool operator==(const Literal& lit) const;
};

#endif