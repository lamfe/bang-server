#include "budha.h"

#include "cards/filter_enums.h"
#include "cards/game_events.h"

#include "effects/base/pick.h"

#include "game/game_table.h"
#include "game/possible_to_play.h"
#include "game/play_verify.h"

#include "utils/range_utils.h"

namespace banggame {

    static bool is_alcohol_card(card_ptr c) {
        return c->has_tag(tag_type::beer)
            || c->name == "WHISKY"
            || c->name == "TEQUILA"
            || c->name == "BEER_KEG";
    }

    // Forces one specific revealed card into play with computed targeting, the same way
    // Helena/Vudce force a named card out of a hand -- reused here so Budha's picked card
    // gets played immediately instead of sitting in his hand for later, manual use.
    static void force_play_revealed_card(player_ptr target, card_ptr picked) {
        if (!target->alive()) {
            target->m_game->remove_cards({picked});
            return;
        }

        game_ptr g = target->m_game;
        picked->move_to(pocket_type::player_hand, target);

        player_ptr old_playing = g->m_playing;
        g->m_playing = target;

        auto matches = generate_playable_cards_list(target)
            | rv::filter([picked](const playable_card_info &info) {
                return info.card == picked;
            })
            | rn::to<std::vector>();

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

            verify_and_play(target, action);
        } else if (picked->owner == target) {
            target->discard_card(picked);
        }

        g->queue_action([g, old_playing]{
            g->m_playing = old_playing;
        }, -1000);
    }

    struct request_budha_play : request_picking {
        request_budha_play(card_ptr origin_card, player_ptr target, card_list cards)
            : request_picking(origin_card, nullptr, target), cards(std::move(cards)) {}

        card_list cards;

        bool can_pick(card_ptr target_card) const override {
            return rn::contains(cards, target_card);
        }

        void on_pick(card_ptr picked) override {
            pop_request();

            card_list remaining = cards;
            std::erase(remaining, picked);
            if (!remaining.empty()) {
                target->m_game->queue_request<request_budha_play>(origin_card, target, remaining);
            }

            force_play_revealed_card(target, picked);
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_KIT_CARLSON", origin_card};
            } else {
                return {"STATUS_KIT_CARLSON_OTHER", target, origin_card};
            }
        }
    };

    // Reveals 3 cards; whatever isn't currently playable is discarded on the spot, and
    // whatever remains playable is queued for Budha to pick the play order of (each pick
    // is immediately force-played for real, possibly chaining into another reveal of 3 if
    // it's itself alcohol). Nothing is left sitting in his hand for later, manual play.
    static void budha_flip_three(player_ptr target, card_ptr origin_card) {
        card_list revealed;
        for (int i = 0; i < 3 && target->alive(); ++i) {
            card_ptr drawn = target->m_game->top_of_deck();
            drawn->move_to(pocket_type::selection, target);
            revealed.push_back(drawn);
        }
        if (!target->alive()) return;

        game_ptr g = target->m_game;
        player_ptr old_playing = g->m_playing;
        g->m_playing = target;

        card_list playable;
        for (card_ptr c : revealed) {
            c->move_to(pocket_type::player_hand, target);

            bool ok = rn::any_of(generate_playable_cards_list(target), [c](const playable_card_info &info) {
                return info.card == c;
            });

            if (ok) {
                c->move_to(pocket_type::selection, target);
                playable.push_back(c);
            } else {
                target->discard_card(c);
            }
        }

        // If any of the playable cards is itself alcohol, only it gets resolved (it'll
        // chain into another reveal of 3) -- the other playable cards from this reveal are
        // discarded instead of being played alongside it.
        if (rn::any_of(playable, is_alcohol_card)) {
            card_list alcohol_only;
            for (card_ptr c : playable) {
                if (is_alcohol_card(c)) {
                    alcohol_only.push_back(c);
                } else {
                    target->discard_card(c);
                }
            }
            playable = std::move(alcohol_only);
        }

        g->queue_action([g, old_playing]{
            g->m_playing = old_playing;
        }, -1000);

        if (!playable.empty()) {
            g->queue_request<request_budha_play>(origin_card, target, playable);
        }
    }

    void equip_budha::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_play_card>(target_card, [target, target_card](player_ptr origin, card_ptr played_card, const effect_context &ctx) {
            if (origin == target && is_alcohol_card(played_card)) {
                target_card->flash_card();
                budha_flip_three(target, target_card);
            }
        });
    }

}
