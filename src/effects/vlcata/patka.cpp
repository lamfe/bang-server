#include "patka.h"

#include "effects/base/resolve.h"
#include "effects/base/pick.h"

#include "target_types/base/players.h"

#include "game/game_table.h"

namespace banggame {

    static bool affects_all_players(card_ptr c) {
        for (const effect_holder &holder : c->get_effect_list(effect_list_type::effects)) {
            if (holder.target == TARGET_TYPE(players)) {
                return true;
            }
        }
        return false;
    }

    struct request_patka_discard : request_picking, interface_resolvable {
        request_patka_discard(card_ptr origin_card, player_ptr target)
            : request_picking(origin_card, nullptr, target) {}

        int discarded = 0;

        bool can_pick(card_ptr target_card) const override {
            return target_card->owner == target && target_card->pocket == pocket_type::player_hand;
        }

        void on_pick(card_ptr target_card) override {
            if (discarded == 0 && affects_all_players(target_card)) {
                target->discard_card(target_card);
                pop_request();
                target->draw_card(2, origin_card);
                return;
            }

            target->discard_card(target_card);
            ++discarded;
            if (target->empty_hand()) {
                on_resolve();
            }
        }

        void on_resolve() override {
            pop_request();
            if (discarded > 1) {
                target->draw_card(discarded - 1, origin_card);
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

    void effect_patka_discard::on_play(card_ptr origin_card, player_ptr origin) {
        origin->m_game->queue_request<request_patka_discard>(origin_card, origin);
    }

}
