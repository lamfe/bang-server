#include "vudce.h"

#include "game/game_table.h"

#include "utils/random_element.h"
#include "utils/range_utils.h"

namespace banggame {

    void effect_vudce_ability::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        card_ptr named = random_element(origin->m_game->m_deck, origin->m_game->rng);

        auto it = rn::find(target->m_hand, named->name, &card::name);
        origin->m_game->add_log("LOG_CARD_HAS_EFFECT", origin_card);

        if (it != target->m_hand.end()) {
            target->discard_card(*it);
        } else {
            target->reveal_hand();
        }
    }

}
