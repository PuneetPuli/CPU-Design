#ifndef BRANCHSIM_H
#define BRANCHSIM_H

/*#include <inttypes.h>*/
#include <stdint.h>

typedef struct branch_stats_t {
    uint64_t num_branches;
    uint64_t pred_taken;
    uint64_t pred_not_taken;
    uint64_t correct;
    double misprediction_rate;
    uint64_t storage_overhead;
} branch_stats_t;

typedef enum predictor_type {
    PTYPE_BIMODAL = 'B',
    PTYPE_GSHARE = 'G',
    PTYPE_LOCAL_HISTORY = 'L',
    PTYPE_TWO_LEVEL_ADAPTIVE = 'T',
} predictor_type;

typedef enum branch_dir {
    TAKEN = 'T',
    NOT_TAKEN = 'N',
} branch_dir;

void setup_predictor(predictor_type ptype, int num_entries, int counter_bits, int history_bits,
    branch_stats_t* p_stats);
branch_dir predict_branch(uint64_t pc, struct branch_stats_t* p_stats);
void update_predictor(uint64_t pc, branch_dir actual, branch_dir predicted, branch_stats_t* p_stats);
void complete_predictor(branch_stats_t* p_stats);

#endif /* BRANCHSIM_H */
