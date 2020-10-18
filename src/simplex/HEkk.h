/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                       */
/*    This file is part of the HiGHS linear optimization suite           */
/*                                                                       */
/*    Written and engineered 2008-2020 at the University of Edinburgh    */
/*                                                                       */
/*    Available as open-source under the MIT License                     */
/*                                                                       */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/**@file simplex/HEkk.h
 * @brief Phase 2 primal simplex solver for HiGHS
 * @author Julian Hall, Ivet Galabova, Qi Huangfu and Michael Feldmeier
 */
#ifndef SIMPLEX_HEKK_H_
#define SIMPLEX_HEKK_H_

#include "HConfig.h"
#include "simplex/HSimplex.h"
#include "simplex/HVector.h"

using std::pair;

/**
 * @brief Primal simplex solver for HiGHS
 *
 */

struct HighsEkkStatus {
  // Status of LP solved by the simplex method and its data
  bool valid = false;
  bool is_dualised = false;
  bool is_permuted = false;
  bool scaling_tried = false;
  bool has_basis = false;            // The simplex LP has a valid simplex basis
  bool has_matrix_col_wise = false;  // The HMatrix column-wise matrix is valid
  bool has_matrix_row_wise = false;  // The HMatrix row-wise matrix is valid
  bool has_factor_arrays =
      false;  // Has the arrays for the representation of B^{-1}
  bool has_dual_steepest_edge_weights = false;  // The DSE weights are known
  bool has_nonbasic_dual_values = false;  // The nonbasic dual values are known
  bool has_basic_primal_values = false;   // The basic primal values are known
  bool has_invert =
      false;  // The representation of B^{-1} corresponds to the current basis
  bool has_fresh_invert = false;  // The representation of B^{-1} corresponds to
                                  // the current basis and is fresh
  bool has_fresh_rebuild = false;  // The data are fresh from rebuild
  bool has_dual_objective_value =
      false;  // The dual objective function value is known
  bool has_primal_objective_value =
      false;                    // The dual objective function value is known
  bool has_dual_ray = false;    // A dual unbounded ray is known
  bool has_primal_ray = false;  // A primal unbounded ray is known
  SimplexSolutionStatus solution_status =
      SimplexSolutionStatus::UNSET;  // The solution status is UNSET
};

class HEkk {
 public:
 HEkk(HighsLp& lp, SimplexBasis& simplex_basis, HighsOptions& options) :
  lp_(lp), simplex_basis_(simplex_basis), options_(options){}
  /**
   * @brief Solve a model instance
   */
  HighsStatus init();
  HighsStatus solve();

  const SimplexAlgorithm algorithm = SimplexAlgorithm::PRIMAL;

  HighsEkkStatus simplex_lp_status;
  HighsModelStatus model_status;

  HMatrix matrix;
  HFactor factor;

  // Simplex information regarding primal solution, dual solution and
  // objective for this Highs Model Object. This is information which
  // should be retained from one run to the next in order to provide
  // hot starts.
  //
  // Part of working model which are assigned and populated as much as
  // possible when a model is being defined

  // workCost: Originally just costs from the model but, in solve(), may
  // be perturbed or set to alternative values in Phase I??
  //
  // workDual: Values of the dual variables corresponding to
  // workCost. Latter not known until solve() is called since B^{-1}
  // is required to compute them. Knowledge of them is indicated by
  // has_nonbasic_dual_values
  //
  // workShift: Values added to workCost in order that workDual
  // remains feasible, thereby remaining dual feasible in phase 2
  //
  std::vector<double> workCost_;
  std::vector<double> workDual_;
  std::vector<double> workShift_;

  // workLower/workUpper: Originally just lower (upper) bounds from
  // the model but, in solve(), may be perturbed or set to
  // alternative values in Phase I??
  //
  // workRange: Distance between lower and upper bounds
  //
  // workValue: Values of the nonbasic variables corresponding to
  // workLower/workUpper and the basis. Always known.
  //
  std::vector<double> workLower_;
  std::vector<double> workUpper_;
  std::vector<double> workRange_;
  std::vector<double> workValue_;

  // baseLower/baseUpper/baseValue: Lower and upper bounds on the
  // basic variables and their values. Latter not known until solve()
  // is called since B^{-1} is required to compute them. Knowledge of
  // them is indicated by has_basic_primal_values
  //
  std::vector<double> baseLower_;
  std::vector<double> baseUpper_;
  std::vector<double> baseValue_;
  //
  // Vectors of random reals for column cost perturbation, a random
  // permutation of all indices for CHUZR and a random permutation of
  // column indices for permuting the columns
  std::vector<double> numTotRandomValue_;
  std::vector<int> numTotPermutation_;
  std::vector<int> numColPermutation_;

  // Dual and primal ray vectors
  int dual_ray_row_;
  int dual_ray_sign_;
  int primal_ray_col_;
  int primal_ray_sign_;

