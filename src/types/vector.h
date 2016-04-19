template <typename T>
class Vector {
  vector<T> vec;

 public:
  Vector() {}

  Vector(vector<T>& v) { set(v); }

  void push_back(T x) { vec.pb(x); }

  void pop_back() { vec.pop_back(); }

  T back() { return vec.back(); }

  T front() { return vec.front(); }

  T& operator[](int i) {
    if (i <= 0) {
      throw "Vector index cannot be equal or less 0.";
    }
    return vec[i - 1];
  }

  void clear() { vec.clear(); }

  void copyTo(Vector& v) { v.set(vec); }

  void set(vector<T>& v) { vec = v; }

  void resize(int sz) { vec.resize(sz); }

  int size() { return vec.size(); }
};