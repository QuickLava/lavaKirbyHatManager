#ifndef LAVA_KIRBY_HAT_MANAGER_H_V1
#define LAVA_KIRBY_HAT_MANAGER_H_V1

#include "lavaByteArray.h"
#include "lavaBrawlConstants.h"
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <unordered_map>

namespace lava
{
	std::string numToHexStringWithPadding(std::size_t numIn, std::size_t paddingLength = 8);
	std::string numToDecStringWithPadding(std::size_t numIn, std::size_t paddingLength = 8);

	namespace brawl
	{
		struct moduleFile
		{

			std::string sourceFilePath = "";
			lava::byteArray fileBody;
			
			// Header Info
			std::size_t ID = SIZE_MAX;
			std::size_t sectionsCount = SIZE_MAX;
			std::size_t sectionsEnd = SIZE_MAX;
			std::size_t headerLength = SIZE_MAX;
			std::size_t nameAddress = SIZE_MAX;
			std::size_t nameLength = SIZE_MAX;
			std::size_t version = SIZE_MAX;
			std::size_t importsAddress = SIZE_MAX;
			std::size_t importsLength = SIZE_MAX;
			std::size_t importsCount = SIZE_MAX;
			std::size_t commandsAddress = SIZE_MAX;
			std::size_t commandsLength = SIZE_MAX;

			std::vector<std::pair<std::size_t, std::size_t>> sectionsInfo{};
			std::vector<std::pair<std::size_t, std::size_t>> importsInfo{};
		private:
			std::pair<std::size_t, std::size_t> _IterateThroughCommandsTill(std::size_t entryAddress);
		public:

			bool populate(std::string filePathIn);
			std::pair<std::size_t, std::size_t> getSectionInfo(std::size_t sectionIndex);

			std::size_t getSectionContainingAddress(std::size_t address);
			std::pair<std::size_t, unsigned long long int> getLinkedCommand(std::size_t entryAddress);
			std::pair<std::size_t, unsigned long long int> neoGetLinkedCommand(std::size_t entryAddress);
			std::size_t insertLinkedCommand(std::size_t entryAddress, unsigned long long int commandIn);
			std::size_t neoInsertLinkedCommand(std::size_t entryAddress, unsigned long long int commandIn);
		};

		namespace kirbyhat
		{
			extern std::ofstream kirbyHatChangelogStream;
			extern std::string outputDirectory;

			extern const std::string version;

			constexpr std::size_t sectionListBeginOffset = 0x4C;

			constexpr std::size_t transactorSectionIndex = 0x04;
			extern std::size_t transactorSectionAddress;
			extern std::size_t transactorSectionLength;
			constexpr std::size_t transactorNullCommand =	0x60000000;
			constexpr std::size_t transactorEntryTag =		0x48000001;

			constexpr std::size_t commandSectionAddressHeaderOffset = 0x48;
			constexpr std::size_t importSectionAddressHeaderOffset = 0x28;
			extern std::size_t importSectionAddress;
			constexpr std::size_t importSectionAddressLengthOffset = 0x14;

			constexpr unsigned long long int mewtwoTransactorLinkEntry = 0x00D00A010002060C;
			extern std::size_t mewtwoTransactorLinkEntryAddress;

			constexpr std::size_t maxFighterID = 0x7F;

