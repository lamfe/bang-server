#include "vudce.h"

#include "effects/base/draw_check.h"

#include "game/game_table.h"

namespace banggame {

    void equip_vudce::on_enable(card_ptr target_card, player_ptr origin) {
        origin->m_game->queue_request<request_check>(origin, target_card, [](card_sign sign) {
            return draw_check_result{ .lucky = sign.is_red() };
        }, [origin, target_card](bool result) {
            if (result) {
                target_card->flash_card();
                origin->m_game->add_log("LOG_CARD_HAS_EFFECT", target_card);
                for (player_ptr p : origin->m_game->range_other_players(origin)) {
                    if (!p->empty_hand()) {
                        p->discard_card(p->random_hand_card());
                    }
                }
            }
        });
    }

}
