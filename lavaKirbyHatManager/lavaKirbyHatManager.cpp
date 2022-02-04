#include "lavaKirbyHatManager.h"

namespace lava
{
	namespace brawl
	{
		namespace kirbyhat
		{
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

			bool addHatTransactor(lava::byteArray& moduleIn, std::size_t charID, std::size_t mapFuncID)
			{
				bool result = 0;

				if (moduleIn.populated())
				{
					if (transactorSectionAddress != SIZE_MAX)
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
							unsigned long long int newEntry = 0x48000001907F0000 + (charID * 0x04);
							std::size_t i = 0;
							bool matchFound = 0;
							while (i < transactorEntries.size() && !matchFound)
							{
								matchFound |= transactorEntries[i] == newEntry;
								i++;
							}
							if (!matchFound)
							{
								moduleIn.setLLong(newEntry, transactorSectionAddress + deltaOff);
								transactorEntries.push_back(newEntry);
							}
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
						}
					}
				}

				return result;
			}

			bool addHatCommand(lava::byteArray& moduleIn, std::size_t charID)
			{
				bool result = 0;

				if (moduleIn.populated())
				{
					std::size_t deltaOff = 0x00;
				}

				return result;
			}
		}
	}
}