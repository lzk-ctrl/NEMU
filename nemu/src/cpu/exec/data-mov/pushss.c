#include "cpu/exec/helper.h"

#define DATA_BYTE 1
#include "pushss-template.h"
#undef DATA_BYTE

#define DATA_BYTE 2
#include "pushss-template.h"
#undef DATA_BYTE

#define DATA_BYTE 4
#include "pushss-template.h"
#undef DATA_BYTE

make_helper_v(pushss_si)