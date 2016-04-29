#ifndef MODEL_H
#define MODEL_H

#include "constants.h"

class Model {
public:
  int varid;
  int sgn;

  Model();

  Literal(int v, int s = kUndefined);

  int var() const;

  int sign() const;

  int index() const;

  Literal operator-();

  bool operator<(const Literal& lit) const;

  bool operator==(const Literal& lit) const;
};

#endif