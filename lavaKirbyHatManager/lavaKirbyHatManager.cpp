#include "lavaKirbyHatManager.h"

namespace lava
{
	std::string numToHexStringWithPadding(std::size_t numIn, std::size_t paddingLength)
	{
		std::stringstream convBuff;
		convBuff << std::hex << numIn;
		std::string result = convBuff.str();
		for (int i = 0; i < result.size(); i++)
		{
			result[i] = std::toupper(result[i]);
		}
		if (result.size() < paddingLength)
		{
			result = std::string(paddingLength - result.size(), '0') + result;
		}
		return result;
	}
	std::string numToDecStringWithPadding(std::size_t numIn, std::size_t paddingLength)
	{
		std::string result = std::to_string(numIn);
		if (result.size() < paddingLength)
		{
			result = std::string(paddingLength - result.size(), '0') + result;
		}
		return result;
	}

	bool fileExists(std::string filepathIn)
	{
		std::ifstream result(filepathIn);
		return result.is_open();
	}
	bool folderExists(std::string folderpathIn)
	{
		bool result = 0;

		if (folderpathIn.back() != '/' && folderpathIn.back() != '\\')
		{
			folderpathIn.push_back('/');
		}
		std::string testFileLoc = folderpathIn + "_test";
		std::ofstream test(testFileLoc);
		if (test.is_open())
		{
			result = 1;
			test.close();
			remove(testFileLoc.c_str());
		}
		return result;
	}
	bool copyFile(std::string sourceFile, std::string targetFile, bool overwriteExistingFile)
	{
		// Record result
		bool result = 0;
		if (sourceFile != targetFile)
		{
			// Initialize in and out streams
			std::ifstream sourceFileStream;
			std::ofstream targetFileStream;
			// Open and test input stream
			sourceFileStream.open(sourceFile, std::ios_base::in | std::ios_base::binary);
			if (sourceFileStream.is_open())
			{
				if (overwriteExistingFile || !fileExists(targetFile))
				{
					// If successful, open and test output stream
					targetFileStream.open(targetFile, std::ios_base::out | std::ios_base::binary);
					if (targetFileStream.is_open())
					{
						// If both streams are open and valid, copy over the file's contents and record the success in result
						targetFileStream << sourceFileStream.rdbuf();
						result = 1;
					}
					targetFileStream.close();
				}
			}
			sourceFileStream.close();
		}
		return result;
	}
	bool backupFile(std::string fileToBackup, std::string backupSuffix, bool overwriteExistingBackup)
	{
		return copyFile(fileToBackup, fileToBackup + backupSuffix, overwriteExistingBackup);
	}

	namespace brawl
	{
		std::pair<std::size_t, std::size_t> moduleFile::_IterateThroughCommandsTill(std::size_t entryAddress)
		{
			std::pair<std::size_t, std::size_t> result = { SIZE_MAX, SIZE_MAX };

			std::size_t currentSectionIndex = getSectionContainingAddress(entryAddress);
			if (currentSectionIndex != SIZE_MAX)
			{
				std::pair<std::size_t, std::size_t> currentSectionInfo = getSectionInfo(currentSectionIndex);
				std::size_t commandCursor = fileBody.searchLong(0x0000CA00 + currentSectionIndex, commandsAddress, commandsAddress + commandsLength);
				std::size_t commandValue1 = 0x00;
				std::size_t commandValue2 = 0x00;
				std::size_t entryCursor = currentSectionInfo.first;
				if (commandCursor != SIZE_MAX)
				{
					entryCursor += fileBody.getShort(commandCursor + 0x08);
					commandCursor += 0x08;
				}
				unsigned short distanceToNext = 0x00;
				//std::cout << "Searching for a command linked to 0x" << lava::numToHexStringWithPadding(entryAddress, 0x08) << " in Section[" << currentSectionIndex << "]...\n";
				bool finished = 0;
				while (!finished && commandCursor < fileBody.body.size())
				{
					distanceToNext = fileBody.getShort(commandCursor + 0x08);
					commandValue1 = fileBody.getLong(commandCursor);
					commandValue2 = fileBody.getLong(commandCursor + 0x04);
					if ((commandValue1 & 0xFFFFFFF0) == 0x0000CA00 && commandValue2 == 0x00000000)
					{
						finished = 1;
					}
					else
					{
						//std::cout << "\t[0x" << lava::numToHexStringWithPadding(entryCursor, 0x08) << ", " << lava::numToHexStringWithPadding(commandCursor) << "] ";
						//std::cout << lava::numToHexStringWithPadding(commandValue1, 0x08) << " " << lava::numToHexStringWithPadding(commandValue2, 0x08) << "\n";
						if (entryCursor >= entryAddress)
						{
							result = { entryCursor, commandCursor };
							finished = 1;
						}
						else
						{
							entryCursor += distanceToNext;
						}
					}
					commandCursor += 0x8;
				}
			}
			return result;
		}

