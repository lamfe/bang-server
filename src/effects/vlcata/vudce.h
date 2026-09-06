#ifndef __VLCATA_VUDCE_H__
#define __VLCATA_VUDCE_H__

#include "cards/card_effect.h"

namespace banggame {
    struct equip_vudce : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };
    DEFINE_EQUIP(vudce, equip_vudce)
}

#endif
