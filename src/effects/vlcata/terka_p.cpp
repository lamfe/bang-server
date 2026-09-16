#include "terka_p.h"

#include "cards/game_enums.h"
#include "effects/base/damage.h"
#include "effects/base/resolve.h"
#include "effects/base/death.h"

#include "game/game_table.h"
#include "game/request_queue.h"
#include "game/bot_suggestion.h"

namespace banggame {

    // No timer at all -- this waits indefinitely for an explicit yes/no, since it only
    // ever fires right after a successful Bang! hit.
    struct request_terka_p_flip : request_resolvable {
        request_terka_p_flip(card_ptr origin_card, player_ptr origin, player_ptr hit_target, int amount)
            : request_resolvable(origin_card, origin, origin)
            , hit_target(hit_target), amount(amount) {}

        player_ptr hit_target;
        int amount;

        prompt_string resolve_prompt() const override {
            if (origin->is_bot() && bot_suggestion::is_target_enemy(origin, hit_target)) {
                return "BOT_DONT_HEAL_ENEMY";
            }
            return {"PROMPT_TERKA_P_HEAL", hit_target};
        }

        void on_resolve() override {
            pop_request();
            hit_target->heal(origin_card, origin, amount + 1);
        }

        game_string status_text(player_ptr owner) const override {
            return {"STATUS_TERKA_P_FLIP", hit_target};
        }
    };

    void equip_terka_p_bang::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_hit>(target_card, [target, target_card](card_ptr origin_card, player_ptr origin, player_ptr hit_target, int damage, effect_flags flags) {
            if (origin == target && damage > 0 && hit_target->alive() && flags.check(effect_flag::is_bang)) {
                target->m_game->queue_request<request_terka_p_flip>(target_card, target, hit_target, damage);
            }
        });
    }

    void effect_terka_p_selfheal::on_play(card_ptr origin_card, player_ptr origin) {
        origin->heal(origin_card, origin, 1);
    }

}
