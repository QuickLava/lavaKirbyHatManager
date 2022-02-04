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
					transactorSectionAddress = moduleIn.getLong(sectionListBeginOffset + (0x8 * transactorSectionIndex));
					transactorSectionAddress -= transactorSectionAddress % 0x4;
					transactorSectionLength = moduleIn.getLong(sectionListBeginOffset + (0x8 * transactorSectionIndex) + 0x04);

					importSectionAddress = moduleIn.getLong(importSectionOffsetHeaderOffset);

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

			void summarizeHats2(std::ofstream& output, lava::byteArray& moduleIn, lava::byteArray& kbxIn)
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

									std::size_t currImportSectionLength = moduleIn.getLong(importSectionAddress + importSectionOffsetLengthOffset);
									currImportSectionLength += 0x08;
									moduleIn.setLong(currImportSectionLength, importSectionAddress + importSectionOffsetLengthOffset);
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
		}
	}
}