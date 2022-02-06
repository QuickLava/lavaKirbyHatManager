#include "lavaByteArray.h"

namespace lava
{
	void byteArray::populate(std::istream& sourceStream)
	{
		_populated = 1;
		std::string buffer = "";
		sourceStream.seekg(0, sourceStream.end);
		std::size_t sourceSize(sourceStream.tellg());
		sourceStream.seekg(0, sourceStream.beg);
		body.resize(sourceSize);
		sourceStream.read(body.data(), sourceSize);
		std::cout << "Loaded " << body.size() << " byte(s) of data.\n";
	}
	bool byteArray::populated()
	{
		return _populated;
	}

	std::vector<char> byteArray::getBytes(std::size_t numToGet, std::size_t startIndex, std::size_t& numGotten)
	{
		numGotten = 0;
		if (startIndex < body.size())
		{
			if (startIndex + numToGet > body.size())
			{
				numToGet = body.size() - startIndex;
				return std::vector<char>(body.begin() + startIndex, body.end());
			}
			numGotten = numToGet;
			return std::vector<char>(body.begin() + startIndex, body.begin() + startIndex + numToGet);
		}
		else
		{
			std::cerr << "\nRequested region startpoint was invalid. Specified index was [" << startIndex << "], max valid index is [" << body.size() - 1 << "].\n";
		}
		return std::vector<char>();
	}
	unsigned long long int byteArray::getLLong(std::size_t startIndex)
	{
		return getObj<unsigned long long int>(startIndex);
	}
	unsigned long int byteArray::getLong(std::size_t startIndex)
	{
		return getObj<unsigned long int>(startIndex);
	}
	unsigned short int byteArray::getShort(std::size_t startIndex)
	{
		return getObj<unsigned short int>(startIndex);
	}
	unsigned char byteArray::getChar(std::size_t startIndex)
	{
		return getObj<unsigned char>(startIndex);
	}

	bool byteArray::setBytes(std::vector<char> bytesIn, std::size_t atIndex)
	{
		bool result = 0;
		if ((atIndex + bytesIn.size()) > atIndex && atIndex + bytesIn.size() < body.size())
		{
			/*int tempInt = 0;
			char* tempPtr = body.data() + atIndex;
			std::cout << "Original Value: " << std::hex;
			for (int i = 0; i < bytesIn.size(); i++)
			{
				tempInt = *(tempPtr + i);
				tempInt &= 0x000000FF;
				std::cout << ((tempInt < 0x10) ? "0" : "") << tempInt;
			}
			std::cout << "\n" << std::dec;*/
			std::memcpy(body.data() + atIndex, bytesIn.data(), bytesIn.size());
			/*tempInt = 0;
			tempPtr = body.data() + atIndex;
			std::cout << "Modified Value: " << std::hex;
			for (int i = 0; i < bytesIn.size(); i++)
			{
				tempInt = *(tempPtr + i);
				tempInt &= 0x000000FF;
				std::cout << ((tempInt < 0x10) ? "0" : "") << tempInt;
			}
			std::cout << "\n" << std::dec;*/
			result = 1;
		}
		return result;
	}
	bool byteArray::setLLong(unsigned long long int valueIn, std::size_t atIndex)
	{
		return setObj<unsigned long long int>(valueIn, atIndex);
	}
	bool byteArray::setLong(unsigned long int valueIn, std::size_t atIndex)
	{
		return setObj<unsigned long int>(valueIn, atIndex);
	}
	bool byteArray::setShort(unsigned short int valueIn, std::size_t atIndex)
	{
		return setObj<unsigned short int>(valueIn, atIndex);
	}
	bool byteArray::setChar(unsigned char valueIn, std::size_t atIndex)
	{
		return setObj<unsigned char>(valueIn, atIndex);
	}

