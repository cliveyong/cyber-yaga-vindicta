#include "tinyECS/components.hpp"

// TEMPLATE
//struct Gun {
//    GUN_TYPE gun_type = GUN_TYPE::PISTOL;
//    float projectile_speed = 3000.0f;
//    float damage = 100.f;
//    float cooldown_ms = 250.0f;
//    float cooldown_timer_ms = 0.0f;
//    float reload_time = 1050.0f;
//    float magazine_size = 4;
//    float remaining_bullets = 14;
//    float current_magazine = 4;
//    float throw_timer_ms = 0.f;
//    float throw_cooldown_time = 500.f;
//    int projectile_count = 1;
//    float spread_cone = 15.f;
//    float recoil_pushback = 1.7f;
//    int penetrating_count = 0;
//    int ricochet_remaining = 0;
//    bool reload_one_at_a_time = false;
//    SOUND_ASSET_ID sound_effect = SOUND_ASSET_ID::PISTOL_GUNSHOT_1;
//    SOUND_ASSET_ID click_effect = SOUND_ASSET_ID::PISTOL_DRY_FIRE_1;
//    SOUND_ASSET_ID reload_effect = SOUND_ASSET_ID::PISTOL_RELOAD_1;
//    SOUND_ASSET_ID rack_effect = SOUND_ASSET_ID::PISTOL_RELOAD_1;
//    TEXTURE_ASSET_ID hud_sprite = TEXTURE_ASSET_ID::PISTOL;
//    TEXTURE_ASSET_ID thrown_sprite = TEXTURE_ASSET_ID::PISTOL;
//};

Gun ENEMY_PISTOL = Gun{
    GUN_TYPE::ENEMY_PISTOL,
    3000.f,
    100.f,
    750.f,
    0.f,
    1050.f,
    7,
    14,
    7,
    0.f,
    500.f,
    1,
    15.f,
    1.7f,
    0,
    0,
    false,
    SOUND_ASSET_ID::PISTOL_GUNSHOT_1,
    SOUND_ASSET_ID::PISTOL_DRY_FIRE_1,
    SOUND_ASSET_ID::PISTOL_RELOAD_1,
    SOUND_ASSET_ID::PISTOL_RELOAD_1,
    TEXTURE_ASSET_ID::PISTOL,
    TEXTURE_ASSET_ID::PISTOL_PICKUP
};

Gun PISTOL = Gun{
    GUN_TYPE::PISTOL,
    3000.f,
    100.f,
    250.f,
    0.f,
    1050.f,
    7,
    14,
    7,
    0.f,
    500.f,
    1,
    15.f,
    1.7f,
    0,
    0,
    false,
    SOUND_ASSET_ID::PISTOL_GUNSHOT_1,
    SOUND_ASSET_ID::PISTOL_DRY_FIRE_1,
    SOUND_ASSET_ID::PISTOL_RELOAD_1,
    SOUND_ASSET_ID::PISTOL_RELOAD_1,
    TEXTURE_ASSET_ID::PISTOL,
    TEXTURE_ASSET_ID::PISTOL_PICKUP
};

Gun DUMMY_GUN = Gun{
    GUN_TYPE::DUMMY_GUN,
    3000.f,
    100.f,
    250.f,
    10000000000000000000000000000000.f,
    1050.f,
    7,
    14,
    7,
    0.f,
    500.f,
    1,
    15.f,
    1.7f,
    0,
    0,
    false,
    SOUND_ASSET_ID::PISTOL_GUNSHOT_1,
    SOUND_ASSET_ID::PISTOL_DRY_FIRE_1,
    SOUND_ASSET_ID::PISTOL_RELOAD_1,
    SOUND_ASSET_ID::PISTOL_RELOAD_1,
    TEXTURE_ASSET_ID::PISTOL,
    TEXTURE_ASSET_ID::PISTOL_PICKUP
};

Gun SHOTGUN = Gun{
    GUN_TYPE::SHOTGUN,
    3000.f,
    90.f,
    500.f,
    0.f,
    600.f,
    4,
    14,
    4,
    0.f,
    500.f,
    5,
    15.f,
    10.f,
    0,
    0,
    true,
    SOUND_ASSET_ID::SHOTGUN_SHOT_1,
    SOUND_ASSET_ID::PISTOL_DRY_FIRE_1,
    SOUND_ASSET_ID::SHOTGUN_RELOAD_1,
    SOUND_ASSET_ID::SHOTGUN_COCK_1,
    TEXTURE_ASSET_ID::SHOTGUN_UI,
    TEXTURE_ASSET_ID::SHOTGUN_PICKUP
};

Gun SMG = Gun{
    GUN_TYPE::SMG,
    3000.f,
    50.f,
    120.f,
    0.f,
    1050.f,
    20,
    0,
    20,
    0.f,
    500.f,
    1,
    15.f,
    3.f,
    0,
    0,
    false,
    SOUND_ASSET_ID::PISTOL_GUNSHOT_1,
    SOUND_ASSET_ID::PISTOL_DRY_FIRE_1,
    SOUND_ASSET_ID::PISTOL_RELOAD_1,
    SOUND_ASSET_ID::PISTOL_RELOAD_1,
    TEXTURE_ASSET_ID::SMG_UI,
    TEXTURE_ASSET_ID::SMG_PICKUP
};


Gun RAILGUN = Gun{
    GUN_TYPE::RAILGUN,
    4000.f,
    100.f,
    150.f,
    0.f,
    600.f,
    5,
    10,
    5,
    0.f,
    500.f,
    1,
    15.f,
    1.7f,
    1,
    0,
    false,
    SOUND_ASSET_ID::RAILGUN_SHOT_1,
    SOUND_ASSET_ID::PISTOL_DRY_FIRE_1,
    SOUND_ASSET_ID::RAILGUN_RELOAD_1,
    SOUND_ASSET_ID::PISTOL_RELOAD_1,
    TEXTURE_ASSET_ID::RAILGUN_UI,
    TEXTURE_ASSET_ID::RAILGUN_UI
};

Gun REVOLVER = Gun{
    GUN_TYPE::REVOLVER,
    4000.f,
    150.f,
    750.f,
    0.f,
    600.f,
    6,
    3,
    6,
    0.f,
    500.f,
    1,
    15.f,
    13.f,
    0,
    2,
    true,
    SOUND_ASSET_ID::REVOLVER_SHOT_1,
    SOUND_ASSET_ID::REVOLVER_DRY_FIRE_1,
    SOUND_ASSET_ID::REVOLVER_RELOAD_1,
    SOUND_ASSET_ID::REVOLVER_COCK_1,
    TEXTURE_ASSET_ID::REVOLVER_UI,
    TEXTURE_ASSET_ID::REVOLVER_UI
};
