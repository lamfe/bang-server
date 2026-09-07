#ifndef __VLCATA_PATKA_H__
#define __VLCATA_PATKA_H__

#include "cards/card_effect.h"

namespace banggame {

    struct effect_patka_discard {
        void on_play(card_ptr origin_card, player_ptr origin);
    };

    DEFINE_EFFECT(patka_discard, effect_patka_discard)

}

#endif
