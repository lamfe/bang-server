#include "termit.h"

#include "cards/game_events.h"
#include "effects/base/generalstore.h"

#include "game/game_table.h"

namespace banggame {

    void equip_termit::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::count_initial_cards>(target_card, [target](const_player_ptr origin) -> std::optional<int> {
            if (origin != target) return std::nullopt;

            auto &players = target->m_game->m_players;
            size_t n = players.size();
            size_t first_idx = n;
            for (size_t i = 0; i < n; ++i) {
                if (players[i] == target->m_game->m_first_player) {
                    first_idx = i;
                    break;
                }
            }
            if (first_idx == n || n < 2) return std::nullopt;

            size_t last_idx = (first_idx + n - 1) % n;
            size_t second_last_idx = (first_idx + n - 2) % n;
            if (players[last_idx] == target || players[second_last_idx] == target) {
                return target->m_hp + 1;
            }
            return std::nullopt;
        });

        target->m_game->add_listener<event_type::on_play_card>(target_card, [target, target_card](player_ptr origin, card_ptr played_card, const effect_context &ctx) {
            if (played_card->name == "GENERAL_STORE" && target->alive()) {
                target_card->flash_card();
                target->m_game->top_of_deck()->move_to(pocket_type::selection);
                target->draw_card(1, target_card);
            }
        });

        target->m_game->add_listener<event_type::apply_maxcards_modifier>(target_card, [target](const_player_ptr origin, int &value) {
            if (origin == target && target->m_hp < 3) {
                ++value;
            }
        });
    }

}
