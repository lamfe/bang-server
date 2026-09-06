#include "petuna.h"

#include "cards/game_enums.h"
#include "cards/game_events.h"
#include "effects/base/draw_check.h"

#include "game/game_table.h"

namespace banggame {

    void equip_petuna::on_enable(card_ptr target_card, player_ptr target) {
        target->m_game->add_listener<event_type::on_turn_end>(target_card, [target, target_card](player_ptr origin, bool skipped) {
            if (!skipped && origin == target && !target->check_player_flags(player_flag::extra_turn)) {
                target->m_game->queue_request<request_check>(target, target_card, [](card_sign sign) {
                    return draw_check_result{ .lucky = sign.is_red() };
                }, [target, target_card](card_sign sign) {
                    if (sign.is_spades()) {
                        target_card->flash_card();
                        target->damage(target_card, target, 1);
                    } else if (sign.is_red()) {
                        target_card->flash_card();
                        ++target->m_extra_turns;
                    }
                });
            }
        });
    }

}
