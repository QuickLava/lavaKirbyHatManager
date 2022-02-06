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
			std::size_t getSectionContainingAddress(std::size_t address);
			std::pair<std::size_t, std::size_t> getSectionInfo(std::size_t sectionIndex);

			std::pair<std::size_t, unsigned long long int> getLinkedCommand(std::size_t entryAddress);
			std::size_t insertLinkedCommand(std::size_t entryAddress, unsigned long long int commandIn);
		};

		namespace kirbyhat
		{
			extern std::ofstream kirbyHatChangelogStream;
			extern const std::string version;
			extern const std::string outputDirectory;
			extern const std::string relFilename;
			extern const std::string kbxFilename;
			extern const std::string inputFilename;
			extern const std::string changelogFilename;
			extern std::unordered_map<std::size_t, std::size_t> fighterIDToMapFuncID;

			extern std::unordered_map<std::size_t, std::string> kirbyHatFIDToNameDict;
			bool buildHatDictionaryFromKBX(lava::byteArray& kbxIn);
			std::vector<std::size_t> addCharacterFIDsAndNamesToMap(const std::vector<std::pair<std::string, std::pair<std::size_t, std::size_t>>>& toAdd);

			constexpr std::size_t maxFighterID = 0x7F;

			constexpr std::size_t transactorNullTag =	0x60000000;
			constexpr std::size_t transactorEntryTag =	0x48000001;
			constexpr std::size_t transactorBlockAddressLinkSection = 0x01;
			constexpr std::size_t transactorBlockAddressLinkLocation = 0x0001FA20;

			std::pair<std::size_t, unsigned long long int> getTransactorBlockLinkInfo(lava::brawl::moduleFile& moduleIn);

			bool addHatToKBX(lava::byteArray& kbxIn, std::size_t charID, std::size_t hatCharID);
			bool addHatToREL(lava::brawl::moduleFile& moduleIn, std::size_t charID, std::size_t hatCharID);
			void summarizeHats(std::ofstream& output, lava::brawl::moduleFile& moduleIn, lava::byteArray& kbxIn);
			bool addHatsToKHEXAsm(std::string asmPathIn, const std::vector<std::pair<std::string, std::pair<std::size_t, std::size_t>>>& toAdd);
		}
	}
}

#endif