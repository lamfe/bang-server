#include "budha.h"

#include "cards/filter_enums.h"
#include "cards/game_events.h"

#include "game/game_table.h"
#include "game/possible_to_play.h"
#include "game/play_verify.h"

#include "utils/misc.h"
#include "utils/range_utils.h"

namespace banggame {

    static bool is_alcohol_card(card_ptr c) {
        return c->has_tag(tag_type::beer) || c->name == "WHISKY" || c->name == "TEQUILA";
    }

    static void budha_flip_three(player_ptr target, int depth) {
        if (depth > 5) return;

        for (int i = 0; i < 3 && target->alive(); ++i) {
            card_ptr drawn = target->m_game->top_of_deck();
            drawn->move_to(pocket_type::player_hand, target);

            auto matches = generate_playable_cards_list(target)
                | rv::filter([drawn](const playable_card_info &info) {
                    return info.card == drawn;
                })
                | rn::to<std::vector>();

            bool played = false;
            if (!matches.empty()) {
                const playable_card_info &chosen = matches.front();

                game_action action{
                    .card = chosen.card,
                    .effect_list = chosen.effect_list
                };
                effect_context ctx;

                for (const auto &[mod_card, mod_response] : chosen.modifiers) {
                    auto &mod_targets = action.modifiers.emplace_back(mod_card, mod_response).targets;
                    mod_card->get_modifier(mod_response).add_context(mod_card, target, ctx);
                    for (const effect_holder &holder : mod_card->get_effect_list(mod_response)) {
                        const auto &t = mod_targets.emplace_back(holder.random_target(mod_card, target, ctx));
                        holder.add_context(mod_card, target, t, ctx);
                    }
                }

                for (const effect_holder &holder : chosen.card->get_effect_list(chosen.effect_list)) {
                    const auto &t = action.targets.emplace_back(holder.random_target(chosen.card, target, ctx));
                    holder.add_context(chosen.card, target, t, ctx);
                }
                action.bypass_prompt = true;

                auto result = verify_and_play(target, action);
                played = std::visit(overloaded{
                    [](play_verify_results::ok) { return true; },
                    [](play_verify_results::prompt) { return false; },
                    [](play_verify_results::error) { return false; }
                }, result);
            }

            if (!played) {
                if (drawn->owner == target && drawn->pocket == pocket_type::player_hand) {
                    target->discard_card(drawn);
                }
            } else if (is_alcohol_card(drawn)) {
                budha_flip_three(target, depth + 1);
            }
        }
    }

    void equip_budha::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_play_card>(target_card, [target, target_card](player_ptr origin, card_ptr played_card, const effect_context &ctx) {
            if (origin == target && is_alcohol_card(played_card)) {
                target_card->flash_card();

                game_ptr g = target->m_game;
                player_ptr old_playing = g->m_playing;
                g->m_playing = target;

                budha_flip_three(target, 0);

                g->queue_action([g, old_playing]{
                    g->m_playing = old_playing;
                }, -1000);
            }
        });
    }

}
