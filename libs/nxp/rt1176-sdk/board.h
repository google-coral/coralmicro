#ifndef _BOARD_H_
#define _BOARD_H_

#if defined(BOARD_REVISION_EVK)
#include "board_evk.h"
#elif defined(BOARD_REVISION_P0) || defined(BOARD_REVISION_P1)
#include "board_p0.h"
#else
#error "Undefined board revision!"
#endif

#endif  // _BOARD_H_
