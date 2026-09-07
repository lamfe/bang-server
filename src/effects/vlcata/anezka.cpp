#include "anezka.h"

#include "game/game_table.h"

#include <set>

namespace banggame {

    void equip_anezka::on_enable(card_ptr target_card, player_ptr target) {
        auto healed_players = std::make_shared<std::set<player_ptr>>();

        target->m_game->add_listener<event_type::check_anezka_can_heal>(target_card, [target, healed_players](player_ptr origin, player_ptr e_target) -> bool {
            if (origin != target || e_target == origin) return true;
            return !healed_players->contains(e_target);
        });

        target->m_game->add_listener<event_type::mark_anezka_healed>(target_card, [target, healed_players](player_ptr origin, player_ptr e_target) {
            if (origin == target && e_target != origin) {
                healed_players->insert(e_target);
            }
        });
    }

    game_string effect_anezka_heal::get_error(card_ptr origin_card, player_ptr origin, player_ptr target) {
        if (!origin->m_game->call_event(event_type::check_anezka_can_heal{origin, target})) {
            return "ERROR_ANEZKA_ALREADY_HEALED";
        }
        return {};
    }

    void effect_anezka_heal::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        target->heal(origin_card, origin, 1);
        origin->m_game->call_event(event_type::mark_anezka_healed{origin, target});
    }

}
