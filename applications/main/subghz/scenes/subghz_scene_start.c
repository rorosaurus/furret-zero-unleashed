#include "../subghz_i.h"
#include <dolphin/dolphin.h>

#include <lib/subghz/protocols/raw.h>

enum SubmenuIndex {
    SubmenuIndexRead = 10,
    SubmenuIndexSaved,
    SubmenuIndexTest,
    SubmenuIndexAddManualy,
    SubmenuIndexFrequencyAnalyzer,
    SubmenuIndexReadRAW,
};

void subghz_scene_start_remove_advanced_preset(SubGhz* subghz) {
    // delete operation is harmless
    subghz_setting_delete_custom_preset(subghz->setting, ADVANCED_AM_PRESET_NAME);
}

void subghz_scene_start_load_advanced_preset(SubGhz* subghz) {
    for(uint8_t i = 0; i < subghz_setting_get_preset_count(subghz->setting); i++) {
        if(!strcmp(subghz_setting_get_preset_name(subghz->setting, i), ADVANCED_AM_PRESET_NAME)) {
            return; // already exists
        }
    }

    // Load custom advanced AM preset with configurable CFGMDM settings
    FlipperFormat* advanced_am_preset = subghz_preset_custom_advanced_am_preset_alloc();
    subghz_setting_load_custom_preset(
        subghz->setting, ADVANCED_AM_PRESET_NAME, advanced_am_preset);
    flipper_format_free(advanced_am_preset);
}

void subghz_scene_start_submenu_callback(void* context, uint32_t index) {
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, index);
}

void subghz_scene_start_on_enter(void* context) {
    SubGhz* subghz = context;
    if(subghz->state_notifications == SubGhzNotificationStateStarting) {
        subghz->state_notifications = SubGhzNotificationStateIDLE;
    }
#ifdef SUBGHZ_SAVE_DETECT_RAW_SETTING
    subghz_last_settings_set_detect_raw_values(subghz);
#else
    subghz_protocol_decoder_raw_set_auto_mode(
        subghz_receiver_search_decoder_base_by_name(
            subghz->txrx->receiver, SUBGHZ_PROTOCOL_RAW_NAME),
        false);
    subghz_receiver_set_filter(subghz->txrx->receiver, SubGhzProtocolFlag_Decodable);
#endif

    submenu_add_item(
        subghz->submenu, "Read", SubmenuIndexRead, subghz_scene_start_submenu_callback, subghz);
    submenu_add_item(
        subghz->submenu,
        "Read RAW",
        SubmenuIndexReadRAW,
        subghz_scene_start_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu, "Saved", SubmenuIndexSaved, subghz_scene_start_submenu_callback, subghz);
    submenu_add_item(
        subghz->submenu,
        "Add Manually",
        SubmenuIndexAddManualy,
        subghz_scene_start_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "Frequency Analyzer",
        SubmenuIndexFrequencyAnalyzer,
        subghz_scene_start_submenu_callback,
        subghz);
    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
        submenu_add_item(
            subghz->submenu, "Test", SubmenuIndexTest, subghz_scene_start_submenu_callback, subghz);
    }
    submenu_set_selected_item(
        subghz->submenu, scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneStart));

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdMenu);
}

bool subghz_scene_start_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    if(event.type == SceneManagerEventTypeBack) {
        //exit app
        scene_manager_stop(subghz->scene_manager);
        view_dispatcher_stop(subghz->view_dispatcher);
        return true;
    } else if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexReadRAW) {
            subghz_scene_start_load_advanced_preset(subghz);
            scene_manager_set_scene_state(
                subghz->scene_manager, SubGhzSceneStart, SubmenuIndexReadRAW);
            subghz->txrx->rx_key_state = SubGhzRxKeyStateIDLE;
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneReadRAW);
            return true;
        } else if(event.event == SubmenuIndexRead) {
            subghz_scene_start_remove_advanced_preset(subghz);
            scene_manager_set_scene_state(
                subghz->scene_manager, SubGhzSceneStart, SubmenuIndexRead);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneReceiver);
            return true;
        } else if(event.event == SubmenuIndexSaved) {
            scene_manager_set_scene_state(
                subghz->scene_manager, SubGhzSceneStart, SubmenuIndexSaved);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSaved);
            return true;
        } else if(event.event == SubmenuIndexAddManualy) {
            scene_manager_set_scene_state(
                subghz->scene_manager, SubGhzSceneStart, SubmenuIndexAddManualy);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSetType);
            return true;
        } else if(event.event == SubmenuIndexFrequencyAnalyzer) {
            scene_manager_set_scene_state(
                subghz->scene_manager, SubGhzSceneStart, SubmenuIndexFrequencyAnalyzer);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneFrequencyAnalyzer);
            DOLPHIN_DEED(DolphinDeedSubGhzFrequencyAnalyzer);
            return true;

        } else if(event.event == SubmenuIndexTest) {
            scene_manager_set_scene_state(
                subghz->scene_manager, SubGhzSceneStart, SubmenuIndexTest);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneTest);
            return true;
        }
    }
    return false;
}

void subghz_scene_start_on_exit(void* context) {
    SubGhz* subghz = context;
    submenu_reset(subghz->submenu);
}
