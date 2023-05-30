#include "Resources.hpp"

#include <DoodleJumpConfig.hpp>

sf::Texture& SpriteStat::getTexture()
{
	return global_textures[texture_name];
}

sf::Sprite SpriteStat::createSprite()
{
	return sf::Sprite(getTexture(), texture_rect);
}

void init_resources()
{
	global_textures.acquire("background", thor::Resources::fromFile<sf::Texture>(RESOURCES_PATH"background.png"));
	global_textures["background"].setRepeated(true);
	global_textures["background"].setSmooth(true);

	global_textures.acquire("empty_white_texture", thor::Resources::fromFile<sf::Texture>(RESOURCES_PATH"empty_white_square.png"));
	global_textures["empty_white_texture"].setRepeated(true);

	global_textures.acquire("doodle", thor::Resources::fromFile<sf::Texture>(RESOURCES_PATH"doodle_basic.png"));
	global_textures.acquire("tiles", thor::Resources::fromFile<sf::Texture>(RESOURCES_PATH"tiles.png"));
	global_textures.acquire("items", thor::Resources::fromFile<sf::Texture>(RESOURCES_PATH"items.png"));
	global_textures.acquire("monsters", thor::Resources::fromFile<sf::Texture>(RESOURCES_PATH"monsters.png"));

	
	global_sprites["background"] = SpriteStat{ "background", {0, 0, 528, 829} };
	global_sprites["empty_white_texture"] = SpriteStat{ "empty_white_texture", {0, 0, 10, 10} };
	global_sprites["doodle_body_right"] = SpriteStat{ "doodle" , {0, 0, 80, 80} };
	global_sprites["doodle_body_up"] = SpriteStat{ "doodle", {80, 0, 80, 80} };
	global_sprites["doodle_feet_right"] = SpriteStat{ "doodle", {0, 80, 80, 20} };	
	global_sprites["doodle_feet_up"] = SpriteStat{ "doodle", {80, 80, 80, 20} };
	global_sprites["doodle_nose"] = SpriteStat{ "doodle", {160, 0, 40, 40} };
	global_sprites["doodle_bullet"] = SpriteStat{ "doodle", { 178, 54, 22, 22 } };
	global_sprites["doodle_star"] = SpriteStat{ "doodle", {176, 76, 24, 24} };

	global_sprites["tiles_normal"] = SpriteStat{ "tiles", { 0, 0, 128, 40 } };
	global_sprites["tiles_horizontal"] = SpriteStat{ "tiles", { 0, 40, 128, 40 } };
	global_sprites["tiles_vertical"] = SpriteStat{ "tiles", { 0, 80, 128, 40 } };
	global_sprites["tiles_decayed_0"] = SpriteStat{ "tiles", { 0, 120, 128, 40 } };
	global_sprites["tiles_decayed_1"] = SpriteStat{ "tiles", { 0, 160, 128, 40 } };
	global_sprites["tiles_decayed_2"] = SpriteStat{ "tiles", { 0, 200, 128, 60 } };
	global_sprites["tiles_decayed_3"] = SpriteStat{ "tiles", { 0, 260, 128, 60 } };
	global_sprites["tiles_bomb_0"] = SpriteStat{ "tiles", { 0, 360, 128, 40 } };
	global_sprites["tiles_bomb_1"] = SpriteStat{ "tiles", { 0, 400, 128, 40 } };
	global_sprites["tiles_bomb_2"] = SpriteStat{ "tiles", { 0, 440, 128, 40 } };
	global_sprites["tiles_bomb_3"] = SpriteStat{ "tiles", { 0, 480, 128, 40 } };
	global_sprites["tiles_bomb_4"] = SpriteStat{ "tiles", { 0, 520, 128, 40 } };
	global_sprites["tiles_bomb_5"] = SpriteStat{ "tiles", { 0, 560, 128, 40 } };
	global_sprites["tiles_bomb_6"] = SpriteStat{ "tiles", { 0, 600, 128, 60 } };
	global_sprites["tiles_bomb_7"] = SpriteStat{"tiles", {0, 660, 128, 60}};
	global_sprites["tiles_one_time"] = SpriteStat{ "tiles", { 0, 720, 128, 40 } };
	global_sprites["tiles_teleport_0"] = SpriteStat{ "tiles", { 0, 760, 128, 40 } };
	global_sprites["tiles_teleport_1"] = SpriteStat{ "tiles", { 0, 800, 128, 40 } };
	global_sprites["tiles_cluster"] = SpriteStat{ "tiles", { 0, 320, 128, 40 } };

	global_sprites["items_spring_0"] = SpriteStat{ "items", { 0, 0, 34, 24 } };
	global_sprites["items_spring_1"] = SpriteStat{ "items", { 0, 24, 34, 54 } };
	global_sprites["items_trampoline_0"] = SpriteStat{ "items", { 34, 0, 72, 28 } };
	global_sprites["items_trampoline_1"] = SpriteStat{ "items", { 34, 56, 72, 34 } };
	global_sprites["items_trampoline_2"] = SpriteStat{ "items", { 34, 28, 72, 28 } };
	global_sprites["items_propeller_hat_0"] = SpriteStat{ "items", { 106, 0, 58, 38 } };
	global_sprites["items_propeller_hat_1"] = SpriteStat{ "items", { 106, 38, 58, 46 } };
	global_sprites["items_propeller_hat_2"] = SpriteStat{ "items", { 106, 84, 58, 52 } };
	global_sprites["items_propeller_hat_3"] = SpriteStat{ "items", { 106, 136, 58, 52 } };
	global_sprites["items_jetpack_0"] = SpriteStat{ "items", { 0, 90, 24, 72 } };
	global_sprites["items_jetpack_1"] = SpriteStat{ "items", { 0, 90, 48, 72 } };
	global_sprites["items_jetpack_2"] = SpriteStat{ "items", { 0, 162, 20, 12 } };
	global_sprites["items_jetpack_3"] = SpriteStat{ "items", { 24, 162, 30, 24 } };
	global_sprites["items_jetpack_4"] = SpriteStat{ "items", { 48, 90, 32, 32 } };
	global_sprites["items_jetpack_5"] = SpriteStat{ "items", { 54, 122, 44, 50 } };
	global_sprites["items_jetpack_6"] = SpriteStat{ "items", { 0, 190, 48, 48 } };
	global_sprites["items_jetpack_7"] = SpriteStat{ "items", { 54, 172, 52, 52 } };
	global_sprites["items_jetpack_8"] = SpriteStat{ "items", { 106, 188, 28, 38 } };
	global_sprites["items_jetpack_9"] = SpriteStat{ "items", { 80, 90, 26, 26 } };
	global_sprites["items_jetpack_10"] = SpriteStat{ "items", { 0, 174, 24, 16 } };
	global_sprites["items_spring_shoes_0"] = SpriteStat{ "items", { 0, 252, 52, 24 } };
	global_sprites["items_spring_shoes_1"] = SpriteStat{ "items", {0, 238, 52, 14} };
	global_sprites["items_spring_shoes_2"] = SpriteStat{ "items", {52, 238, 48, 14} };
	global_sprites["items_spring_shoes_3"] = SpriteStat{ "items", {0, 238, 52, 38} };
	global_sprites["items_shield_0"] = SpriteStat{ "items", { 100, 230, 66, 66 } };
	global_sprites["items_shield_1"] = SpriteStat{ "items", { 170, 0, 192, 192 } };
	global_sprites["items_shield_2"] = SpriteStat{ "items", { 170, 192, 192, 192 } };
	global_sprites["items_shield_3"] = SpriteStat{ "items", { 170, 384, 192, 192 } };

	global_sprites["monsters_blue_one_eyed"] = SpriteStat{ "monsters", { 0, 0, 74, 98 } };
	global_sprites["monsters_camron"] = SpriteStat{ "monsters", { 184, 0, 92, 72 } };
	global_sprites["monsters_purple_spider"] = SpriteStat{ "monsters", { 74, 0, 110, 98 } };
	global_sprites["monsters_large_blue"] = SpriteStat{ "monsters", { 432, 0, 170, 106 } };
	global_sprites["monsters_ufo_0"] = SpriteStat{ "monsters", { 276, 0, 156, 68 } };
	global_sprites["monsters_ufo_1"] = SpriteStat{ "monsters", { 0, 98, 160, 200 } };
	global_sprites["monsters_black_hole"] = SpriteStat{ "monsters", {160, 98, 140, 130} };
	global_sprites["monsters_oval_green_0"] = SpriteStat{ "monsters", { 300, 106, 124, 168 } };
	global_sprites["monsters_oval_green_1"] = SpriteStat{ "monsters", { 424, 106, 124, 168 } };
	global_sprites["monsters_flat_green_0"] = SpriteStat{ "monsters", { 602, 0, 182, 62 } };
	global_sprites["monsters_flat_green_1"] = SpriteStat{ "monsters", { 602, 62, 182, 62 } };
	global_sprites["monsters_large_green_0"] = SpriteStat{ "monsters", { 0, 298, 162, 102 } };
	global_sprites["monsters_large_green_1"] = SpriteStat{ "monsters", { 162, 298, 162, 102 } };
	global_sprites["monsters_large_green_2"] = SpriteStat{ "monsters", { 324, 298, 162, 116 } };
	global_sprites["monsters_large_green_3"] = SpriteStat{ "monsters", { 486, 298, 162, 120 } };
	global_sprites["monsters_large_green_4"] = SpriteStat{ "monsters", { 648, 298, 162, 120 } };
	global_sprites["monsters_blue_winged_0"] = SpriteStat{ "monsters", { 0, 420, 156, 88 } };
	global_sprites["monsters_blue_winged_1"] = SpriteStat{ "monsters", { 156, 420, 156, 88 } };
	global_sprites["monsters_blue_winged_2"] = SpriteStat{ "monsters", { 312, 420, 156, 88 } };
	global_sprites["monsters_blue_winged_3"] = SpriteStat{ "monsters", { 468, 420, 156, 88 } };
	global_sprites["monsters_blue_winged_4"] = SpriteStat{ "monsters", { 624, 420, 156, 88 } };
	global_sprites["monsters_the_terrifying_0"] = SpriteStat{ "monsters", { 126, 510, 126, 174 } };
	global_sprites["monsters_the_terrifying_1"] = SpriteStat{ "monsters", { 252, 510, 126, 174 } };
	global_sprites["monsters_the_terrifying_2"] = SpriteStat{ "monsters", { 378, 510, 126, 174 } };
	global_sprites["monsters_the_terrifying_3"] = SpriteStat{ "monsters", { 504, 510, 134, 174 } };
	global_sprites["monsters_the_terrifying_4"] = SpriteStat{ "monsters", { 0, 510, 126, 174 } };
}

void release_resources()
{
	global_textures.release("background");
	global_textures.release("empty_white_texture");
	global_textures.release("doodle");
	global_textures.release("tiles");
	global_textures.release("items");
	global_textures.release("monsters");
}