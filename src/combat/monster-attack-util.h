﻿#pragma once

#include "system/angband.h"

/* MONster-Attack-Player、地図のMAPと紛らわしいのでmonapとした */
typedef struct monap_type {
#ifdef JP
    int abbreviate; // 2回目以降の省略表現フラグ.
#endif
    MONSTER_IDX m_idx;
    monster_type *m_ptr;
    concptr act;
} monap_type;

monap_type *initialize_monap_type(player_type *target_ptr, monap_type *monap_ptr, MONSTER_IDX m_idx);
