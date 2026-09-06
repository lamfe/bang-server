#include "helena.h"

#include "effects/wildwestshow/helena_zontero.h"

namespace banggame {

    void equip_helena::on_enable(card_ptr target_card, player_ptr target) {
        equip_helena_zontero{}.on_enable(target_card, target);
    }

}
