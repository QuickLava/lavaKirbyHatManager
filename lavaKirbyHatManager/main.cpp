#include <conio.h>
#include "lavaKirbyHatManager.h"

// Variaibles for forcing decisions to certain values, allowing the program to be used without interaction.
int KBXCopyOverride = INT_MAX;
int RELCopyOverride = INT_MAX;
int ASMCopyOverride = INT_MAX;
int GCTBuildOverride = INT_MAX;
int CloseOnFinishBypass = INT_MAX;
int DisableNamesInASM = INT_MAX;
enum argumentIDs
{
	aI_KBX = 1,
	aI_REL,
	aI_ASM,
	aI_GCT,
	aI_CLOSE,
	aI_ASM_NAMES,
	argumentCount
};

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
bool yesNoDecision(char yesKey, char noKey)
{
	char keyIn = ' ';
	yesKey = std::tolower(yesKey);
	noKey = std::tolower(noKey);
	while (keyIn != yesKey && keyIn != noKey)
	{
		keyIn = _getch();
		keyIn = std::tolower(keyIn);
	}
	return (keyIn == yesKey);
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
bool validateNewHatEntry(const std::pair<std::string, std::pair<std::size_t, std::size_t>>& pairIn, std::ostream& output)
{
	bool result = 0;
	std::string errorMessage = "";
	if (pairIn.second.first != SIZE_MAX)
	{
		if (pairIn.second.second != SIZE_MAX)
		{
			if (pairIn.second.first <= lava::brawl::kirbyhat::maxFighterID)
			{
				if (lava::brawl::kirbyhat::kirbyHatFIDToName.find(pairIn.second.second) != lava::brawl::kirbyhat::kirbyHatFIDToName.end())
				{
					errorMessage = "";
					result = 1;
				}
				else
				{
					errorMessage = "Specified Source ID currently has no Kirby Hat; there is nothing to build this Character's Hat from.\n";
				}
			}
			else
			{
				errorMessage = "Specified Fighter ID is too high. Maximum valid value is 0x" + 
					lava::numToHexStringWithPadding(lava::brawl::kirbyhat::maxFighterID, 0x04) + ".\n";
			}
		}
		else
		{
			errorMessage = "Specified Source ID is NULL (0x" + lava::numToHexStringWithPadding(pairIn.second.second) + ").\n";
		}
	}
	else
	{
		errorMessage = "Specified Fighter ID is NULL (0x" + lava::numToHexStringWithPadding(pairIn.second.second) + ").\n";
	}
	if (result == 0)
	{
		output << "[ERROR] \"" << pairIn.first << "\" (Fighter ID 0x" <<
			lava::numToHexStringWithPadding(pairIn.second.first, 0x04) << ", Source ID 0x" <<
			lava::numToHexStringWithPadding(pairIn.second.second, 0x04) << ")\n";
		output << "\t" << errorMessage;
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
				if (validateNewHatEntry(newEntry, output))
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
						lava::numToHexStringWithPadding(newEntry.second.second, 0x04) << ")\n";
					result.push_back(newEntry);
				}
			}
		}
	}

	return result;
}

bool offerCopyOverAndBackup(std::string fileToCopy, std::string fileToOverwrite, int decisionOverride = INT_MAX)
{
	bool backupSucceeded = 0;
	bool copySucceeded = 0;

	std::cout << "Detected \"" << fileToOverwrite << "\".\n" <<
		"Would you like to copy \"" << fileToCopy << "\" over it? " <<
		"A backup will be made of the existing file.\n";
	std::cout << "[Press 'Y' for Yes, 'N' for No]\n";

	if ((decisionOverride == INT_MAX && yesNoDecision('y', 'n')) || (decisionOverride != INT_MAX && decisionOverride != 0))
	{
		std::cout << "Making backup... ";
		if (lava::backupFile(fileToOverwrite, ".bak", 1))
		{
			backupSucceeded = 1;
			std::cout << "Success!\nCopying over \"" << fileToCopy << "\"... ";
			if (lava::copyFile(fileToCopy, fileToOverwrite, 1))
			{
				copySucceeded = 1;
				std::cout << "Success!\n";
			}
			else
			{
				std::cerr << "Failure! Please ensure that the destination file is able to be written to!\n";
			}
		}
		else
		{
			std::cerr << "Backup failed! Please ensure that " << fileToOverwrite << ".bak is able to be written to!\n";
		}
	}
	else
	{
		std::cout << "Skipping copy.\n";
	}
	return backupSucceeded && copySucceeded;
}

