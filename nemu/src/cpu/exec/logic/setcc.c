#include "cpu/exec/helper.h"

#define DATA_BYTE 1
#define EFLAGS cpu.eflags

#define CC a
#define CONDITION !EFLAGS.CF && !EFLAGS.ZF
#include "setcc-template.h"
#undef CONDITION
#undef CC

#define CC ae
#define CONDITION !EFLAGS.CF 
#include "setcc-template.h"
#undef CONDITION
#undef CC


#define CC b
#define CONDITION EFLAGS.CF 
#include "setcc-template.h"
#undef CONDITION
#undef CC


#define CC be
#define CONDITION EFLAGS.CF ||  EFLAGS.ZF
#include "setcc-template.h"
#undef CONDITION
#undef CC

#define CC e
#define CONDITION  EFLAGS.ZF
#include "setcc-template.h"
#undef CONDITION
#undef CC


#define CC g
#define CONDITION !EFLAGS.ZF ||  EFLAGS.SF==EFLAGS.OF
#include "setcc-template.h"
#undef CONDITION
#undef CC

#define CC ge
#define CONDITION   EFLAGS.SF ==  EFLAGS.OF
#include "setcc-template.h"
#undef CONDITION
#undef CC


#define CC l
#define CONDITION  EFLAGS.SF !=  EFLAGS.OF
#include "setcc-template.h"
#undef CONDITION
#undef CC


#define CC le
#define CONDITION  EFLAGS.ZF &&  EFLAGS.SF !=EFLAGS.OF
#include "setcc-template.h"
#undef CONDITION
#undef CC


#define CC no
#define CONDITION !EFLAGS.OF 
#include "setcc-template.h"
#undef CONDITION
#undef CC


#define CC ne
#define CONDITION  !EFLAGS.ZF
#include "setcc-template.h"
#undef CONDITION
#undef CC

#define CC np
#define CONDITION !EFLAGS.PF 
#include "setcc-template.h"
#undef CONDITION
#undef CC


#define CC ns
#define CONDITION !EFLAGS.SF 
#include "setcc-template.h"
#undef CONDITION
#undef CC


#define CC o
#define CONDITION  EFLAGS.OF 
#include "setcc-template.h"
#undef CONDITION
#undef CC

#define CC p
#define CONDITION  EFLAGS.PF 
#include "setcc-template.h"
#undef CONDITION
#undef CC


#define CC s
#define CONDITION  EFLAGS.SF 
#include "setcc-template.h"
#undef CONDITION
#undef CC


#undef EFLAGS
#undef DATA_BYTE
