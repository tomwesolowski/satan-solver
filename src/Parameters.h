#ifndef PARAMETERS_H
#define PARAMETERS_H

// Here you can set up all parameters of Solver.
// Be careful, they have been chosen after lots of experiments.
// Solver can slow down significantly if you change any of them.
struct SolverParameters {
	// Solver parameters.
  double G = 0.9999;
  int learnt_limit_ = 100;
  int conflicts_limit_ = 250;

  // VarDatabase parameters.
  double decay_factor_ = 0.9;
  double max_activity_ = 1e8;
  double rescale_factor_ = 0.0001;

  int max_diff_queue_size_ = 10;
  int max_avg_diff_ = 7;
  
  double decay_factor_slow_ = 0.99;
  double decay_factor_fast_ = 0.75;
};

#endif // PARAMETERS_H