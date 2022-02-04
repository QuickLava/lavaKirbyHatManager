#ifndef LAVA_BRAWL_CONSTANTS_H_V1
#define LAVA_BRAWL_CONSTANTS_H_V1

#include <unordered_map>

namespace lava
{
	namespace brawl
	{
		enum LAVA_CHARA_SLOT_IDS
		{
			LCSI_BOWSER = 12,
			LCSI_CAPTAIN_FALCON = 10,
			LCSI_CHARIZARD = 30,
			LCSI_DEDEDE = 35,
			LCSI_DIDDY_KONG = 28,
			LCSI_DONKEY_KONG = 1,
			LCSI_FALCO = 21,
			LCSI_FOX = 7,
			LCSI_GANONDORF = 22,
			LCSI_GIGA_BOWSER = 44,
			LCSI_ICE_CLIMBERS = 16,
			LCSI_IKE = 37,
			LCSI_IVYSAUR = 34,
			LCSI_JIGGLYPUFF = 39,
			LCSI_KIRBY = 6,
			LCSI_LINK = 2,
			LCSI_LUCARIO = 36,
			LCSI_LUCAS = 27,
			LCSI_LUIGI = 9,
			LCSI_MARIO = 0,
			LCSI_MARTH = 19,
			LCSI_META_KNIGHT = 24,
			LCSI_MR_GAME_AND_WATCH = 20,
			LCSI_NESS = 11,
			LCSI_OLIMAR = 26,
			LCSI_PEACH = 13,
			LCSI_PIKACHU = 8,
			LCSI_PIT = 25,
			LCSI_POKEMON_TRAINER = 72,
			LCSI_ROB = 38,
			LCSI_SAMUS = 3,
			LCSI_SHEIK = 15,
			LCSI_SNAKE = 42,
			LCSI_SONIC = 43,
			LCSI_SOPO = 17,
			LCSI_SQUIRTLE = 32,
			LCSI_TOON_LINK = 40,
			LCSI_WARIO = 23,
			LCSI_WARIOMAN = 45,
			LCSI_WOLF = 41,
			LCSI_YOSHI = 5,
			LCSI_ZELDA = 14,
			LCSI_ZERO_SUIT_SAMUS = 4,
			LCSI_MEWTWO = 51,
			LCSI_ROY = 50,
			LCSI_KNUCKLES = 53,
			LCSI_RIDLEY = 56,
			//LCSI_DARK_SAMUS = 0x40,
			//LCSI_WALUIGI = 0x39,
		};
		enum LAVA_CHARA_FIGHTER_IDS
		{
			LCFI_BOWSER = 0x0B,
			LCFI_CAPTAIN_FALCON = 0x09,
			LCFI_CHARIZARD = 0x1D,
			LCFI_DEDEDE = 0x20,
			LCFI_DIDDY_KONG = 0x1B,
			LCFI_DONKEY_KONG = 0x01,
			LCFI_FALCO = 0x13,
			LCFI_FOX = 0x06,
			LCFI_GANONDORF = 0x14,
			LCFI_GIGA_BOWSER = 0x30,
			LCFI_IKE = 0x22,
			LCFI_IVYSAUR = 0x1F,
			LCFI_JIGGLYPUFF = 0x25,
			LCFI_KIRBY = 0x05,
			LCFI_LINK = 0x02,
			LCFI_LUCARIO = 0x21,
			LCFI_LUCAS = 0x1A,
			LCFI_LUIGI = 0x08,
			LCFI_MARIO = 0x00,
			LCFI_MARTH = 0x11,
			LCFI_META_KNIGHT = 0x16,
			LCFI_MR_GAME_AND_WATCH = 0x12,
			LCFI_NANA = 0x10,
			LCFI_NESS = 0x0A,
			LCFI_OLIMAR = 0x19,
			LCFI_PEACH = 0x0C,
			LCFI_PIKACHU = 0x07,
			LCFI_PIT = 0x17,
			LCFI_POKEMON_TRAINER = 0x1C,
			LCFI_POPO = 0x0F,
			LCFI_ROB = 0x23,
			LCFI_SAMUS = 0x03,
			LCFI_SHEIK = 0x0E,
			LCFI_SNAKE = 0x2E,
			LCFI_SONIC = 0x2F,
			LCFI_SOPO = 0x0F,
			LCFI_SQUIRTLE = 0x1E,
			LCFI_TOON_LINK = 0x29,
			LCFI_WARIO = 0x15,
			LCFI_WARIOMAN = 0x31,
			LCFI_WOLF = 0x2C,
			LCFI_YOSHI = 0x04,
			LCFI_ZELDA = 0x0D,
			LCFI_ZERO_SUIT_SAMUS = 0x18,
			LCFI_MEWTWO = 0x26,
			LCFI_ROY = 0x27,
			LCFI_KNUCKLES = 0x2D,
			LCFI_RIDLEY = 0x2A,
			//LCFI_DARK_SAMUS = 0x40,
			//LCFI_WALUIGI = 0x28,
		};

		const std::unordered_map<std::size_t, std::string> fighterIDToName = {
			{ 0x0B, "BOWSER" },
			{ 0x09, "CAPTAIN_FALCON" },
			{ 0x1D, "CHARIZARD" },
			{ 0x20, "DEDEDE" },
			{ 0x1B, "DIDDY_KONG" },
			{ 0x01, "DONKEY_KONG" },
			{ 0x13, "FALCO" },
			{ 0x06, "FOX" },
			{ 0x14, "GANONDORF" },
			{ 0x30, "GIGA_BOWSER" },
			{ 0x22, "IKE" },
			{ 0x1F, "IVYSAUR" },
			{ 0x25, "JIGGLYPUFF" },
			{ 0x05, "KIRBY" },
			{ 0x02, "LINK" },
			{ 0x21, "LUCARIO" },
			{ 0x1A, "LUCAS" },
			{ 0x08, "LUIGI" },
			{ 0x00, "MARIO" },
			{ 0x11, "MARTH" },
			{ 0x16, "META_KNIGHT" },
			{ 0x12, "MR_GAME_AND_WATCH" },
			{ 0x10, "NANA" },
			{ 0x0A, "NESS" },
			{ 0x19, "OLIMAR" },
			{ 0x0C, "PEACH" },
			{ 0x07, "PIKACHU" },
			{ 0x17, "PIT" },
			{ 0x1C, "POKEMON_TRAINER" },
			{ 0x0F, "POPO" },
			{ 0x23, "ROB" },
			{ 0x03, "SAMUS" },
			{ 0x0E, "SHEIK" },
			{ 0x2E, "SNAKE" },
			{ 0x2F, "SONIC" },
			{ 0x0F, "SOPO" },
			{ 0x1E, "SQUIRTLE" },
			{ 0x29, "TOON_LINK" },
			{ 0x15, "WARIO" },
			{ 0x31, "WARIOMAN" },
			{ 0x2C, "WOLF" },
			{ 0x04, "YOSHI" },
			{ 0x0D, "ZELDA" },
			{ 0x18, "ZERO_SUIT_SAMUS" },
			{ 0x26, "MEWTWO" },
			{ 0x27, "ROY" },
			{ 0x2D, "KNUCKLES" },
			{ 0x2A, "RIDLEY" },
			//{ 0x40, "DARK_SAMUS" },
			//{ 0x28, "WALUIGI" },
		};
	}
}


#endif