		bool moduleFile::populate(std::string filePathIn)
		{
			std::ifstream inputStream(filePathIn, std::ios_base::in | std::ios_base::binary);
			if (inputStream.is_open())
			{
				fileBody.populate(inputStream);
				if (fileBody.populated())
				{
					ID = fileBody.getLong(0x00);
					sectionsCount = fileBody.getLong(0x0C);
					headerLength = fileBody.getLong(0x10);
					nameAddress = fileBody.getLong(0x14);
					nameLength = fileBody.getLong(0x14);
					version = fileBody.getLong(0x1C);
					importsAddress = fileBody.getLong(0x28);
					importsLength = fileBody.getLong(0x2C);
					importsCount = importsLength / 0x08;
					commandsAddress = fileBody.getLong(0x48);

					sectionsInfo.resize(sectionsCount);
					std::size_t sectionAddrTemp = SIZE_MAX;
					for (std::size_t i = 0; i < sectionsCount; i++)
					{
						sectionAddrTemp = fileBody.getLong(headerLength + (i * 0x08));
						sectionAddrTemp -= sectionAddrTemp % 4;
						sectionsInfo[i] = { sectionAddrTemp, fileBody.getLong(headerLength + (i * 0x08) + 0x04) };
					}
					importsInfo.resize(importsCount);

					bool foundModuleImport = 0;
					for (std::size_t i = 0; i < importsCount; i++)
					{
						importsInfo[i] = { fileBody.getLong(importsAddress + (i * 0x08)), fileBody.getLong(importsAddress + (i * 0x08) + 0x04) };
						if (foundModuleImport)
						{
							commandsLength = importsInfo[i].second - importsInfo[i - 1].second;
							foundModuleImport = 0;
						}
						if (importsInfo[i].first == ID)
						{
							foundModuleImport = 1;
						}
					}
				}
			}
			return fileBody.populated();
		}
		std::size_t moduleFile::getSectionContainingAddress(std::size_t address)
		{
			std::size_t result = SIZE_MAX;
			std::size_t currentSectionIndex = 0x01;
			while (result == SIZE_MAX && currentSectionIndex < sectionsCount)
			{
				if (address < getSectionInfo(currentSectionIndex).first)
				{
					result = currentSectionIndex - 1;
				}
				else
				{
					currentSectionIndex++;
				}
			}
			if (result == SIZE_MAX)
			{
				if (address < (sectionsInfo.back().first + sectionsInfo.back().second))
				{
					result = sectionsInfo.size() - 1;
				}
			}
			return result;
		}
		std::pair<size_t, std::size_t> moduleFile::getSectionInfo(std::size_t sectionIndex)
		{
			std::pair<std::size_t, std::size_t> result = { SIZE_MAX, SIZE_MAX };
			if (sectionIndex < sectionsCount)
			{
				result = sectionsInfo[sectionIndex];
			}
			return result;
		}

