#ifndef __VLCATA_VERCA_H__
#define __VLCATA_VERCA_H__

#include "cards/card_effect.h"

namespace banggame {
    struct equip_verca_instant_green : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };
    DEFINE_EQUIP(verca_instant_green, equip_verca_instant_green)

    struct effect_verca_discard {
        game_string get_error(card_ptr origin_card, player_ptr origin, card_ptr target_card);
        void on_play(card_ptr origin_card, player_ptr origin, card_ptr target_card);
    };
    DEFINE_EFFECT(verca_discard, effect_verca_discard)
}

#endif