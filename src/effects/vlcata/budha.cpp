#include "budha.h"

#include "cards/filter_enums.h"
#include "cards/game_events.h"
#include "effects/base/pick.h"

#include "game/game_table.h"

namespace banggame {

    static bool is_alcohol_card(card_ptr c) {
        return c->has_tag(tag_type::beer) || c->name == "WHISKY" || c->name == "TEQUILA";
    }

    struct request_budha_reveal : selection_picker {
        request_budha_reveal(card_ptr origin_card, player_ptr target, int remaining)
            : selection_picker(origin_card, nullptr, target)
            , remaining(remaining) {}

        int remaining;

        void on_update() override {
            auto_pick();
        }

        void on_pick(card_ptr target_card) override {
            target->add_to_hand(target_card);
            if (--remaining <= 0) {
                pop_request();
            }
        }

        game_string status_text(player_ptr owner) const override {
            if (owner == target) {
                return {"STATUS_KIT_CARLSON", origin_card};
            } else {
                return {"STATUS_KIT_CARLSON_OTHER", target, origin_card};
            }
        }
    };

    void equip_budha::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_play_card>(target_card, [target, target_card](player_ptr origin, card_ptr played_card, const effect_context &ctx) {
            if (origin == target && is_alcohol_card(played_card)) {
                target_card->flash_card();
                for (int i = 0; i < 3 && target->alive(); ++i) {
                    target->m_game->top_of_deck()->move_to(pocket_type::selection, target);
                }
                target->m_game->queue_request<request_budha_reveal>(target_card, target, 3);
            }
        });
    }

}
