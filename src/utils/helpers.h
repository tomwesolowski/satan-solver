void Assert(bool cond, string msg) {
	if(!cond) {
		cerr << msg << endl;
		assert(false);
	}
}