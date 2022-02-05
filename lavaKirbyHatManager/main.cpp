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

std::vector<std::pair<std::size_t, std::size_t>> parseInput (std::string inputFilePath)
{
	std::vector<std::pair<std::size_t, std::size_t>> result{};

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
				// Removes any space characters
				manipStr = "";
				for (std::size_t i = 0; i < currentLine.size(); i++)
				{
					if (!std::isspace(currentLine[i]))
					{
						manipStr += currentLine[i];
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
							result.push_back(newEntry);
						}
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
	kHCS->open(lava::brawl::kirbyhat::outputDirectory + "EX_KirbyHats_Changelog.txt", std::ios_base::out);
	*kHCS << "lavaKirbyHatManager " << lava::brawl::kirbyhat::version << "\n";

	std::ifstream hatsIn;
	hatsIn.open("EX_KirbyHats.txt", std::ios_base::in);
	if (hatsIn.is_open())
	{
		std::vector<std::pair<std::size_t, std::size_t>> toAdd{};

		std::string currentLine = "";
		std::string manipStr = "";
		while (std::getline(hatsIn, currentLine))
		{
			// Disregard the current line if it's empty, or is marked as a comment
			if (!currentLine.empty() && currentLine[0] != '#' && currentLine[0] != '/')
			{
				// Removes any space characters
				manipStr = "";
				for (std::size_t i = 0; i < currentLine.size(); i++)
				{
					if (!std::isspace(currentLine[i]))
					{
						manipStr += currentLine[i];
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
							toAdd.push_back(newEntry);
						}
					}
				}
			}
		}

		if (!toAdd.empty())
		{
			*kHCS << "\nAdding Hats...\n";

			lava::brawl::moduleFile relIn;
			bool doModuleEdit = relIn.populate("ft_kirby.rel");

			lava::byteArray kbxFile;
			std::ifstream kbxFileIn("KirbyHat.kbx", std::ios_base::in | std::ios_base::binary);
			bool doKBXEdit = 0;
			if (kbxFileIn.is_open())
			{
				kbxFile.populate(kbxFileIn);
				doKBXEdit = 1;
			}

			for (std::size_t i = 0; i < toAdd.size(); i++)
			{
				*kHCS << "EX Character ID: 0x" << lava::numToHexStringWithPadding(toAdd[i].first, 0x04) << ", Source Character ID: 0x" << lava::numToHexStringWithPadding(toAdd[i].second, 0x04) << "\n";

				if (doKBXEdit)
				{
					lava::brawl::kirbyhat::addHatToKBX(kbxFile, toAdd[i].first, toAdd[i].second);
				}

				if (doModuleEdit)
				{
					lava::brawl::kirbyhat::addHatToREL(relIn, toAdd[i].first, toAdd[i].second);
				}
			}
			if (relIn.fileBody.populated() && kbxFile.populated())
			{
				*kHCS << "\n";
				lava::brawl::kirbyhat::summarizeHats(*kHCS, relIn, kbxFile);
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
		*kHCS << "\n";



	}

}