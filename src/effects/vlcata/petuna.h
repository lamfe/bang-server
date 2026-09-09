#ifndef __VLCATA_PETUNA_H__
#define __VLCATA_PETUNA_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_petuna_check {
        bool can_play(card_ptr origin_card, player_ptr origin);
        void on_play(card_ptr origin_card, player_ptr origin);
    };
    DEFINE_EFFECT(petuna_check, effect_petuna_check)

}

#endif
