#include "cpu/exec/helper.h"

#define EFLAGS cpu.eflags

#define CC a
#define CONDITION !EFLAGS.ZF && !EFLAGS.CF
#include "jcc-template0.h"
make_helper_v(concat3(j,CC,_rel))
#undef CONDITION
#undef CC


#define CC ae
#define CONDITION  !EFLAGS.CF
#include "jcc-template0.h"
make_helper_v(concat3(j,CC,_rel))
#undef CONDITION
#undef CC


#define CC b
#define CONDITION  EFLAGS.CF
#include "jcc-template0.h"
make_helper_v(concat3(j,CC,_rel))
#undef CONDITION
#undef CC


#define CC be
#define CONDITION  EFLAGS.ZF || EFLAGS.CF
#include "jcc-template0.h"
make_helper_v(concat3(j,CC,_rel))
#undef CONDITION
#undef CC


#define CC g
#define CONDITION !EFLAGS.ZF && EFLAGS.SF == EFLAGS.OF
#include "jcc-template0.h"
make_helper_v(concat3(j,CC,_rel))
#undef CONDITION
#undef CC


#define CC ge
#define CONDITION  EFLAGS.SF == EFLAGS.OF
#include "jcc-template0.h"
make_helper_v(concat3(j,CC,_rel))
#undef CONDITION
#undef CC


#define CC e
/*ji de shan chu kai shi xie de je*/
#define CONDITION  EFLAGS.ZF
#include "jcc-template0.h"
make_helper_v(concat3(j,CC,_rel))
#undef CONDITION
#undef CC


#define CC l
#define CONDITION  EFLAGS.SF!= EFLAGS.OF
#include "jcc-template0.h"
make_helper_v(concat3(j,CC,_rel))
#undef CONDITION
#undef CC


#define CC le
#define CONDITION   EFLAGS.ZF || EFLAGS.SF !=EFLAGS.OF
#include "jcc-template0.h"
make_helper_v(concat3(j,CC,_rel))
#undef CONDITION
#undef CC


#define CC o
#define CONDITION  EFLAGS.OF 
#include "jcc-template0.h"
make_helper_v(concat3(j,CC,_rel))
#undef CONDITION
#undef CC


#define CC no
#define CONDITION !EFLAGS.OF 
#include "jcc-template0.h"
make_helper_v(concat3(j,CC,_rel))
#undef CONDITION
#undef CC


#define CC p
#define CONDITION  EFLAGS.PF 
#include "jcc-template0.h"
make_helper_v(concat3(j,CC,_rel))
#undef CONDITION
#undef CC

#define CC ne
#define CONDITION !EFLAGS.ZF 
#include "jcc-template0.h"
make_helper_v(concat3(j,CC,_rel))
#undef CONDITION
#undef CC

#define CC s
#define CONDITION   EFLAGS.SF 
#include "jcc-template0.h"
make_helper_v(concat3(j,CC,_rel))
#undef CONDITION
#undef CC


#define CC pe
#define CONDITION   EFLAGS.PF 
#include "jcc-template0.h"
make_helper_v(concat3(j,CC,_rel))
#undef CONDITION
#undef CC


#define CC po
#define CONDITION  !EFLAGS.PF 
#include "jcc-template0.h"
make_helper_v(concat3(j,CC,_rel))
#undef CONDITION
#undef CC


#define CC ns
#define CONDITION !EFLAGS.SF 
#include "jcc-template0.h"
make_helper_v(concat3(j,CC,_rel))
#undef CONDITION
#undef CC



#undef EFLAGS
