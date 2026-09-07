#include "vudce.h"

#include "game/game_table.h"

namespace banggame {

    void effect_vudce_ability::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        origin->m_game->add_log("LOG_CARD_HAS_EFFECT", origin_card);
        target->discard_card(target->random_hand_card());
    }

}
