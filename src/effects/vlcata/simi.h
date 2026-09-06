#ifndef __VLCATA_SIMI_H__
#define __VLCATA_SIMI_H__

#include "cards/card_effect.h"

namespace banggame {
    struct equip_simi_dynamite_master : event_equip {
        void on_enable(card_ptr target_card, player_ptr target);
    };
    DEFINE_EQUIP(simi_dynamite_master, equip_simi_dynamite_master)
}

#endif