		std::pair<std::size_t, unsigned long long int> moduleFile::getLinkedCommand(std::size_t entryAddress)
		{
			std::pair<std::size_t, unsigned long long int> result = { SIZE_MAX, ULLONG_MAX };

			std::pair<std::size_t, std::size_t> linkedCommandLocation = _IterateThroughCommandsTill(entryAddress);

			if (linkedCommandLocation.first != SIZE_MAX && linkedCommandLocation.second != SIZE_MAX)
			{
				if (linkedCommandLocation.first == entryAddress)
				{
					result.first = linkedCommandLocation.second;
					result.second = fileBody.getLLong(result.first);
				}
			}

			return result;
		}
		std::size_t moduleFile::insertLinkedCommand(std::size_t entryAddress, unsigned long long int commandIn)
		{
			std::size_t result = SIZE_MAX;

			std::pair<std::size_t, std::size_t> linkedCommandLocation = _IterateThroughCommandsTill(entryAddress);

			if (linkedCommandLocation.first != SIZE_MAX && linkedCommandLocation.second != SIZE_MAX)
			{
				if (linkedCommandLocation.first != entryAddress)
				{
					unsigned short distanceFromPrev = fileBody.getShort(linkedCommandLocation.second);
					unsigned long long int newCommandDistanceFromPrev = entryAddress - (linkedCommandLocation.first - distanceFromPrev);
					commandIn &= 0x0000FFFFFFFFFFFF;
					commandIn += newCommandDistanceFromPrev << (0x08 * 0x06);
					fileBody.insertLLong(commandIn, linkedCommandLocation.second);
					fileBody.setShort(distanceFromPrev - newCommandDistanceFromPrev, linkedCommandLocation.second + 0x08);
					//std::cout << "\tWROTE COMMAND!\n";
					result = linkedCommandLocation.second;
				}
			}

			return result;
		}
		

		namespace kirbyhat
		{
			const std::string BuildFolder = ".././";
			const std::string GCTRMExePath = BuildFolder + "GCTRealMate.exe";
			const std::string GCTRMCommandBase = "\"" + GCTRMExePath + "\" -g -l -q ";
#define BUILD_NETPLAY_FILES
#ifdef BUILD_NETPLAY_FILES
			const std::string mainGCTName = "NETPLAY";
			const std::string boostGCTName = "NETBOOST";
#else
			const std::string mainGCTName = "RSBE01";
			const std::string boostGCTName = "BOOST";
#endif
			const std::string mainGCTFile = BuildFolder + mainGCTName + ".GCT";
			const std::string mainGCTTextFile = BuildFolder + mainGCTName + ".txt";
			const std::string boostGCTFile = BuildFolder + boostGCTName + ".GCT";
			const std::string boostGCTTextFile = BuildFolder + boostGCTName + ".txt";

			std::ofstream kirbyHatChangelogStream = std::ofstream();
			const std::string version = "v0.8.2";
			const std::string inputFilename = "EX_KirbyHats.txt";
			const std::string outputDirectory = "./EX_KirbyHats_Output/";
			const std::string relAutoplaceFilename = BuildFolder + "pf/module/ft_kirby.rel";
			const std::string kbxAutoplaceFilename = BuildFolder + "pf/BrawlEx/KirbyHat.kbx";
			const std::string khexASMAutoplaceFilename = BuildFolder + "Source/Extras/KirbyHatEX.asm";
			const std::string changelogFilename = outputDirectory + "EX_KirbyHats_Changelog.txt";
#ifdef USE_EX_PREFIX_FOR_INPUT
			const std::string relFilename = "EX_ft_kirby.rel";
			const std::string kbxFilename = "EX_KirbyHat.kbx";
			const std::string khexASMFilename = "EX_KirbyHatEX.asm";
#else
			const std::string relFilename = "ft_kirby.rel";
			const std::string kbxFilename = "KirbyHat.kbx";
			const std::string khexASMFilename = "KirbyHatEX.asm";
#endif
#ifdef USE_EDIT_SUFFIX_FOR_OUTPUT
			const std::string relEditFilename = lava::brawl::kirbyhat::outputDirectory + "ft_kirby_edit.rel";
			const std::string kbxEditFilename = lava::brawl::kirbyhat::outputDirectory + "KirbyHat_edit.kbx";
			const std::string khexASMEditFilename = lava::brawl::kirbyhat::outputDirectory + "KirbyHatEX_edit.asm";
#else
			const std::string relEditFilename = lava::brawl::kirbyhat::outputDirectory + "ft_kirby.rel";
			const std::string kbxEditFilename = lava::brawl::kirbyhat::outputDirectory + "KirbyHat.kbx";
			const std::string khexASMEditFilename = lava::brawl::kirbyhat::outputDirectory + "KirbyHatEX.asm";
#endif
			