bool handleAutoGCTRMProcess(std::string khexASMOutputLocation, int decisionOverride = INT_MAX)
{
	bool result = 0;

	if (lava::fileExists(lava::brawl::kirbyhat::GCTRMExePath) && 
		lava::fileExists(lava::brawl::kirbyhat::mainGCTTextFile) && lava::fileExists(lava::brawl::kirbyhat::boostGCTTextFile))
	{
		std::cout << "\nDetected \"" << lava::brawl::kirbyhat::GCTRMExePath << "\".\nWould you like to build \"" <<
			lava::brawl::kirbyhat::mainGCTFile << "\" and \"" << lava::brawl::kirbyhat::boostGCTFile << "\"?" <<
			" Backups will be made of the existing files.\n";
		std::cout << "[Press 'Y' for Yes, 'N' for No]\n";
		if ((decisionOverride == INT_MAX && yesNoDecision('y', 'n')) || (decisionOverride != INT_MAX && decisionOverride != 0))
		{
			std::cout << "Backing up files... ";
			if (lava::backupFile(lava::brawl::kirbyhat::mainGCTFile, ".bak", 1) && lava::backupFile(lava::brawl::kirbyhat::boostGCTFile, ".bak", 1))
			{
				std::cout << "Success! Running GCTRM.\n";
				result = 1;
				std::string commandFull = "\"" +  lava::brawl::kirbyhat::GCTRMCommandBase + "\"" + lava::brawl::kirbyhat::mainGCTTextFile + "\"\"";
				std::cout << "\n" << commandFull << "\n";
				system(commandFull.c_str());
				commandFull = "\"" + lava::brawl::kirbyhat::GCTRMCommandBase + "\"" + lava::brawl::kirbyhat::boostGCTTextFile + "\"\"";
				std::cout << "\n" << commandFull << "\n";
				system(commandFull.c_str());
				lava::brawl::kirbyhat::kirbyHatChangelogStream << "\nNote: Backed up and rebuilt \"" << lava::brawl::kirbyhat::mainGCTFile << "\".";
				lava::brawl::kirbyhat::kirbyHatChangelogStream << "\nNote: Backed up and rebuilt \"" << lava::brawl::kirbyhat::boostGCTFile << "\".\n";
			}
			else
			{
				std::cerr << "Something went wrong while backing up the files. Skipping GCTRM.\n";
			}
		}
		else
		{
			std::cout << "Skipping GCTRM.\n";
		}
	}
	return result;
}