			const std::unordered_map<std::size_t, std::size_t> fighterIDToMapFuncID =
			{
				{LAVA_CHARA_FIGHTER_IDS::LCFI_MARIO, 0x0001fa3c},
				{LAVA_CHARA_FIGHTER_IDS::LCFI_DONKEY_KONG, 0x0001faac},
				{LAVA_CHARA_FIGHTER_IDS::LCFI_LINK, 0x0001fb1c},
				{LAVA_CHARA_FIGHTER_IDS::LCFI_SAMUS, 0x0001fb8c},
				{LAVA_CHARA_FIGHTER_IDS::LCFI_YOSHI, 0x0001fbfc},
				{LAVA_CHARA_FIGHTER_IDS::LCFI_FOX, 0x0001fc6c},
				{LAVA_CHARA_FIGHTER_IDS::LCFI_PIKACHU, 0x0001fcdc},
				{LAVA_CHARA_FIGHTER_IDS::LCFI_LUIGI, 0x0001fd4c},
				{LAVA_CHARA_FIGHTER_IDS::LCFI_NESS, 0x0001fdbc},
				{LAVA_CHARA_FIGHTER_IDS::LCFI_BOWSER, 0x0001fe2c},
				{LAVA_CHARA_FIGHTER_IDS::LCFI_PEACH, 0x0001fe9c},
				{LAVA_CHARA_FIGHTER_IDS::LCFI_SHEIK, 0x0001ff0c},
				{LAVA_CHARA_FIGHTER_IDS::LCFI_POPO, 0x0001ff7c},
				{LAVA_CHARA_FIGHTER_IDS::LCFI_MARTH, 0x0001ffec},
				{LAVA_CHARA_FIGHTER_IDS::LCFI_MR_GAME_AND_WATCH, 0x0002005c},
				{LAVA_CHARA_FIGHTER_IDS::LCFI_FALCO, 0x000200cc},
				{LAVA_CHARA_FIGHTER_IDS::LCFI_WARIO, 0x0002013c},
				{LAVA_CHARA_FIGHTER_IDS::LCFI_META_KNIGHT, 0x000201ac},
				{LAVA_CHARA_FIGHTER_IDS::LCFI_PIT, 0x0002021c},
				{LAVA_CHARA_FIGHTER_IDS::LCFI_ZERO_SUIT_SAMUS, 0x0002028c},
				{LAVA_CHARA_FIGHTER_IDS::LCFI_OLIMAR, 0x000202fc},
				{LAVA_CHARA_FIGHTER_IDS::LCFI_LUCAS, 0x0002036c},
				{LAVA_CHARA_FIGHTER_IDS::LCFI_DIDDY_KONG, 0x000203dc},
				{LAVA_CHARA_FIGHTER_IDS::LCFI_CHARIZARD, 0x0002044c},
				{LAVA_CHARA_FIGHTER_IDS::LCFI_SQUIRTLE, 0x000204bc},
				{LAVA_CHARA_FIGHTER_IDS::LCFI_IVYSAUR, 0x0002052c},
				{LAVA_CHARA_FIGHTER_IDS::LCFI_DEDEDE, 0x0002059c},
				{LAVA_CHARA_FIGHTER_IDS::LCFI_LUCARIO, 0x0002060c},
				{LAVA_CHARA_FIGHTER_IDS::LCFI_IKE, 0x0002067c},
				{LAVA_CHARA_FIGHTER_IDS::LCFI_ROB, 0x000206ec},
				{LAVA_CHARA_FIGHTER_IDS::LCFI_JIGGLYPUFF, 0x0002075c},
				{LAVA_CHARA_FIGHTER_IDS::LCFI_TOON_LINK, 0x000207cc},
				{LAVA_CHARA_FIGHTER_IDS::LCFI_WOLF, 0x0002083c},
				{LAVA_CHARA_FIGHTER_IDS::LCFI_SNAKE, 0x000208ac},
				{LAVA_CHARA_FIGHTER_IDS::LCFI_SONIC, 0x0002091c}
			};

			bool setup(lava::byteArray& moduleIn);
			void summarizeHats(std::ofstream& output, lava::byteArray& moduleIn, lava::byteArray& kbxIn);
			
			std::size_t getSectionOffset(lava::byteArray& moduleIn, std::size_t sectionIndex);
			std::size_t getSectionSize(lava::byteArray& moduleIn, std::size_t sectionIndex);

			unsigned long long getCommandInfoForEntry(lava::byteArray& moduleIn, std::size_t entryAddress);
			void traceForwardsThroughCommands(lava::byteArray& moduleIn);
			void traceBackwardsThroughCommands(lava::byteArray& moduleIn, std::size_t firstCommandEntryOffset, std::size_t firstCommandOffset, std::size_t currSectionIn);

			bool addHatToREL(lava::byteArray& moduleIn, std::size_t charID, std::size_t hatCharID);
			bool addHatToKBX(lava::byteArray& kbxIn, std::size_t charID, std::size_t hatCharID);
			bool addHatToASM(lava::byteArray& asmIn, std::size_t charID, std::size_t hatCharID);

			constexpr std::size_t transactorBlockAddressLinkSection = 0x01;
			constexpr std::size_t transactorBlockAddressLinkLocation = 0x0001FA20;
			std::pair<std::size_t, unsigned long long int> getTransactorBlockLinkInfo(lava::brawl::moduleFile& moduleIn);
			bool neoAddHatToREL(lava::brawl::moduleFile& moduleIn, std::size_t charID, std::size_t hatCharID);
			void neoSummarizeHats(std::ofstream& output, lava::brawl::moduleFile& moduleIn, lava::byteArray& kbxIn);
		}
	}
}

#endif