#include "vudce.h"

#include "cards/bang_cards.h"
#include "cards/filter_enums.h"
#include "effects/base/pick.h"

#include "game/game_table.h"

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

    // Every choosable card is a freshly-spawned representative sitting in the shared
    // selection pool, the same way Emporio (General Store) presents its choices.
    static card_list build_nameable_cards(player_ptr origin, card_list &dummies) {
        static constexpr std::string_view common_names[] = { "BANG", "MISSED", "BEER", "DUEL", "INDIANS", "PANIC", "CAT_BALOU" };

        card_list choices;
        for (std::string_view name : common_names) {
            if (card_ptr c = make_representative_by_name(origin, name)) {
                choices.push_back(c);
                dummies.push_back(c);
            }
        }
        for (player_ptr p : origin->m_game->m_players) {
            for (card_ptr c : p->m_table) {
                if (c->is_green() || c->has_tag(tag_type::weapon)) {
                    card_ptr rep = make_representative(origin, *c);
                    choices.push_back(rep);
                    dummies.push_back(rep);
                }
            }
        }
        return choices;
    }

    struct request_vudce_name : request_picking {
        request_vudce_name(card_ptr origin_card, player_ptr origin, player_ptr hand_target, card_list choices, card_list dummies)
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
                hand_target->reveal_hand();
                return;
            }

            hand_target->discard_card(*it);
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_KIT_CARLSON", origin_card};
            } else {
                return {"STATUS_KIT_CARLSON_OTHER", target, origin_card};
            }
        }
    };

    void effect_vudce_ability::on_play(card_ptr origin_card, player_ptr origin, player_ptr target) {
        card_list dummies;
        card_list choices = build_nameable_cards(origin, dummies);

        origin->m_game->add_log("LOG_CARD_HAS_EFFECT", origin_card);
        origin->m_game->queue_request<request_vudce_name>(origin_card, origin, target, std::move(choices), std::move(dummies));
    }

}
