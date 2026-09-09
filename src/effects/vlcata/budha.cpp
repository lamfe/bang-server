#include "budha.h"

#include "cards/filter_enums.h"
#include "cards/game_events.h"

#include "game/game_table.h"
#include "game/possible_to_play.h"

#include "utils/range_utils.h"

namespace banggame {

    static bool is_alcohol_card(card_ptr c) {
        return c->has_tag(tag_type::beer)
            || c->name == "WHISKY"
            || c->name == "TEQUILA"
            || c->name == "BEER_KEG";
    }

    static void budha_flip_three(player_ptr target) {
        card_list revealed;
        for (int i = 0; i < 3 && target->alive(); ++i) {
            card_ptr drawn = target->m_game->top_of_deck();
            drawn->move_to(pocket_type::selection, target);
            revealed.push_back(drawn);
        }

        game_ptr g = target->m_game;
        player_ptr old_playing = g->m_playing;
        g->m_playing = target;

        for (card_ptr c : revealed) {
            if (!target->alive()) break;
            c->move_to(pocket_type::player_hand, target);

            bool playable = rn::any_of(generate_playable_cards_list(target), [c](const playable_card_info &info) {
                return info.card == c;
            });

            if (!playable) {
                target->discard_card(c);
            }
        }

        g->queue_action([g, old_playing]{
            g->m_playing = old_playing;
        }, -1000);
    }

    void equip_budha::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_play_card>(target_card, [target, target_card](player_ptr origin, card_ptr played_card, const effect_context &ctx) {
            if (origin == target && is_alcohol_card(played_card)) {
                target_card->flash_card();
                budha_flip_three(target);
            }
        });
    }

}
