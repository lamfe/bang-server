#include "helena.h"

#include "game/game_table.h"
#include "game/possible_to_play.h"
#include "game/play_verify.h"

#include "utils/random_element.h"
#include "utils/range_utils.h"

namespace banggame {

    void effect_helena_ability::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        game_ptr g = target->m_game;
        player_ptr old_playing = g->m_playing;
        g->m_playing = target;

        auto hand_cards = generate_playable_cards_list(target)
            | rv::filter([target](const playable_card_info &info) {
                return info.card->owner == target && info.card->pocket == pocket_type::player_hand;
            })
            | rn::to<std::vector>();

        if (!hand_cards.empty()) {
            const playable_card_info &chosen = random_element(hand_cards, g->rng);

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

            origin->m_game->add_log("LOG_CARD_HAS_EFFECT", origin_card);
            verify_and_play(target, action);
        }

        g->queue_action([g, old_playing]{
            g->m_playing = old_playing;
        }, -1000);
    }

}
