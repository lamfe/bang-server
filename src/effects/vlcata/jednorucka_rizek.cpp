#include "jednorucka_rizek.h"

#include "cards/game_enums.h"
#include "cards/game_events.h"
#include "effects/base/death.h"

#include "game/game_table.h"

namespace banggame {

    void equip_jednorucka_rizek::on_enable(card_ptr target_card, player_ptr origin) {
        auto already_saved = std::make_shared<bool>(false);

        origin->m_game->add_listener<event_type::on_player_death>(target_card, [origin, target_card, already_saved](player_ptr target, bool tried_save) {
            if (origin == target && !*already_saved) {
                *already_saved = true;
                target_card->flash_card();
                origin->m_game->add_log("LOG_CARD_HAS_EFFECT", target_card);

                // Prevents the elimination outright (hp<=0 check in handle_player_death runs after this)
                origin->set_hp(1);

                // Applied a step later so it doesn't retroactively make handle_player_death's
                // own alive()-check see him as eliminated during this same resolution.
                origin->m_game->queue_action([origin, target_card]{
                    origin->add_player_flags(player_flag::coffin);
                    origin->add_player_flags(player_flag::keep_alive);

                    origin->m_game->add_listener<event_type::on_turn_end>({target_card, -60}, [origin, target_card](player_ptr end_origin, bool skipped) {
                        if (end_origin == origin->get_prev_player()) {
                            origin->remove_player_flags(player_flag::coffin);
                            origin->remove_player_flags(player_flag::keep_alive);
                            origin->set_hp(3);
                            origin->draw_card(3, target_card);
                            origin->m_game->remove_listeners(event_card_key{target_card, -60});
                        }
                    });
                }, 40);
            }
        });
    }

}