			std::unordered_map<std::size_t, std::size_t> fighterIDToMapFuncID =
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
			std::unordered_map<std::size_t, std::string> kirbyHatFIDToName{};
			bool buildHatDictionaryFromKBX(lava::byteArray& kbxIn)
			{
				bool result = 0;

				if (kbxIn.populated())
				{
					result = 1;
					std::size_t hatActionVal = SIZE_MAX;
					std::size_t hatSubactionVal1 = SIZE_MAX;
					std::size_t hatSubactionVal2 = SIZE_MAX;
					std::size_t hatSubactionVal3 = SIZE_MAX;
					std::size_t hatSubactionVal4 = SIZE_MAX;
					for (std::size_t fighterID = 0x00; fighterID < maxFighterID; fighterID++)
					{
						hatActionVal = kbxIn.getLong(0x800 + (fighterID * 0x04));
						hatSubactionVal1 = kbxIn.getLong((fighterID * 0x10));
						hatSubactionVal2 = kbxIn.getLong((fighterID * 0x10) + 0x04);
						hatSubactionVal3 = kbxIn.getLong((fighterID * 0x10) + 0x08);
						hatSubactionVal4 = kbxIn.getLong((fighterID * 0x10) + 0x0C);
						if (hatSubactionVal1 || hatSubactionVal2 || hatSubactionVal3 || hatSubactionVal4)
						{
							// Certain character slots have entries with NULLED out hat data entries.
							// It seems that this configuration causes Kirby not gain an ability on a copy attempt.
							// I'm hoping that this is a functionality that can be lent to P+Ex characters to prevent crashing on copy attempts.
							// To facilitate this without bloating the hat summary, the only entry like this I'll be cataloguing is Kirby's
							if (fighterID == lava::brawl::LAVA_CHARA_FIGHTER_IDS::LCFI_KIRBY || (hatSubactionVal3 != 0x00000330 && hatActionVal != 0xFFFFFFFF))
							{
								std::unordered_map<std::size_t, std::string>::const_iterator nameItr =
									lava::brawl::LAVA_CHARA_FID_TO_NAME.find(fighterID);
								if (nameItr != lava::brawl::LAVA_CHARA_FID_TO_NAME.end())
								{
									lava::brawl::kirbyhat::kirbyHatFIDToName.insert(std::make_pair(fighterID, nameItr->second));
								}
								else
								{
									lava::brawl::kirbyhat::kirbyHatFIDToName.insert(std::make_pair(fighterID, "UNRECOGNIZED"));
								}
							}
						}
					}
				}

				return result;
			}

			std::vector<std::size_t> addCharacterFIDsAndNamesToMap(const std::vector<std::pair<std::string, std::pair<std::size_t, std::size_t>>>& toAdd)
			{
				std::vector<std::size_t> collidingIDs{};
				for (std::size_t i = 0; i < toAdd.size(); i++)
				{
					const std::pair<std::string, std::pair<std::size_t, std::size_t>>* currentHatPtr = &toAdd[i];
					std::pair<std::size_t, std::string> newEntry = std::make_pair(currentHatPtr->second.first, currentHatPtr->first);
					auto nameEmplaceRes = lava::brawl::kirbyhat::kirbyHatFIDToName.emplace(std::make_pair(currentHatPtr->second.first, currentHatPtr->first));
					if (!nameEmplaceRes.second)
					{
						std::cout << "[WARNING] Collision Occured on Fighter \"" << currentHatPtr->first << "\" (Fighter ID 0x" << lava::numToHexStringWithPadding(currentHatPtr->second.first, 0x04) << "):\n";
						std::cout << "\tNo hat will be added for this fighter.\n";
						collidingIDs.push_back(i);
					}
				}
				return collidingIDs;
			}

