#include <conio.h>
#include "lavaKirbyHatManager.h"

int stringToNum(const std::string& stringIn, bool allowNeg, int defaultVal)
{
	int result = defaultVal;
	std::string manipStr = stringIn;
	int base = (manipStr.find("0x") == 0) ? 16 : 10;
	char* res = nullptr;
	result = std::strtoul(manipStr.c_str(), &res, base);
	if (res != (manipStr.c_str() + manipStr.size()))
	{
		result = defaultVal;
	}
	if (result < 0 && !allowNeg)
	{
		result = defaultVal;
	}
	return result;
}

std::pair<std::size_t, std::size_t> parseIDPair(std::string pairStringIn)
{
	std::pair<std::size_t, std::size_t> result = { SIZE_MAX, SIZE_MAX };

	// Removes any space characters
	std::string manipStr = "";
	for (std::size_t i = 0; i < pairStringIn.size(); i++)
	{
		if (!std::isspace(pairStringIn[i]))
		{
			manipStr += pairStringIn[i];
		}
	}

	std::size_t delimLoc = manipStr.find(':');
	if (delimLoc != std::string::npos && delimLoc > 0 && delimLoc < (manipStr.size() - 1))
	{
		std::pair<std::size_t, std::size_t> newEntry = { SIZE_MAX, SIZE_MAX };
		newEntry.first = stringToNum(manipStr.substr(0, delimLoc), 1, SIZE_MAX);
		newEntry.second = stringToNum(manipStr.substr(delimLoc + 1), 1, SIZE_MAX);
		if (newEntry.first != SIZE_MAX)
		{
			if (newEntry.second != SIZE_MAX)
			{
				result = newEntry;
			}
		}
	}

	return result;
}
std::vector<std::pair<std::string, std::pair<std::size_t, std::size_t>>> parseInput(std::string inputFilePath, std::ostream& output)
{
	std::vector<std::pair<std::string, std::pair<std::size_t, std::size_t>>> result{};

	std::ifstream hatsIn(inputFilePath, std::ios_base::in);
	if (hatsIn.is_open())
	{
		std::string currentLine = "";
		std::string manipStr = "";
		while (std::getline(hatsIn, currentLine))
		{
			// Disregard the current line if it's empty, or is marked as a comment
			if (!currentLine.empty() && currentLine[0] != '#' && currentLine[0] != '/')
			{
				manipStr = "";
				bool inQuote = 0;
				bool doEscapeChar = 0;
				for (std::size_t i = 0; i < currentLine.size(); i++)
				{
					if (currentLine[i] == '\"' && !doEscapeChar)
					{
						inQuote = !inQuote;
					}
					else if (currentLine[i] == '\\')
					{
						doEscapeChar = 1;
					}
					else if (inQuote || !std::isspace(currentLine[i]))
					{
						doEscapeChar = 0;
						manipStr += currentLine[i];
					}
				}

				std::pair<std::string, std::pair<std::size_t, std::size_t>> newEntry = { "NEW_UNRECOGNIZED", {SIZE_MAX, SIZE_MAX} };

				std::size_t nameIDDelimLoc = manipStr.find('=');
				if (nameIDDelimLoc != std::string::npos && nameIDDelimLoc < (manipStr.size() - 1))
				{
					newEntry.first = manipStr.substr(0, nameIDDelimLoc);
				}
				manipStr = manipStr.substr(nameIDDelimLoc + 1, std::string::npos);
				newEntry.second = parseIDPair(manipStr);
				if (newEntry.second.first != SIZE_MAX)
				{
					if (newEntry.second.second != SIZE_MAX)
					{
						bool conflictFound = 0;
						std::size_t i = 0;
						while (!conflictFound && i < result.size())
						{
							if (newEntry.second.first == result[i].second.first)
							{
								conflictFound = 1;
							}
							else
							{
								i++;
							}
						}
						if (conflictFound)
						{
							std::pair<std::string, std::pair<std::size_t, std::size_t>>* conflictingElementPtr = &result[i];
							output << "[WARNING] Collision Occured in EX_KirbyHats.txt on Fighter ID 0x" << lava::numToHexStringWithPadding(newEntry.second.first, 0x04) << ":\n";
							output << "\tRemoving old entry \"" << conflictingElementPtr->first << "\", adding new entry \"" << newEntry.first << "\" (using Source Hat ID 0x" <<
								lava::numToHexStringWithPadding(newEntry.second.second, 0x04) << ").\n";
							result.erase(result.begin() + i);
						}
						output << "[LOADED] \"" << newEntry.first << "\" (Fighter ID 0x" << 
							lava::numToHexStringWithPadding(newEntry.second.first, 0x04) << ", Source ID 0x" <<
							lava::numToHexStringWithPadding(newEntry.second.second, 0x04) << "\n";
						result.push_back(newEntry);
					}
				}
			}
		}
	}

	return result;
}

