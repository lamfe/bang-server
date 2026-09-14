#include "simi.h"

#include "cards/game_enums.h"
#include "effects/base/damage.h"

#include "game/game_table.h"

#include "utils/range_utils.h"

namespace banggame {

    void equip_simi_dynamite_master::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_hit>(target_card, [target, target_card](card_ptr origin_card, player_ptr origin, player_ptr hit_target, int damage, effect_flags flags) {
            if (hit_target == target && damage > 0 && origin_card && origin_card->name == "DYNAMITE") {
                target_card->flash_card();
                target->heal(target_card, nullptr, damage);
            }
        });

        // Only take over as sheriff during genuine initial game setup (m_playing is still
        // null then, before the very first turn starts) -- not when someone ends up holding
        // the Simi identity mid-game instead, e.g. via New Identity reassigning it to a
        // different player, or Vera Custer temporarily copying it. Re-running the reshuffle
        // in either of those cases would reshuffle everyone's roles all over again.
        if (target->m_game->m_playing != nullptr) return;

        // Runs as a high-priority setup action, queued here during character selection but
        // not actually executed until the whole setup routine (all characters picked, all
        // HP set to their pre-swap max) has returned, and strictly before the default-
        // priority action that deals out everyone's starting hand. So the role swap and the
        // HP fixup below both land before anyone's hand size is ever computed -- Simi and
        // whoever gets demoted both start the game with their final, correct HP/hand size,
        // instead of getting corrected after the fact.
        target->m_game->queue_action([target, target_card] {
            game_ptr g = target->m_game;

            // No sheriff role exists in 3-player games, so the ability doesn't apply
            if (g->m_players.size() <= 3) return;

            auto other_players = rv::filter(g->m_players, [target](player_ptr p) {
                return p->alive() && p != target;
            }) | rn::to<std::vector>();

            // Roles are drawn from ALL alive players (Simi included), with exactly one
            // "sheriff" entry removed from the pool -- whoever actually held it, Simi or
            // not -- so the other players are reshuffled among the remaining non-sheriff
            // roles and Simi becoming sheriff never produces a second sheriff.
            auto all_alive = rv::filter(g->m_players, &player::alive) | rn::to<std::vector>();
            auto roles = all_alive | rv::transform(&player::get_base_role) | rn::to<std::vector>();
            if (auto it = rn::find(roles, player_role::sheriff); it != roles.end()) {
                roles.erase(it);
            }
            rn::shuffle(roles, g->rng);

            target->hide_role();
            target->set_role(player_role::sheriff);

            for (auto [p, role] : rv::zip(other_players, roles)) {
                p->hide_role();
                p->set_role(role);
            }

            // Only the sheriff role affects max HP, so this is a no-op for everyone whose
            // role change wasn't sheriff <-> something-else, and correctly fixes both Simi
            // (now sheriff) and whoever just got demoted from it.
            for (player_ptr p : all_alive) {
                p->m_max_hp = p->get_character_max_hp();
                p->set_hp(p->m_max_hp, true);
            }

            g->add_log("LOG_SIMI_BECOMES_SHERIFF", target);
        }, 200);
    }

    bool effect_simi_take_dynamite::can_play(card_ptr origin_card, player_ptr origin) {
        return rn::any_of(origin->m_game->m_discards, [](card_ptr c) { return c->name == "DYNAMITE"; });
    }

    void effect_simi_take_dynamite::on_play(card_ptr origin_card, player_ptr origin) {
        auto it = rn::find_if(origin->m_game->m_discards, [](card_ptr c) { return c->name == "DYNAMITE"; });
        if (it != origin->m_game->m_discards.end()) {
            card_ptr dynamite = *it;
            origin->m_game->add_log("LOG_EQUIPPED_CARD", dynamite, origin);
            origin->equip_card(dynamite);
        }
    }

}