			std::pair<std::size_t, unsigned long long int> getTransactorBlockLinkInfo(lava::brawl::moduleFile& moduleIn)
			{
				static std::pair<std::size_t, unsigned long long int> transactorBlockCommandInfo = {SIZE_MAX, ULLONG_MAX};
				if (transactorBlockCommandInfo.first == SIZE_MAX || transactorBlockCommandInfo.second == ULLONG_MAX)
				{
					transactorBlockCommandInfo =
						moduleIn.getLinkedCommand(moduleIn.getSectionInfo(transactorBlockAddressLinkSection).first + transactorBlockAddressLinkLocation);
				}
				return transactorBlockCommandInfo;
			}
			/*std::vector<std::size_t> catalogueTransactors(lava::brawl::moduleFile& moduleIn, bool forceRefresh)
			{
				static std::vector<std::pair<std::size_t, std::size_t>> transactorEntries{};

				if (moduleIn.fileBody.populated())
				{
					std::unordered_map<std::size_t, std::size_t>::const_iterator itr = fighterIDToMapFuncID.find(hatCharID);
					if (itr != fighterIDToMapFuncID.end())
					{
						std::size_t mapFuncID = itr->second;

						std::pair<std::size_t, unsigned long long int> transactorBlockCommandInfo = getTransactorBlockLinkInfo(moduleIn);

						std::size_t transactorBlockSectionID = (transactorBlockCommandInfo.second >> (0x08 * 0x04)) & 0x000000FF;
						std::pair<std::size_t, std::size_t> transactorBlockSectionInfo = moduleIn.getSectionInfo(transactorBlockSectionID);
						std::size_t transactorBlockSectionAddress = transactorBlockSectionInfo.first;

						std::size_t deltaOff = transactorBlockCommandInfo.second;

						bool nullFound = 0;
						bool entryFound = 0;
						//std::vector<std::size_t> transactorEntries{};
						while (deltaOff < transactorBlockSectionInfo.second && !nullFound)
						{
							std::size_t valAtDeltaOff = moduleIn.fileBody.getLong(transactorBlockSectionAddress + deltaOff);
							switch (valAtDeltaOff)
							{
								case transactorNullTag:
								{
									nullFound = 1;
									break;
								}
								case transactorEntryTag:
								{
									entryFound = 1;
									std::size_t fighterID;
									std::size_t transactor;
									transactorEntries.push_back(moduleIn.fileBody.getLong(transactorBlockSectionAddress + deltaOff + 0x04));
									break;
								}
							default:
							{
								if (entryFound)
								{
									std::cerr << "[FAILURE] Hat Transactor Not Written. The alotted hat space is full.\n";
									std::cerr << "[FAILURE] Hat Command Not Written: The alotted hat space is full.\n";
								}
								else
								{
									std::cerr << "[FAILURE] Hat Transactor Not Written. REL is likely malformed, check it in BrawlCrate.\n";
									std::cerr << "[FAILURE] Hat Command Not Written. REL is likely malformed, check it in BrawlCrate.\n";
								}
								break;
							}
							}
							if (!nullFound)
							{
								deltaOff += 0x08;
							}
						}
					}
					else
					{
						kirbyHatChangelogStream << "\t\t" << "[SUCCESS] No transactor is associated with this ID (0x" <<
							lava::numToHexStringWithPadding(hatCharID, 0x04) << "). No REL edits necessary.\n";
					}
				}
			}*/

