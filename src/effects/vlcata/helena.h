#ifndef __VLCATA_HELENA_H__
#define __VLCATA_HELENA_H__

#include "cards/card_effect.h"

namespace banggame {
    struct equip_helena : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };
    DEFINE_EQUIP(helena, equip_helena)
}

#endif