int main()
{
	std::ofstream* kHCS = &lava::brawl::kirbyhat::kirbyHatChangelogStream;
	kHCS->open(lava::brawl::kirbyhat::outputDirectory + lava::brawl::kirbyhat::changelogFilename, std::ios_base::out);
	*kHCS << "lavaKirbyHatManager " << lava::brawl::kirbyhat::version << "\n";
	std::cerr << "lavaKirbyHatManager " << lava::brawl::kirbyhat::version << "\n";

	std::ifstream hatsIn;
	hatsIn.open(lava::brawl::kirbyhat::inputFilename, std::ios_base::in);
	if (hatsIn.is_open())
	{
		*kHCS << "\nLoading Hat Entries...\n";
		std::cerr << "\nLoading Hat Entries...\n";
		std::vector<std::pair<std::string, std::pair<std::size_t, std::size_t>>> toAdd = parseInput(lava::brawl::kirbyhat::inputFilename, *kHCS);

		lava::brawl::moduleFile relIn;
		bool doModuleEdit = relIn.populate(lava::brawl::kirbyhat::relFilename);
		if (!doModuleEdit)
		{
			*kHCS << "[ERROR] No REL file was found. Please check that an \"" << lava::brawl::kirbyhat::relFilename << "\" file exists in the same directory as this program and try again.\n";
			std::cerr << "[ERROR] No REL file was found. Please check that an \"" << lava::brawl::kirbyhat::relFilename << "\" file exists in the same directory as this program and try again.\n";
		}

		lava::byteArray kbxFile;
		std::ifstream kbxFileIn(lava::brawl::kirbyhat::kbxFilename, std::ios_base::in | std::ios_base::binary);
		bool doKBXEdit = 0;
		if (kbxFileIn.is_open())
		{
			kbxFile.populate(kbxFileIn);
			lava::brawl::kirbyhat::buildHatDictionaryFromKBX(kbxFile);
			doKBXEdit = 1;
		}
		if (!doKBXEdit)
		{
			*kHCS << "[ERROR] No KBX file was found. Please check that an \"" << lava::brawl::kirbyhat::kbxFilename << "\" file exists in the same directory as this program and try again.\n";
			std::cerr << "[ERROR] No KBX file was found. Please check that an \"" << lava::brawl::kirbyhat::kbxFilename << "\" file exists in the same directory as this program and try again.\n";
		}

		if (!toAdd.empty())
		{
			*kHCS << "\nAdding Hats...\n";

			std::vector<std::size_t> collisionIndeces = lava::brawl::kirbyhat::addCharacterFIDsAndNamesToMap(toAdd);

			for (std::size_t i = 0; i < toAdd.size(); i++)
			{
				std::pair<std::string, std::pair<std::size_t, std::size_t>>* currentHatPtr = &toAdd[i];
				*kHCS << "\"" << currentHatPtr->first << "\" ID: 0x" << lava::numToHexStringWithPadding(currentHatPtr->second.first, 0x04) << ", Source Character ID: 0x" << lava::numToHexStringWithPadding(currentHatPtr->second.second, 0x04) << "\n";

				if (std::find(collisionIndeces.begin(), collisionIndeces.end(), i) == collisionIndeces.end())
				{
					if (doKBXEdit)
					{
						lava::brawl::kirbyhat::addHatToKBX(kbxFile, currentHatPtr->second.first, currentHatPtr->second.second);
					}

					if (doModuleEdit)
					{
						lava::brawl::kirbyhat::addHatToREL(relIn, currentHatPtr->second.first, currentHatPtr->second.second);
					}
				}
				else
				{
					*kHCS << "\t[ERROR] Skipping Character: Fighter ID collided with \"" << 
						lava::brawl::kirbyhat::kirbyHatFIDToNameDict.find(currentHatPtr->second.first)->second << 
						"\" (ID 0x" << lava::numToHexStringWithPadding(currentHatPtr->second.first, 0x04) << ")\n";
				}
			}
			
			if (relIn.fileBody.populated())
			{
				relIn.fileBody.dumpToFile(lava::brawl::kirbyhat::outputDirectory + "ft_kirby_edit.rel");
			}
			if (kbxFile.populated())
			{
				kbxFile.dumpToFile(lava::brawl::kirbyhat::outputDirectory + "KirbyHat_edit.kbx");
			}
		}
		else
		{
			*kHCS << "[ERROR] No validly specified hats were found. Please check that there are valid entries in \"" << lava::brawl::kirbyhat::inputFilename << "\" and try again.\n";
			std::cerr << "[ERROR] No validly specified hats were found. Please check that there are valid entries in \"" << lava::brawl::kirbyhat::inputFilename << "\" and try again.\n";
		}

		if (relIn.fileBody.populated() && kbxFile.populated())
		{
			*kHCS << "\n";
			lava::brawl::kirbyhat::summarizeHats(*kHCS, relIn, kbxFile);
		}
		*kHCS << "\n";
	}
	else
	{
		*kHCS << "[ERROR] No input file was found. Please check that an \"" << lava::brawl::kirbyhat::inputFilename << "\" file exists in the same directory as this program and try again.\n";
		std::cerr << "[ERROR] No input file was found. Please check that an \"" << lava::brawl::kirbyhat::inputFilename << "\" file exists in the same directory as this program and try again.\n";
	}
	std::cout << "\nPress any key to exit.\n";
	_getch();
}