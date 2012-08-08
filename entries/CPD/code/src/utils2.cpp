

#include "utils2.h"

int getQuarter(int target_col, int target_row, int origin_col, int origin_row) {
    if (target_col <= origin_col) {
        if (target_row <= origin_row)
            return 0;
        else
            return 3;
    } else {
        if (target_row <= origin_row)
            return 1;
        else
            return 2;
    }
}
