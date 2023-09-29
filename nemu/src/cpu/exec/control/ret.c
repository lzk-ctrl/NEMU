#include "cpu/exec/helper.h"

//宏定义DATA_BYTE的目的就是为了看着方便
#define DATA_BYTE 1
#include "ret-template.h"
#undef DATA_BYTE

#define DATA_BYTE 2
#include "ret-template.h"
#undef DATA_BYTE

#define DATA_BYTE 4
#include "ret-template.h"
#undef DATA_BYTE

make_helper_v(ret_n) //v后缀：如果是16为则变为w后缀，否则变为l后缀
make_helper_v(ret_i)