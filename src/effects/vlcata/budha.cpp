#include "budha.h"

#include "cards/filter_enums.h"
#include "cards/game_events.h"

#include "game/game_table.h"

namespace banggame {

    static bool is_alcohol_card(card_ptr c) {
        return c->has_tag(tag_type::beer) || c->name == "WHISKY" || c->name == "TEQUILA";
    }

    void equip_budha::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_play_card>(target_card, [target, target_card](player_ptr origin, card_ptr played_card, const effect_context &ctx) {
            if (origin == target && is_alcohol_card(played_card)) {
                target_card->flash_card();
                for (int i = 0; i < 3 && target->alive(); ++i) {
                    card_ptr drawn = target->m_game->top_of_deck();
                    if (is_alcohol_card(drawn)) {
                        target->m_game->add_log("LOG_CARD_HAS_EFFECT", drawn);
                        drawn->move_to(pocket_type::discard_pile);
                        target->heal(target_card, nullptr, 1);
                    } else {
                        drawn->move_to(pocket_type::discard_pile);
                    }
                }
            }
        });
    }

}
