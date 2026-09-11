#include "helena.h"

#include <set>

#include "cards/bang_cards.h"
#include "cards/filter_enums.h"

#include "game/game_table.h"
#include "game/game_options.h"
#include "game/possible_to_play.h"
#include "game/play_verify.h"
#include "effects/base/pick.h"
#include "effects/base/resolve.h"

#include "utils/range_utils.h"

namespace banggame {

    static card_ptr make_representative(player_ptr origin, const card_data &data) {
        card_ptr c = origin->m_game->add_card(data);
        origin->m_game->add_cards_to({c}, pocket_type::selection, nullptr, card_visibility::shown);
        return c;
    }

    static card_ptr make_representative_by_name(player_ptr origin, std::string_view name) {
        auto it = rn::find_if(bang_cards.deck, [&](const card_data &d) { return d.name == name; });
        if (it == bang_cards.deck.end()) return nullptr;
        return make_representative(origin, *it);
    }

    static bool card_expansion_enabled(player_ptr origin, const card_data &data) {
        for (ruleset_ptr r : data.expansion) {
            if (!origin->m_game->m_options.expansions.contains(r)) return false;
        }
        return true;
    }

    // Every choosable card is a freshly-spawned representative sitting in the shared
    // selection pool, the same way Emporio (General Store) presents its choices.
    static card_list build_nameable_cards(player_ptr origin, card_list &dummies) {
        static constexpr std::string_view base_names[] = {
            "BANG", "BEER", "DUEL", "PANIC", "CAT_BALOU", "GENERAL_STORE", "GATLING",
            "INDIANS", "SALOON", "WELLS_FARGO", "STAGECOACH", "JAIL", "DYNAMITE",
            "VOLCANIC", "SCHOFIELD", "REMINGTON", "REV_CARABINE", "WINCHESTER",
            "SCOPE", "MUSTANG"
        };

        std::set<std::string_view> added;
        card_list choices;

        for (std::string_view name : base_names) {
            if (card_ptr c = make_representative_by_name(origin, name)) {
                choices.push_back(c);
                dummies.push_back(c);
                added.insert(name);
            }
        }

        // Every enabled-expansion card that can normally be played -- a brown card with an
        // effects: entry, or any blue/green equip-type card -- becomes nameable too.
        for (const card_data &data : bang_cards.deck) {
            if (!data.expansion.empty() && !added.contains(data.name)
                && card_expansion_enabled(origin, data)
                && ((data.is_brown() && !data.effects.empty()) || data.is_blue() || data.is_green()))
            {
                card_ptr c = make_representative(origin, data);
                choices.push_back(c);
                dummies.push_back(c);
                added.insert(data.name);
            }
        }

        return choices;
    }

    struct request_helena_no_card : request_dismissable {
        request_helena_no_card(card_ptr origin_card, player_ptr origin, player_ptr hand_target)
            : request_dismissable(origin_card, origin, origin), hand_target(hand_target) {}

        player_ptr hand_target;

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_NAMED_CARD_NOT_FOUND", hand_target};
            } else {
                return {"STATUS_NAMED_CARD_NOT_FOUND_OTHER", target, hand_target};
            }
        }
    };

    static void force_play_named_card(player_ptr hand_target, card_ptr chosen_card) {
        game_ptr g = hand_target->m_game;
        player_ptr old_playing = g->m_playing;
        g->m_playing = hand_target;

        auto matches = generate_playable_cards_list(hand_target)
            | rv::filter([chosen_card](const playable_card_info &info) {
                return info.card == chosen_card;
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
                mod_card->get_modifier(mod_response).add_context(mod_card, hand_target, ctx);
                for (const effect_holder &holder : mod_card->get_effect_list(mod_response)) {
                    const auto &t = mod_targets.emplace_back(holder.random_target(mod_card, hand_target, ctx));
                    holder.add_context(mod_card, hand_target, t, ctx);
                }
            }

            for (const effect_holder &holder : chosen.card->get_effect_list(chosen.effect_list)) {
                const auto &t = action.targets.emplace_back(holder.random_target(chosen.card, hand_target, ctx));
                holder.add_context(chosen.card, hand_target, t, ctx);
            }
            action.bypass_prompt = true;

            verify_and_play(hand_target, action);
        }

        g->queue_action([g, old_playing]{
            g->m_playing = old_playing;
        }, -1000);
    }

    struct request_helena_name : request_picking {
        request_helena_name(card_ptr origin_card, player_ptr origin, player_ptr hand_target, card_list choices, card_list dummies)
            : request_picking(origin_card, nullptr, origin)
            , hand_target(hand_target), choices(std::move(choices)), dummies(std::move(dummies)) {}

        player_ptr hand_target;
        card_list choices;
        card_list dummies;

        bool can_pick(card_ptr target_card) const override {
            return rn::contains(choices, target_card);
        }

        void on_pick(card_ptr picked) override {
            pop_request();
            std::string_view picked_name = picked->name;
            target->m_game->remove_cards(dummies);

            auto it = rn::find(hand_target->m_hand, picked_name, &card::name);
            if (it == hand_target->m_hand.end()) {
                target->m_game->queue_request<request_helena_no_card>(origin_card, target, hand_target);
                return;
            }

            force_play_named_card(hand_target, *it);
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_KIT_CARLSON", origin_card};
            } else {
                return {"STATUS_KIT_CARLSON_OTHER", target, origin_card};
            }
        }
    };

    void effect_helena_ability::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        card_list dummies;
        card_list choices = build_nameable_cards(origin, dummies);

        origin->m_game->add_log("LOG_CARD_HAS_EFFECT", origin_card);
        origin->m_game->queue_request<request_helena_name>(origin_card, origin, target, std::move(choices), std::move(dummies));
    }

}
