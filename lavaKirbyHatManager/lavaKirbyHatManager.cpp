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
				std::cout << "Searching for a command linked to 0x" << lava::numToHexStringWithPadding(entryAddress, 0x08) << " in Section[" << currentSectionIndex << "],,.\n";
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
						std::cout << "\t[0x" << lava::numToHexStringWithPadding(entryCursor, 0x08) << ", " << lava::numToHexStringWithPadding(commandCursor) << "] ";
						std::cout << lava::numToHexStringWithPadding(commandValue1, 0x08) << " " << lava::numToHexStringWithPadding(commandValue2, 0x08) << "\n";
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
		std::pair<size_t, std::size_t> moduleFile::getSectionInfo(std::size_t sectionIndex)
		{
			std::pair<std::size_t, std::size_t> result = { SIZE_MAX, SIZE_MAX };
			if (sectionIndex < sectionsCount)
			{
				result = sectionsInfo[sectionIndex];
			}
			return result;
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

		std::pair<std::size_t, unsigned long long int> moduleFile::getLinkedCommand(std::size_t entryAddress)
		{
			std::pair<std::size_t, unsigned long long int> result = {SIZE_MAX, ULLONG_MAX};

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
				std::cout << "Searching for a command linked to 0x" << lava::numToHexStringWithPadding(entryAddress, 0x08) << " in Section[" << currentSectionIndex << "],,.\n";
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
						std::cout << "\t[0x" << lava::numToHexStringWithPadding(entryCursor, 0x08) << ", " << lava::numToHexStringWithPadding(commandCursor) << "] ";
						std::cout << lava::numToHexStringWithPadding(commandValue1, 0x08) << " " << lava::numToHexStringWithPadding(commandValue2, 0x08) << "\n";
						if (entryCursor == entryAddress)
						{
							std::cout << "\tFOUND TARGET!\n";	
							result = {commandCursor, fileBody.getLLong(commandCursor) };
							finished = 1;
						}
						else if (entryCursor < entryAddress)
						{
							entryCursor += distanceToNext;
						}
						else
						{
							std::cout << "botched\n";
						}
					}
					commandCursor += 0x8;
				}
			}
			
			return result;
		}

		std::pair<std::size_t, unsigned long long int> moduleFile::neoGetLinkedCommand(std::size_t entryAddress)
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
				std::cout << "Searching for a command linked to 0x" << lava::numToHexStringWithPadding(entryAddress, 0x08) << " in Section[" << currentSectionIndex << "],,.\n";
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
						std::cout << "\t[0x" << lava::numToHexStringWithPadding(entryCursor, 0x08) << ", " << lava::numToHexStringWithPadding(commandCursor) << "] ";
						std::cout << lava::numToHexStringWithPadding(commandValue1, 0x08) << " " << lava::numToHexStringWithPadding(commandValue2, 0x08) << "\n";
						if (entryCursor > entryAddress)
						{
							unsigned short distanceFromPrev = fileBody.getShort(commandCursor);
							unsigned long long int newCommandDistanceFromPrev = entryAddress - (entryCursor - distanceFromPrev);
							commandIn &= 0x0000FFFFFFFFFFFF;
							commandIn += newCommandDistanceFromPrev << (0x08 * 0x06);
							fileBody.insertLLong(commandIn, commandCursor);
							fileBody.setShort(distanceFromPrev - newCommandDistanceFromPrev, commandCursor + 0x08);
							std::cout << "\tWROTE COMMAND!\n";
							result = commandCursor;
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

		std::size_t moduleFile::neoInsertLinkedCommand(std::size_t entryAddress, unsigned long long int commandIn)
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
					std::cout << "\tWROTE COMMAND!\n";
					result = linkedCommandLocation.second;
				}
			}

			return result;
		}

		namespace kirbyhat
		{

			std::ofstream kirbyHatChangelogStream = std::ofstream();
			std::string outputDirectory = "./EX_KirbyHats_Output/";

			const std::string version = "v0.1";

			std::size_t transactorSectionAddress = SIZE_MAX;
			std::size_t transactorSectionLength = 0x00;
			std::size_t importSectionAddress = SIZE_MAX;
			std::size_t mewtwoTransactorLinkEntryAddress = SIZE_MAX;

			bool setup(lava::byteArray& moduleIn)
			{
				bool result = SIZE_MAX;

				if (moduleIn.populated())
				{
					result = 1;
					traceForwardsThroughCommands(moduleIn);
					transactorSectionAddress = moduleIn.getLong(sectionListBeginOffset + (0x8 * transactorSectionIndex));
					transactorSectionAddress -= transactorSectionAddress % 0x4;
					transactorSectionLength = moduleIn.getLong(sectionListBeginOffset + (0x8 * transactorSectionIndex) + 0x04);

					importSectionAddress = moduleIn.getLong(importSectionAddressHeaderOffset);

					mewtwoTransactorLinkEntryAddress = moduleIn.searchLLong(mewtwoTransactorLinkEntry, importSectionAddress);
				}

				return result;
			}
			
			/*void summarizeHats(std::ofstream& output, lava::byteArray& moduleIn)
			{
				output << "Hat Summary:\n";
				output << "[Note:]\n";

				if (moduleIn.populated())
				{
					if (transactorSectionAddress != SIZE_MAX)
					{
						if (mewtwoTransactorLinkEntryAddress != SIZE_MAX)
						{
							std::size_t deltaOff = 0x00;
							bool nullFound = 0;
							std::vector<unsigned long long int> transactorEntries{};
							while (deltaOff < transactorSectionLength && !nullFound)
							{
								std::size_t valAtDeltaOff = moduleIn.getLong(transactorSectionAddress + deltaOff);
								switch (valAtDeltaOff)
								{
									case transactorNullCommand:
									{
										nullFound = 1;
										break;
									}
									case transactorEntryTag:
									{
										transactorEntries.push_back(moduleIn.getLLong(transactorSectionAddress + deltaOff));
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
							if (transactorEntries.size())
							{
								for (std::size_t i = 0; i < transactorEntries.size(); i++)
								{
									std::size_t fighterID = transactorEntries[i] & 0x0000000000000FFF;
									fighterID /= 4;
									std::size_t commandAddress = mewtwoTransactorLinkEntryAddress + (0x08 * i);
									std::size_t funcID = moduleIn.getLong(commandAddress + 0x04);
									output << "\t" << "[" << lava::numToHexStringWithPadding(i, 0x02) << "] Fighter ID: 0x" <<
										lava::numToHexStringWithPadding(fighterID, 0x04) << ", Command: 0x" <<
										lava::numToHexStringWithPadding(funcID, 0x08) << "\n";
								}
							}
							else
							{
								kirbyHatChangelogStream << "\t\t" << "[ERROR] Unable to locate hat entries in this REL. Ensure that this is a P+EX Kirby module and try again.\n";
							}
						}
					}
				}
			}*/

			void summarizeHats(std::ofstream& output, lava::byteArray& moduleIn, lava::byteArray& kbxIn)
			{
				output << "Hat Summary:\n";

				if (moduleIn.populated())
				{
					if (kbxIn.populated())
					{
						if (transactorSectionAddress != SIZE_MAX)
						{
							if (mewtwoTransactorLinkEntryAddress != SIZE_MAX)
							{
								std::size_t deltaOff = 0x00;
								bool nullFound = 0;
								std::vector<std::pair<std::size_t, std::size_t>> transactorEntries{};
								while (deltaOff < transactorSectionLength && !nullFound)
								{
									std::size_t valAtDeltaOff = moduleIn.getLong(transactorSectionAddress + deltaOff);
									switch (valAtDeltaOff)
									{
										case transactorNullCommand:
										{
											nullFound = 1;
											break;
										}
										case transactorEntryTag:
										{
											transactorEntries.push_back({ valAtDeltaOff, moduleIn.getLong(transactorSectionAddress + deltaOff + 0x04)});
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
								if (transactorEntries.empty())
								{
									kirbyHatChangelogStream << "\t\t" << "[ERROR] Unable to locate hat entries in this REL. Ensure that this is a P+EX Kirby module and try again.\n";
								}

								std::size_t hatActionVal = SIZE_MAX;
								std::size_t hatSubactionVal1 = SIZE_MAX;
								std::size_t hatSubactionVal2 = SIZE_MAX;
								std::size_t hatSubactionVal3 = SIZE_MAX;
								std::size_t hatSubactionVal4 = SIZE_MAX;
								std::vector<std::size_t> noAbilityHats{};
								for (std::size_t fighterID = 0x00; fighterID < maxFighterID; fighterID++)
								{
									hatActionVal = kbxIn.getLong(0x800 + (fighterID * 0x04));
									hatSubactionVal1 = kbxIn.getLong((fighterID * 0x10));
									hatSubactionVal2 = kbxIn.getLong((fighterID * 0x10) + 0x04);
									hatSubactionVal3 = kbxIn.getLong((fighterID * 0x10) + 0x08);
									hatSubactionVal4 = kbxIn.getLong((fighterID * 0x10) + 0x0C);
									if (hatSubactionVal1 || hatSubactionVal2 || hatSubactionVal3 || hatSubactionVal4)
									{
										if (hatSubactionVal3 != 0x00000330 && hatActionVal != 0xFFFFFFFF)
										{
											output << "--------[ID 0x" << lava::numToHexStringWithPadding(fighterID, 0x4) << "] Name: ";
											std::unordered_map<std::size_t, std::string>::const_iterator nameItr = lava::brawl::fighterIDToName.find(fighterID);
											if (nameItr != lava::brawl::fighterIDToName.end())
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
												std::size_t transactorFighterID = transactorEntries[transactorEntriesIndex].second / 0x04;
												transactorFighterID &= 0x00000FFF;
												if (fighterID == transactorFighterID)
												{
													std::size_t commandAddress = mewtwoTransactorLinkEntryAddress + (0x08 * transactorEntriesIndex);
													std::size_t funcID = moduleIn.getLong(commandAddress + 0x04);
													output << "\t\tTransactor Entry:\t0x" <<
														lava::numToHexStringWithPadding(transactorEntries[transactorEntriesIndex].first, 0x08) << " " <<
														lava::numToHexStringWithPadding(transactorEntries[transactorEntriesIndex].second, 0x08) << "\n";
													output << "\t\tCommand Entry:\t\t0x" << lava::numToHexStringWithPadding(funcID, 0x08) << "\n";
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
			

			std::size_t getSectionOffset(lava::byteArray& moduleIn, std::size_t sectionIndex)
			{
				std::size_t result = moduleIn.getLong(sectionListBeginOffset + (0x8 * sectionIndex));
				result -= result % 0x4;
				return result;
			}
			std::size_t getSectionSize(lava::byteArray& moduleIn, std::size_t sectionIndex)
			{
				std::size_t result = moduleIn.getLong(sectionListBeginOffset + (0x8 * sectionIndex) + 0x04);
				return result;
			}

			unsigned long long getCommandInfoForEntry(lava::byteArray& moduleIn, std::size_t entryAddress)
			{
				std::size_t currentSection = SIZE_MAX;
				std::size_t sectionIndex = 0x00;
				while (currentSection == SIZE_MAX && sectionIndex)
				{
					if (entryAddress < getSectionOffset(moduleIn, sectionIndex))
					{
						currentSection = sectionIndex;
					}
				}

				std::size_t commandCursor = moduleIn.getLong(commandSectionAddressHeaderOffset);
				std::size_t commandValue1 = 0x00;
				std::size_t commandValue2 = 0x00;
				std::size_t entryCursor = getSectionSize(moduleIn, currentSection);
				unsigned short distanceToNext = 0x00;
				std::cout << "Starting in Section[" << currentSection << "]\n";
				bool finished = 0;
				while (!finished && commandCursor < moduleIn.body.size())
				{
					distanceToNext = moduleIn.getShort(commandCursor + 0x08);
					commandValue1 = moduleIn.getLong(commandCursor);
					commandValue2 = moduleIn.getLong(commandCursor + 0x04);
					if ((commandValue1 & 0xFFFFFFF0) == 0x0000CA00 && commandValue2 == 0x00000000)
					{
						if (currentSection <= (commandValue1 & 0x0000000F))
						{
							currentSection = commandValue1 & 0x0000000F;
							entryCursor = getSectionOffset(moduleIn, currentSection);
							std::cout << "Switched to Section[" << currentSection << "]\n";
						}
						else
						{
							finished = 1;
						}
					}
					else
					{
						std::cout << "[0x" << lava::numToHexStringWithPadding(entryCursor, 0x08) << ", " << lava::numToHexStringWithPadding(commandCursor) << "] ";
						std::cout << lava::numToHexStringWithPadding(commandValue1, 0x08) << " " << lava::numToHexStringWithPadding(commandValue2, 0x08) << "\n";
						entryCursor += distanceToNext;
					}
					commandCursor += 0x8;
				}

				return 0ull;
			}

			void traceForwardsThroughCommands(lava::byteArray& moduleIn)
			{
				std::size_t currentSection = 0x01;
				
				std::size_t commandCursor = moduleIn.getLong(commandSectionAddressHeaderOffset);
				std::size_t commandValue1 = 0x00;
				std::size_t commandValue2 = 0x00;
				std::size_t entryCursor = getSectionSize(moduleIn, currentSection);
				unsigned short distanceToNext = 0x00;
				std::cout << "Starting in Section[" << currentSection << "]\n";
				bool finished = 0;
				while (!finished && commandCursor < moduleIn.body.size())
				{
					distanceToNext = moduleIn.getShort(commandCursor + 0x08);
					commandValue1 = moduleIn.getLong(commandCursor);
					commandValue2 = moduleIn.getLong(commandCursor + 0x04);
					if ((commandValue1 & 0xFFFFFFF0) == 0x0000CA00 && commandValue2 == 0x00000000)
					{
						if (currentSection <= (commandValue1 & 0x0000000F))
						{
							currentSection = commandValue1 & 0x0000000F;
							entryCursor = getSectionOffset(moduleIn, currentSection);
							std::cout << "Switched to Section[" << currentSection << "]\n";
						}
						else
						{
							finished = 1;
						}
					}
					else
					{
						std::cout << "[0x" << lava::numToHexStringWithPadding(entryCursor, 0x08) << ", " << lava::numToHexStringWithPadding(commandCursor) << "] ";
						std::cout << lava::numToHexStringWithPadding(commandValue1, 0x08) << " " << lava::numToHexStringWithPadding(commandValue2, 0x08) << "\n";
						entryCursor += distanceToNext;
					}
					commandCursor += 0x8;
				}
			}

			void traceBackwardsThroughCommands(lava::byteArray& moduleIn, std::size_t firstCommandEntryOffset, std::size_t firstCommandOffset, std::size_t currSectionIn)
			{
				std::size_t currentSection = currSectionIn;
				std::size_t currentSectionOffset = getSectionOffset(moduleIn, currentSection);
				std::size_t currentSectionLength = getSectionSize(moduleIn, currentSection);
				std::size_t commandCursor = firstCommandOffset;
				std::size_t entryCursor = firstCommandEntryOffset;
				unsigned short distanceFromPrev = 0x00;
				std::cout << "Starting in Section[" << currentSection << "]\n";
				while (entryCursor <= (entryCursor + distanceFromPrev) && commandCursor < (commandCursor + 8))
				{
					std::cout << "[0x" << lava::numToHexStringWithPadding(entryCursor, 0x08) << ", " << lava::numToHexStringWithPadding(commandCursor) << "] ";
					std::cout << lava::numToHexStringWithPadding(moduleIn.getLong(commandCursor), 0x08) << " " << lava::numToHexStringWithPadding(moduleIn.getLong(commandCursor + 0x04), 0x08) << "\n";
					distanceFromPrev = moduleIn.getShort(commandCursor);
					if (entryCursor <= currentSectionOffset && ((currentSection - 1) <= currentSection) )
					{
						currentSection -= 1;
						currentSectionOffset = getSectionOffset(moduleIn, currentSection);
						currentSectionLength = getSectionSize(moduleIn, currentSection);
						entryCursor = currentSectionOffset + currentSectionLength - 0x08;
						std::cout << "Switched to Section[" << currentSection << "]\n";
					}
					else
					{
						entryCursor -= distanceFromPrev;
					}
					commandCursor -= 8;
				}
			}

			bool addHatToREL(lava::byteArray& moduleIn, std::size_t charID, std::size_t hatCharID)
			{
				bool result = 0;

				kirbyHatChangelogStream << "\t" << "Editing REL...\n";

				if (moduleIn.populated())
				{
					if (transactorSectionAddress != SIZE_MAX)
					{
						std::unordered_map<std::size_t, std::size_t>::const_iterator itr = fighterIDToMapFuncID.find(hatCharID);
						if (itr != fighterIDToMapFuncID.end())
						{
							std::size_t mapFuncID = itr->second;

							std::size_t deltaOff = 0x00;
							bool nullFound = 0;
							std::vector<std::size_t> transactorEntries{};
							while (deltaOff < transactorSectionLength && !nullFound)
							{
								std::size_t valAtDeltaOff = moduleIn.getLong(transactorSectionAddress + deltaOff);
								switch (valAtDeltaOff)
								{
									case transactorNullCommand:
									{
										nullFound = 1;
										break;
									}
									case transactorEntryTag:
									{
										traceBackwardsThroughCommands(moduleIn, transactorSectionAddress + deltaOff, mewtwoTransactorLinkEntryAddress, transactorSectionIndex);
										transactorEntries.push_back(moduleIn.getLong(transactorSectionAddress + deltaOff + 0x04));
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
							if (transactorEntries.size())
							{
								std::size_t newEntry = 0x907F0000 + (charID * 0x04);
								std::size_t transactorEntriesIndex = 0;
								bool matchFound = 0;
								while (transactorEntriesIndex < transactorEntries.size() && !matchFound)
								{
									matchFound |= transactorEntries[transactorEntriesIndex] == newEntry;
									transactorEntriesIndex++;
								}
								if (!matchFound)
								{
									moduleIn.setLong(transactorEntryTag, transactorSectionAddress + deltaOff);
									moduleIn.setLong(newEntry, transactorSectionAddress + deltaOff + 0x04);
									transactorEntries.push_back(newEntry);
									kirbyHatChangelogStream << "\t\t" << "[SUCCESS] Wrote Hat Transactor (0x" <<
										lava::numToHexStringWithPadding(transactorEntryTag, 0x08) << " " <<
										lava::numToHexStringWithPadding(newEntry, 0x08) << ") to 0x" <<
										lava::numToHexStringWithPadding(deltaOff, 0x04) << " of Section[" << 
										transactorSectionIndex << "].\n";
								}
								if (mewtwoTransactorLinkEntryAddress != SIZE_MAX)
								{
									std::size_t commandInsertAddress = mewtwoTransactorLinkEntryAddress + (0x08 * (transactorEntries.size() - 1));
									unsigned long long int newCommand = 0x00080A0100000000 + mapFuncID;
									moduleIn.insertLLong(newCommand, commandInsertAddress);

									unsigned short nextCommandDistanceFromPrev = moduleIn.getShort(commandInsertAddress + 0x08);
									nextCommandDistanceFromPrev -= 0x08;
									moduleIn.setShort(nextCommandDistanceFromPrev, commandInsertAddress + 0x08);

									std::size_t currImportSectionLength = moduleIn.getLong(importSectionAddress + importSectionAddressLengthOffset);
									currImportSectionLength += 0x08;
									moduleIn.setLong(currImportSectionLength, importSectionAddress + importSectionAddressLengthOffset);
									kirbyHatChangelogStream << "\t\t" << "[SUCCESS] Wrote Hat Command (0x" <<
										lava::numToHexStringWithPadding(newCommand, 0x08) << ") to 0x" << 
										lava::numToHexStringWithPadding(commandInsertAddress, 0x08) << ".\n";
								}
							}
							else
							{
								kirbyHatChangelogStream << "\t\t" << "[ERROR] Unable to locate hat entries in this REL. Ensure that this is a P+EX Kirby module and try again.\n";
							}
						}
						else
						{
							kirbyHatChangelogStream << "\t\t" << "[SUCCESS] No transactor is associated with this ID (0x" << 
								lava::numToHexStringWithPadding(hatCharID, 0x04) << "). No REL edits necessary.\n";
						}
					}
				}

				return result;
			}
			bool addHatToKBX(lava::byteArray& kbxIn, std::size_t charID, std::size_t hatCharID)
			{
				bool result = 0;

				if (kbxIn.populated())
				{
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
			bool addHatToASM(std::ifstream& asmIn, std::size_t charID, std::size_t hatCharID)
			{
				bool result = 0;

				if (asmIn.is_open())
				{
					std::string currentLine = "";
					while (std::getline(asmIn, currentLine))
					{
						if (currentLine.find("Kirby Hat Float Fix") != std::string::npos )
						{

						}
					}
				}

				return result;
			}

			std::pair<std::size_t, unsigned long long int> getTransactorBlockLinkInfo(lava::brawl::moduleFile& moduleIn)
			{
				static std::pair<std::size_t, unsigned long long int> transactorBlockCommandInfo = {SIZE_MAX, ULLONG_MAX};
				if (transactorBlockCommandInfo.first == SIZE_MAX || transactorBlockCommandInfo.second == ULLONG_MAX)
				{
					transactorBlockCommandInfo =
						moduleIn.neoGetLinkedCommand(moduleIn.getSectionInfo(transactorBlockAddressLinkSection).first + transactorBlockAddressLinkLocation);
				}
				return transactorBlockCommandInfo;
			}
			bool neoAddHatToREL(lava::brawl::moduleFile& moduleIn, std::size_t charID, std::size_t hatCharID)
			{
				bool result = 0;

				kirbyHatChangelogStream << "\t" << "Editing REL...\n";

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
						std::vector<std::size_t> transactorEntries{};
						while (deltaOff < transactorBlockSectionInfo.second && !nullFound)
						{
							std::size_t valAtDeltaOff = moduleIn.fileBody.getLong(transactorBlockSectionAddress + deltaOff);
							switch (valAtDeltaOff)
							{
								case transactorNullCommand:
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
								transactorSectionIndex << "].\n";

							unsigned long long int newCommand = 0x00000A0100000000 + mapFuncID;
							std::size_t commandInsertAddress = moduleIn.neoInsertLinkedCommand(transactorBlockSectionAddress + deltaOff, newCommand);
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
			void neoSummarizeHats(std::ofstream& output, lava::brawl::moduleFile& moduleIn, lava::byteArray& kbxIn)
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
								case transactorNullCommand:
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
						for (std::size_t fighterID = 0x00; fighterID < maxFighterID; fighterID++)
						{
							hatActionVal = kbxIn.getLong(0x800 + (fighterID * 0x04));
							hatSubactionVal1 = kbxIn.getLong((fighterID * 0x10));
							hatSubactionVal2 = kbxIn.getLong((fighterID * 0x10) + 0x04);
							hatSubactionVal3 = kbxIn.getLong((fighterID * 0x10) + 0x08);
							hatSubactionVal4 = kbxIn.getLong((fighterID * 0x10) + 0x0C);
							if (hatSubactionVal1 || hatSubactionVal2 || hatSubactionVal3 || hatSubactionVal4)
							{
								if (hatSubactionVal3 != 0x00000330 && hatActionVal != 0xFFFFFFFF)
								{
									output << "--------[ID 0x" << lava::numToHexStringWithPadding(fighterID, 0x4) << "] Name: ";
									std::unordered_map<std::size_t, std::string>::const_iterator nameItr = lava::brawl::fighterIDToName.find(fighterID);
									if (nameItr != lava::brawl::fighterIDToName.end())
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
												moduleIn.neoGetLinkedCommand(currentFighterTransactorAddress);
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