  int num_primal_infeasibilities = -1;
  double max_primal_infeasibility;
  double sum_primal_infeasibilities;
  int num_dual_infeasibilities = -1;
  double max_dual_infeasibility;
  double sum_dual_infeasibilities;

  // Records of cumulative iteration counts - updated at the end of a phase
  int dual_phase1_iteration_count = 0;
  int dual_phase2_iteration_count = 0;
  int primal_phase1_iteration_count = 0;
  int primal_phase2_iteration_count = 0;

  int min_threads = 1;
  int num_threads = 1;
  int max_threads = HIGHS_THREAD_LIMIT;

  // Value of dual objective - only set when computed from scratch in dual
  // rebuild()
  double dual_objective_value;
  // Value of primal objective - only set when computed from scratch in primal
  // rebuild()
  double primal_objective_value;

 private:
  // References:
  //
  // LP to be solved, HiGHS options to be used
  HighsLp& lp_;
  SimplexBasis& simplex_basis_;
  HighsOptions& options_;

  int solver_num_col;
  int solver_num_row;
  int solver_num_tot;

  HighsSimplexAnalysis* analysis;

  bool no_free_columns;

  int isPrimalPhase1;

  int solvePhase;
  // Pivot related
  int invertHint;
  int columnIn;
  int rowOut;
  int columnOut;
  int phase1OutBnd;
  double thetaDual;
  double thetaPrimal;
  double alpha;
  //  double alphaRow;
  double numericalTrouble;
  int num_flip_since_rebuild;

  // Primal phase 1 tools
  vector<pair<double, int> > ph1SorterR;
  vector<pair<double, int> > ph1SorterT;

  // Devex weight
  int num_devex_iterations;
  int num_bad_devex_weight;
  vector<double> devex_weight;
  vector<int> devex_index;

  // Solve buffer
  HVector row_ep;
  HVector row_ap;
  HVector col_aq;

  // Options from HighsOptions for the simplex solver
  int simplex_strategy;
  int dual_edge_weight_strategy;
  int primal_edge_weight_strategy;
  int price_strategy;

  double dual_simplex_cost_perturbation_multiplier;
  double factor_pivot_threshold;
  int update_limit;

  // Internal options - can't be changed externally
  bool run_quiet = false;
  bool store_squared_primal_infeasibility = false;

  /*
#ifndef HiGHSDEV
  bool analyse_lp_solution = false;  // true;//
#else
  bool analyse_lp_solution = true;
  // Options for reporting timing
  bool report_simplex_inner_clock = false;
  bool report_simplex_outer_clock = false;
  bool report_simplex_phases_clock = false;
  bool report_HFactor_clock = false;
  // Option for analysing the LP simplex iterations, INVERT time and rebuild
  // time
  bool analyse_lp = false;
  bool analyse_iterations = false;
  bool analyse_invert_form = false;
  bool analyse_invert_condition = false;
  bool analyse_invert_time = false;
  bool analyse_rebuild_time = false;
#endif
  */
  // Simplex runtime information
  int allow_cost_perturbation = true;
  int costs_perturbed = 0;

  /*
  // Data for backtracking in the event of a singular basis
  int phase1_backtracking_test_done = false;
  int phase2_backtracking_test_done = false;
  bool backtracking_ = false;
  bool valid_backtracking_basis_ = false;
  SimplexBasis backtracking_basis_;
  int backtracking_basis_costs_perturbed_;
  std::vector<double> backtracking_basis_workShift_;
  std::vector<double> backtracking_basis_edge_weights_;
  */

  /*
#ifdef HiGHSDEV
  // Analysis of INVERT
  int num_invert = 0;
  // Analysis of INVERT form
  int num_kernel = 0;
  int num_major_kernel = 0;
  const double major_kernel_relative_dim_threshold = 0.1;
  double max_kernel_dim = 0;
  double sum_kernel_dim = 0;
  double running_average_kernel_dim = 0;
  double sum_invert_fill_factor = 0;
  double sum_kernel_fill_factor = 0;
  double sum_major_kernel_fill_factor = 0;
  double running_average_invert_fill_factor = 1;
  double running_average_kernel_fill_factor = 1;
  double running_average_major_kernel_fill_factor = 1;

  int total_inverts;
  double total_invert_time;
  double invert_condition = 1;
#endif
  */

  // Number of UPDATE operations performed - should be zeroed when INVERT is
  // performed
  int update_count;
  // Value of dual objective that is updated in dual simplex solver
  double updated_dual_objective_value;
  // Value of primal objective that is updated in primal simplex solver
  double updated_primal_objective_value;
  // Number of logical variables in the basis
  int num_basic_logicals;


  int computeFactor();
  void computePrimal();
  void computeDual();

  void allocateWorkAndBaseArrays();
  void initialisePhase2ColCost();
  void initialisePhase2RowCost();
  void initialiseCost(int perturb = 0);
  void initialisePhase2ColBound();
  void initialisePhase2RowBound();
  void initialiseBound(int phase = 2);
  void initialiseNonbasicWorkValue();

};

#endif /* SIMPLEX_HEKK_H_ */