			bool addHatToKBX(lava::byteArray& kbxIn, std::size_t charID, std::size_t hatCharID)
			{
				bool result = 0;

				if (kbxIn.populated())
				{
					result = 1;

					std::size_t numGotten = 0;
					kirbyHatChangelogStream << "\t" << "Editing KBX...\n";

					std::vector<char> hatCharActionEntry{};
					hatCharActionEntry = kbxIn.getBytes(0x4, 0x800 + (hatCharID * 0x4), numGotten);
					kbxIn.setBytes(hatCharActionEntry, 0x800 + (charID * 0x4));
					kirbyHatChangelogStream << "\t\t" << "[SUCCESS] Wrote Hat Action (0x" <<
						lava::numToHexStringWithPadding(kbxIn.getLong(0x800 + (charID * 0x4)), 0x08) <<
						") to 0x" << lava::numToHexStringWithPadding(0x800 + (charID * 0x4), 0x04) << ".\n";

					std::vector<char> hatCharSubactionEntry{};
					hatCharSubactionEntry = kbxIn.getBytes(0x10, hatCharID * 0x10, numGotten);
					kbxIn.setBytes(hatCharSubactionEntry, charID * 0x10);
					kirbyHatChangelogStream << "\t\t" << "[SUCCESS] Wrote Hat Subaction (0x" <<
						lava::numToHexStringWithPadding(kbxIn.getLong(charID * 0x10), 0x08) <<
						" " << lava::numToHexStringWithPadding(kbxIn.getLong(charID * 0x10 + 0x04), 0x08) <<
						" " << lava::numToHexStringWithPadding(kbxIn.getLong(charID * 0x10 + 0x08), 0x08) <<
						" " << lava::numToHexStringWithPadding(kbxIn.getLong(charID * 0x10 + 0x0C), 0x08) <<
						") to 0x" << lava::numToHexStringWithPadding(charID * 0x10, 4) << ".\n";
				}

				return result;
			}
			bool addHatToREL(lava::brawl::moduleFile& moduleIn, std::size_t charID, std::size_t hatCharID)
			{
				bool result = 0;

				kirbyHatChangelogStream << "\t" << "Editing REL...\n";

				if (moduleIn.fileBody.populated())
				{
					result = 1;
					std::unordered_map<std::size_t, std::size_t>::const_iterator itr = fighterIDToMapFuncID.find(hatCharID);
					if (itr != fighterIDToMapFuncID.end())
					{
						std::size_t mapFuncID = itr->second;

						std::pair<std::size_t, unsigned long long int> transactorBlockCommandInfo = getTransactorBlockLinkInfo(moduleIn);

						std::size_t transactorBlockSectionID = (transactorBlockCommandInfo.second >> (0x08 * 0x04)) & 0x000000FF;
						std::pair<std::size_t, std::size_t> transactorBlockSectionInfo = moduleIn.getSectionInfo(transactorBlockSectionID);
						std::size_t transactorBlockSectionAddress = transactorBlockSectionInfo.first;

						std::size_t deltaOff = transactorBlockCommandInfo.second;

						bool nullFound = 0;
						bool entryFound = 0;
						std::vector<std::size_t> transactorEntries{};
						while (deltaOff < transactorBlockSectionInfo.second && !nullFound)
						{
							std::size_t valAtDeltaOff = moduleIn.fileBody.getLong(transactorBlockSectionAddress + deltaOff);
							switch (valAtDeltaOff)
							{
								case transactorNullTag:
								{
									nullFound = 1;
									break;
								}
								case transactorEntryTag:
								{
									entryFound = 1;
									transactorEntries.push_back(moduleIn.fileBody.getLong(transactorBlockSectionAddress + deltaOff + 0x04));
									break;
								}
								default:
								{
									if (entryFound)
									{
										std::cerr << "[FAILURE] Hat Transactor Not Written. The alotted hat space is full.\n";
										std::cerr << "[FAILURE] Hat Command Not Written: The alotted hat space is full.\n";
									}
									else
									{
										std::cerr << "[FAILURE] Hat Transactor Not Written. REL is likely malformed, check it in BrawlCrate.\n";
										std::cerr << "[FAILURE] Hat Command Not Written. REL is likely malformed, check it in BrawlCrate.\n";
									}
									break;
								}
							}
							if (!nullFound)
							{
								deltaOff += 0x08;
							}
						}

						if (nullFound)
						{
							std::size_t newEntry = 0x907F0000 + (charID * 0x04);
							moduleIn.fileBody.setLong(transactorEntryTag, transactorBlockSectionAddress + deltaOff);
							moduleIn.fileBody.setLong(newEntry, transactorBlockSectionAddress + deltaOff + 0x04);
							kirbyHatChangelogStream << "\t\t" << "[SUCCESS] Wrote Hat Transactor (0x" <<
								lava::numToHexStringWithPadding(transactorEntryTag, 0x08) << " " <<
								lava::numToHexStringWithPadding(newEntry, 0x08) << ") to 0x" <<
								lava::numToHexStringWithPadding(deltaOff, 0x04) << " of Section[" <<
								transactorBlockSectionID << "].\n";

							unsigned long long int newCommand = 0x00000A0100000000 + mapFuncID;
							std::size_t commandInsertAddress = moduleIn.insertLinkedCommand(transactorBlockSectionAddress + deltaOff, newCommand);
							kirbyHatChangelogStream << "\t\t" << "[SUCCESS] Wrote Hat Command (0x" <<
								lava::numToHexStringWithPadding(newCommand, 0x08) << ") to 0x" <<
								lava::numToHexStringWithPadding(commandInsertAddress, 0x08) << ".\n";

							moduleIn.importsInfo[2].second += 0x08;
							moduleIn.fileBody.setLong(moduleIn.importsInfo[2].second, moduleIn.importsAddress + 0x14);
						}
					}
					else
					{
						kirbyHatChangelogStream << "\t\t" << "[SUCCESS] No transactor is associated with this ID (0x" <<
							lava::numToHexStringWithPadding(hatCharID, 0x04) << "). No REL edits necessary.\n";
					}
				}
				return result;
			}
			bool addHatsToKHEXAsm(std::string asmPathIn, std::string asmEditPathIn, const std::vector<std::pair<std::string, std::pair<std::size_t, std::size_t>>>& toAdd)
			{
				bool result = 0;

				std::ifstream asmIn(asmPathIn, std::ios_base::in);
				std::ofstream asmOut(asmEditPathIn, std::ios_base::out);
				if (asmIn.is_open() && asmOut.is_open())
				{
					std::string currentLine = "";
					bool lastLineWasHatFloatLine = 0;
					while (std::getline(asmIn, currentLine))
					{
						if (!result && currentLine.find("%HatFloatFix") != std::string::npos)
						{
							lastLineWasHatFloatLine = 1;
						}
						else
						{
							if (lastLineWasHatFloatLine)
							{
								const std::pair<std::string, std::pair<std::size_t, std::size_t>>* currEntryPtr;
								std::string sourceCharName = "";
								for (std::size_t i = 0; i < toAdd.size(); i++)
								{
									currEntryPtr = &toAdd[i];
									sourceCharName = lava::brawl::kirbyhat::kirbyHatFIDToName.at(currEntryPtr->second.second);
									asmOut << "\t%HatFloatFix(0x" << lava::numToHexStringWithPadding(currEntryPtr->second.first, 0x02) << ", 0x" <<
										lava::numToHexStringWithPadding(currEntryPtr->second.second, 0x02) + ")";
									asmOut << "\t#" << currEntryPtr->first << "/" << sourceCharName << "\n";
								}
								result = 1;
							}
							lastLineWasHatFloatLine = 0;
						}
						asmOut << currentLine << "\n";
					}
				}

				return result;
			}

