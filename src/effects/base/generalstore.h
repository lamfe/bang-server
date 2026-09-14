#ifndef __BASE_GENERALSTORE_H__
#define __BASE_GENERALSTORE_H__

#include "cards/card_effect.h"
#include "pick.h"

namespace banggame {

    namespace event_type {
        struct count_generalstore_cards {
            nullable_ref<int> value;
        };
        struct count_generalstore_extra_picks {
            const_player_ptr target;
            nullable_ref<int> value;
        };
    }

    struct request_generalstore : selection_picker {
        request_generalstore(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags = {}, int extra_picks = 0)
            : selection_picker(origin_card, origin, target, flags), extra_picks(extra_picks) {}

        int extra_picks;

        void on_update() override;

        void on_pick(card_ptr target_card) override;

        game_string status_text(player_ptr owner) const override;
    };

    struct effect_generalstore {
        prompt_string on_prompt(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags);

        void on_play(card_ptr origin_card, player_ptr origin, const effect_context &ctx);
        void on_play(card_ptr origin_card, player_ptr origin, player_ptr target, effect_flags flags, const effect_context &ctx);
    };

    DEFINE_EFFECT(generalstore, effect_generalstore)
}

#endif