	bool byteArray::insertBytes(std::vector<char> bytesIn, std::size_t atIndex)
	{
		bool result = 0;
		if (atIndex < body.size())
		{
			result = 1;
			body.insert(body.begin() + atIndex, bytesIn.begin(), bytesIn.end());
		}
		return result;
	}
	bool byteArray::insertLLong(unsigned long long int valueIn, std::size_t atIndex)
	{
		return insertObj<unsigned long long int>(valueIn, atIndex);
	}
	bool byteArray::insertLong(unsigned long int valueIn, std::size_t atIndex)
	{
		return insertObj<unsigned long int>(valueIn, atIndex);
	}
	bool byteArray::insertShort(unsigned short int valueIn, std::size_t atIndex)
	{
		return insertObj<unsigned short int>(valueIn, atIndex);
	}
	bool byteArray::insertChar(unsigned char valueIn, std::size_t atIndex)
	{
		return insertObj<unsigned char>(valueIn, atIndex);
	}

	std::size_t byteArray::search(const std::vector<char>& searchCriteria, std::size_t startItr, std::size_t endItr)
	{
		std::vector<char>::iterator itr = body.end();
		if (endItr < startItr)
		{
			endItr = SIZE_MAX;
		}
		if (endItr > body.size())
		{
			endItr = body.size();
		}
		if (body.size() && startItr < body.size() && searchCriteria.size())
		{
			itr = std::search(body.begin() + startItr, body.begin() + endItr, searchCriteria.begin(), searchCriteria.end());
		}
		return (itr != body.end()) ? itr - body.begin() : SIZE_MAX;
	}
	std::size_t byteArray::searchLLong(unsigned long long int searchCriteria, std::size_t startItr, std::size_t endItr)
	{
		return findObj<unsigned long long int>(searchCriteria, startItr, endItr);
	}
	std::size_t byteArray::searchLong(unsigned long int searchCriteria, std::size_t startItr, std::size_t endItr)
	{
		return findObj<unsigned long int>(searchCriteria, startItr, endItr);
	}
	std::size_t byteArray::searchShort(unsigned short int searchCriteria, std::size_t startItr, std::size_t endItr)
	{
		return findObj<unsigned short>(searchCriteria, startItr, endItr);
	}
	std::size_t byteArray::searchChar(unsigned char searchCriteria, std::size_t startItr, std::size_t endItr)
	{
		return findObj<unsigned char>(searchCriteria, startItr, endItr);
	}

	std::vector<std::size_t> byteArray::searchMultiple(const std::vector<char>& searchCriteria, std::size_t startItr, std::size_t endItr)
	{
		std::size_t cursor = startItr;
		std::vector<std::size_t> result;
		std::size_t critSize = searchCriteria.size();
		bool done = 0;
		while (!done && cursor <= endItr)
		{
			cursor = search(searchCriteria, cursor);
			if (cursor != SIZE_MAX)
			{
				result.push_back(cursor);
				if ((cursor + critSize) > cursor)
				{
					cursor += critSize;
				}
			}
			else
			{
				done = 1;
			}
		}
		return result;
	}
	std::vector<std::size_t> byteArray::searchMultipleLLong(unsigned long long int searchCriteria, std::size_t startItr, std::size_t endItr)
	{
		return findObjMultiple<unsigned long long int>(searchCriteria, startItr, endItr);
	}
	std::vector<std::size_t> byteArray::searchMultipleLong(unsigned long int searchCriteria, std::size_t startItr, std::size_t endItr)
	{
		return findObjMultiple<unsigned long int>(searchCriteria, startItr, endItr);
	}
	std::vector<std::size_t> byteArray::searchMultipleShort(unsigned short int searchCriteria, std::size_t startItr, std::size_t endItr)
	{
		return findObjMultiple<unsigned short>(searchCriteria, startItr, endItr);
	}
	std::vector<std::size_t> byteArray::searchMultipleChar(unsigned char searchCriteria, std::size_t startItr, std::size_t endItr)
	{
		return findObjMultiple<unsigned char>(searchCriteria, startItr, endItr);
	}

	bool byteArray::dumpToFile(std::string targetPath)
	{
		bool result = 0;
		std::ofstream output;
		output.open(targetPath, std::ios_base::binary | std::ios_base::out);
		if (output.is_open())
		{
			output.write(body.data(), body.size());
			result = 1;
			std::cout << "Dumped body to \"" << targetPath << "\".\n";
		}
		return result;
	}
}