			void summarizeHats(std::ofstream& output, lava::brawl::moduleFile& moduleIn, lava::byteArray& kbxIn, std::vector<std::size_t> newIDs)
			{
				output << "Hat Summary:\n";

				if (moduleIn.fileBody.populated())
				{
					if (kbxIn.populated())
					{
						std::pair<std::size_t, unsigned long long int> transactorBlockCommandInfo = getTransactorBlockLinkInfo(moduleIn);

						std::size_t transactorBlockSectionID = (transactorBlockCommandInfo.second >> (0x08 * 0x04)) & 0x000000FF;
						std::pair<std::size_t, std::size_t> transactorBlockSectionInfo = moduleIn.getSectionInfo(transactorBlockSectionID);
						std::size_t transactorBlockSectionAddress = transactorBlockSectionInfo.first;

						std::size_t deltaOff = transactorBlockCommandInfo.second;

						bool nullFound = 0;
						bool entryFound = 0;
						std::vector<std::size_t> transactorEntries{};
						while (deltaOff < transactorBlockSectionInfo.second && !nullFound)
						{
							std::size_t valAtDeltaOff = moduleIn.fileBody.getLong(transactorBlockSectionAddress + deltaOff);
							switch (valAtDeltaOff)
							{
								case transactorNullTag:
								{
									nullFound = 1;
									break;
								}
								case transactorEntryTag:
								{
									entryFound = 1;
									transactorEntries.push_back(moduleIn.fileBody.getLong(transactorBlockSectionAddress + deltaOff + 0x04));
									break;
								}
								default:
								{
									break;
								}
							}
							if (!nullFound)
							{
								deltaOff += 0x08;
							}
						}

						std::size_t hatActionVal = SIZE_MAX;
						std::size_t hatSubactionVal1 = SIZE_MAX;
						std::size_t hatSubactionVal2 = SIZE_MAX;
						std::size_t hatSubactionVal3 = SIZE_MAX;
						std::size_t hatSubactionVal4 = SIZE_MAX;
						std::vector<std::size_t> noAbilityHats{};
						bool newEntry = 0;
						for (std::size_t fighterID = 0x00; fighterID < maxFighterID; fighterID++)
						{
							newEntry = std::find(newIDs.begin(), newIDs.end(), fighterID) != newIDs.end();
							hatActionVal = kbxIn.getLong(0x800 + (fighterID * 0x04));
							hatSubactionVal1 = kbxIn.getLong((fighterID * 0x10));
							hatSubactionVal2 = kbxIn.getLong((fighterID * 0x10) + 0x04);
							hatSubactionVal3 = kbxIn.getLong((fighterID * 0x10) + 0x08);
							hatSubactionVal4 = kbxIn.getLong((fighterID * 0x10) + 0x0C);
							if (hatSubactionVal1 || hatSubactionVal2 || hatSubactionVal3 || hatSubactionVal4)
							{
								// We want to avoid printing any hats with nulled out entry data, EXCEPT
								// Newly added entries (which can have nulled out entries now),
								// and Kirby, since being able to check newly nulled entries against Kirby is now important.
								if (newEntry || fighterID == LAVA_CHARA_FIGHTER_IDS::LCFI_KIRBY || (hatSubactionVal3 != 0x00000330 && hatActionVal != 0xFFFFFFFF))
								{
									output << "--------[ID 0x" << lava::numToHexStringWithPadding(fighterID, 0x4) << "] Name: ";
									std::unordered_map<std::size_t, std::string>::const_iterator nameItr =
										lava::brawl::kirbyhat::kirbyHatFIDToName.find(fighterID);
									if (nameItr != lava::brawl::kirbyhat::kirbyHatFIDToName.end())
									{
										output << nameItr->second << std::string(51 - nameItr->second.size(), '-');
									}
									else
									{
										output << "UNRECOGNIZED" << std::string(39, '-');
									}
									output << "\n";
									output << "\t\tAction Value:\t\t0x" << lava::numToHexStringWithPadding(hatActionVal, 0x04) << "\n";
									output << "\t\tSubaction Vals:\t\t" << "0x" << lava::numToHexStringWithPadding(hatSubactionVal1, 0x08) <<
										" " << lava::numToHexStringWithPadding(hatSubactionVal2, 0x08) <<
										" " << lava::numToHexStringWithPadding(hatSubactionVal3, 0x08) <<
										" " << lava::numToHexStringWithPadding(hatSubactionVal4, 0x08) << "\n";

									for (std::size_t transactorEntriesIndex = 0; transactorEntriesIndex < transactorEntries.size(); transactorEntriesIndex++)
									{
										std::size_t transactorFighterID = transactorEntries[transactorEntriesIndex] / 0x04;
										transactorFighterID &= 0x00000FFF;
										if (fighterID == transactorFighterID)
										{
											std::size_t currentFighterTransactorAddress =
												transactorBlockSectionAddress + transactorBlockCommandInfo.second + (0x08 * transactorEntriesIndex);
											std::pair<std::size_t, unsigned long long int> currentFighterCommandInfo =
												moduleIn.getLinkedCommand(currentFighterTransactorAddress);
											output << "\t\tTransactor Entry:\t0x" <<
												lava::numToHexStringWithPadding(transactorEntryTag, 0x08) << " " <<
												lava::numToHexStringWithPadding(transactorEntries[transactorEntriesIndex], 0x08) << "\n";
											output << "\t\tCommand Entry:\t\t0x" <<
												lava::numToHexStringWithPadding(currentFighterCommandInfo.second >> (0x08 * 0x04), 0x08) << " " <<
												lava::numToHexStringWithPadding(currentFighterCommandInfo.second, 0x08) << "\n";
										}
									}
								}
								else
								{
									noAbilityHats.push_back(fighterID);
								}
							}
						}
					}
				}
			}
		}
	}
}