int main(int argc, char** argv)
{
	if (argc > 1)
	{
		for (unsigned long i = 1; i < argc && i < argumentIDs::argumentCount; i++)
		{
			if (std::strcmp("-", argv[i]) != 0)
			{
				try
				{
					bool decisionVal = !(std::stoi(argv[i]) == 0);
					switch (i)
					{
						case argumentIDs::aI_KBX:
						{
							KBXCopyOverride = decisionVal;
							std::cout << "[C.ARG] Forcing KBX decision to: ";
							break;
						}
						case argumentIDs::aI_REL:
						{
							RELCopyOverride = decisionVal;
							std::cout << "[C.ARG] Forcing REL decision to: ";
							break;
						}
						case argumentIDs::aI_ASM:
						{
							ASMCopyOverride = decisionVal;
							std::cout << "[C.ARG] Forcing ASM decision to: ";
							break;
						}
						case argumentIDs::aI_GCT:
						{
							GCTBuildOverride = decisionVal;
							std::cout << "[C.ARG] Forcing GCT decision to: ";
							break;
						}
						case argumentIDs::aI_CLOSE:
						{
							CloseOnFinishBypass = decisionVal;
							std::cout << "[C.ARG] Bypass push button to close?: ";
							break;
						}
						case argumentIDs::aI_ASM_NAMES:
						{
							DisableNamesInASM = decisionVal;
							std::cout << "[C.ARG] Disable character names in ASM?: ";
							break;
						}
						default:
						{
							break;
						}
					}
					if (decisionVal)
					{
						std::cout << "Yes\n";
					}
					else
					{
						std::cout << "No\n";
					}
				}
				catch (std::exception e)
				{
					std::cerr << "[ERROR] Invalid argument value (\"" << argv[i] << "\") provided. ";
					switch (i)
					{
						case argumentIDs::aI_KBX:
						{
							std::cerr << "KBX";
							break;
						}
						case argumentIDs::aI_REL:
						{
							std::cerr << "REL";
							break;
						}
						case argumentIDs::aI_ASM:
						{
							std::cerr << "ASM";
							break;
						}
						case argumentIDs::aI_GCT:
						{
							std::cerr << "GCT";
							break;
						}
						case argumentIDs::aI_CLOSE:
						{
							std::cerr << "Push button to close";
							break;
						}
						case argumentIDs::aI_ASM_NAMES:
						{
							std::cerr << "Disable ASM Names";
							break;
						}
						default:
						{
							break;
						}
					}
					std::cerr << " argument not processed!\n";
				}
			}
		}
	}

	std::ofstream* kHCS = &lava::brawl::kirbyhat::kirbyHatChangelogStream;
	kHCS->open(lava::brawl::kirbyhat::changelogFilename, std::ios_base::out);
	*kHCS << "lavaKirbyHatManager " << lava::brawl::kirbyhat::version << " Changelog\n";
	std::cout << "lavaKirbyHatManager " << lava::brawl::kirbyhat::version << "\n";

	if (lava::folderExists(lava::brawl::kirbyhat::outputDirectory))
	{
		std::ifstream hatsIn;
		hatsIn.open(lava::brawl::kirbyhat::inputFilename, std::ios_base::in);
		if (hatsIn.is_open())
		{
			lava::brawl::moduleFile relIn;
			bool doModuleEdit = relIn.populate(lava::brawl::kirbyhat::relFilename);
			if (!doModuleEdit)
			{
				*kHCS << "[ERROR] No REL file was found. Please check that a \"" << lava::brawl::kirbyhat::relFilename << "\" file exists in the same directory as this program and try again.\n";
				std::cerr << "[ERROR] No REL file was found. Please check that a \"" << lava::brawl::kirbyhat::relFilename << "\" file exists in the same directory as this program and try again.\n";
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
				*kHCS << "[ERROR] No KBX file was found. Please check that a \"" << lava::brawl::kirbyhat::kbxFilename << "\" file exists in the same directory as this program and try again.\n";
				std::cerr << "[ERROR] No KBX file was found. Please check that a \"" << lava::brawl::kirbyhat::kbxFilename << "\" file exists in the same directory as this program and try again.\n";
			}

			*kHCS << "\nLoading Hat Entries...\n";
			std::vector<std::pair<std::string, std::pair<std::size_t, std::size_t>>> toAdd = parseInput(lava::brawl::kirbyhat::inputFilename, *kHCS);
			std::vector<std::size_t> added{};

			if (!toAdd.empty())
			{
				*kHCS << "\nAdding Hats...\n";

				std::vector<std::size_t> collisionIndeces = lava::brawl::kirbyhat::addCharacterFIDsAndNamesToMap(toAdd);

				for (std::size_t i = 0; i < toAdd.size(); i++)
				{
					std::pair<std::string, std::pair<std::size_t, std::size_t>>* currentHatPtr = &toAdd[i];
					auto sourceCharNameItr = lava::brawl::kirbyhat::kirbyHatFIDToName.find(currentHatPtr->second.second);
					*kHCS << "\"" << currentHatPtr->first << "\" ID: 0x" << lava::numToHexStringWithPadding(currentHatPtr->second.first, 0x04) << ", Source Character ID: 0x" << lava::numToHexStringWithPadding(currentHatPtr->second.second, 0x04) << " (" << sourceCharNameItr->second << ")\n";

					if (std::find(collisionIndeces.begin(), collisionIndeces.end(), i) == collisionIndeces.end())
					{
						added.push_back(currentHatPtr->second.first);

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
							lava::brawl::kirbyhat::kirbyHatFIDToName.find(currentHatPtr->second.first)->second <<
							"\" (ID 0x" << lava::numToHexStringWithPadding(currentHatPtr->second.first, 0x04) << ")\n";
					}
				}

				std::cout << "\n";
			}
			else
			{
				*kHCS << "\n[WARNING] No validly specified hats were found. Please check that there are valid entries in \"" << lava::brawl::kirbyhat::inputFilename << "\" and try again.\n";
				std::cerr << "\n[WARNING] No validly specified hats were found. Please check that there are valid entries in \"" << lava::brawl::kirbyhat::inputFilename << "\" and try again.\n\n";
			}

			if (relIn.fileBody.populated())
			{
				if (!toAdd.empty())
				{
					std::cout << "Added Hats to REL. ";
				}
				else
				{
					std::cout << "There were no Hats to add to REL. ";
				}
				relIn.fileBody.dumpToFile(lava::brawl::kirbyhat::relEditFilename);
				if (lava::fileExists(lava::brawl::kirbyhat::relAutoplaceFilename))
				{
					if (offerCopyOverAndBackup(lava::brawl::kirbyhat::relEditFilename, lava::brawl::kirbyhat::relAutoplaceFilename, RELCopyOverride))
					{
						*kHCS << "\nNote: Backed up \"" << lava::brawl::kirbyhat::relAutoplaceFilename << "\" and overwrote it with the newly edited REL.";
					}
					std::cout << "\n";
				}
			}
			if (kbxFile.populated())
			{
				if (!toAdd.empty())
				{
					std::cout << "Added Hats to KBX. ";
				}
				else
				{
					std::cout << "There were no Hats to add to KBX. ";
				}
				kbxFile.dumpToFile(lava::brawl::kirbyhat::kbxEditFilename);
				if (lava::fileExists(lava::brawl::kirbyhat::kbxAutoplaceFilename))
				{
					if (offerCopyOverAndBackup(lava::brawl::kirbyhat::kbxEditFilename, lava::brawl::kirbyhat::kbxAutoplaceFilename, KBXCopyOverride))
					{
						*kHCS << "\nNote: Backed up \"" << lava::brawl::kirbyhat::kbxAutoplaceFilename << "\" and overwrote it with the newly edited KBX.";
					}
					std::cout << "\n";
				}
			}
			if (lava::brawl::kirbyhat::addHatsToKHEXAsm(lava::brawl::kirbyhat::khexASMFilename, lava::brawl::kirbyhat::khexASMEditFilename, toAdd, DisableNamesInASM == 1))
			{
				if (!toAdd.empty())
				{
					std::cout << "Successfully added Hats to \"" << lava::brawl::kirbyhat::khexASMEditFilename << "\".\n";
				}
				else
				{
					std::cout << "No hats were added to \"" << lava::brawl::kirbyhat::khexASMEditFilename << "\".\n";
				}
				if (lava::fileExists(lava::brawl::kirbyhat::khexASMAutoplaceFilename))
				{
					if (offerCopyOverAndBackup(lava::brawl::kirbyhat::khexASMEditFilename, lava::brawl::kirbyhat::khexASMAutoplaceFilename, ASMCopyOverride))
					{
						*kHCS << "\nNote: Backed up \"" << lava::brawl::kirbyhat::khexASMAutoplaceFilename << "\" and overwrote it with the newly edited ASM.\n";
						handleAutoGCTRMProcess(lava::brawl::kirbyhat::khexASMEditFilename, GCTBuildOverride);
					}
				}
			}
			
			if (relIn.fileBody.populated() && kbxFile.populated())
			{
				*kHCS << "\n";
				lava::brawl::kirbyhat::summarizeHats(*kHCS, relIn, kbxFile, added);
			}
			*kHCS << "\n";
			std::cout << "\nSee changelog and summary in \"" << lava::brawl::kirbyhat::changelogFilename << ".\n";
		}
		else
		{
			*kHCS << "[ERROR] No input file was found. Please check that an \"" << lava::brawl::kirbyhat::inputFilename << "\" file exists in the same directory as this program and try again.\n";
			std::cerr << "[ERROR] No input file was found. Please check that an \"" << lava::brawl::kirbyhat::inputFilename << "\" file exists in the same directory as this program and try again.\n";
		}
	}
	else
	{
		std::cerr << "[ERROR] Output folder doesn't currently exist. Please check that a \"" << lava::brawl::kirbyhat::outputDirectory << "\" folder exists in the same directory as this program and try again.\n";
	}
	
	if (CloseOnFinishBypass == INT_MAX || CloseOnFinishBypass == 0)
	{
		std::cout << "\nPress any key to exit.\n";
		_getch();
	}
}