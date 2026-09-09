#include "petuna.h"

#include "cards/game_enums.h"
#include "effects/base/draw_check.h"

#include "game/game_table.h"

namespace banggame {

    bool effect_petuna_check::can_play(card_ptr origin_card, player_ptr origin) {
        return !origin->check_player_flags(player_flag::extra_turn);
    }

    void effect_petuna_check::on_play(card_ptr origin_card, player_ptr origin) {
        origin->m_game->queue_request<request_check>(origin, origin_card, [](card_sign sign) {
            return draw_check_result{ .lucky = sign.is_red() };
        }, [origin, origin_card](card_sign sign) {
            if (sign.is_spades()) {
                origin_card->flash_card();
                origin->damage(origin_card, origin, 1);
            } else if (sign.is_red()) {
                origin_card->flash_card();
                ++origin->m_extra_turns;
            }
        });
    }

}
