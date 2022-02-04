#ifndef LAVA_BYTE_ARRAY_H_V1
#define LAVA_BYTE_ARRAY_H_V1

#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>


namespace lava
{
	struct byteArray
	{
	private:
		bool _populated = 0;
	public:
		std::vector<char> body = {};
	private:
		template<typename objectType>
		objectType getObj(std::size_t startIndex)
		{
			objectType result = SIZE_MAX;
			std::size_t numGotten = 0;
			std::vector<char> bytes = getBytes(sizeof(objectType), startIndex, numGotten);
			if (numGotten == sizeof(objectType))
			{
				std::reverse(bytes.begin(), bytes.end());
				result = *((objectType*)bytes.data());
			}
			return result;
		}
		template<typename objectType>
		bool setObj(const objectType& objectIn, std::size_t startIndex)
		{
			bool result = 0;
			if (startIndex + sizeof(objectType) < body.size())
			{
				std::vector<char> temp((char*)(&objectIn), (char*)(&objectIn) + sizeof(objectType));
				std::reverse(temp.begin(), temp.end());
				result = setBytes(temp, startIndex);
			}
			return result;
		}
		template<typename objectType>
		bool insertObj(const objectType& objectIn, std::size_t startIndex)
		{
			bool result = 0;
			if (startIndex < body.size())
			{
				std::vector<char> temp((char*)(&objectIn), (char*)(&objectIn) + sizeof(objectType));
				std::reverse(temp.begin(), temp.end());
				result = insertBytes(temp, startIndex);
			}
			return result;
		}

		template<typename objectType>
		std::size_t findObj(const objectType& objectIn, std::size_t startItr = 0, std::size_t endItr = SIZE_MAX)
		{
			std::size_t result = 0;
			if (startItr < body.size())
			{
				std::vector<char> temp((char*)(&objectIn), (char*)(&objectIn) + sizeof(objectType));
				std::reverse(temp.begin(), temp.end());
				result = search(temp, startItr, endItr);
			}
			return result;
		}

		template<typename objectType>
		std::vector<std::size_t> findObjMultiple(const objectType& objectIn, std::size_t startItr = 0, std::size_t endItr = SIZE_MAX)
		{
			std::vector<std::size_t> result{};
			if (startItr < body.size())
			{
				std::vector<char> temp((char*)(&objectIn), (char*)(&objectIn) + sizeof(objectType));
				std::reverse(temp.begin(), temp.end());
				result = searchMultiple(temp, startItr, endItr);
			}
			return result;
		}

	public:
		void populate(std::istream& sourceStream);
		bool populated();

		std::vector<char> getBytes(std::size_t numToGet, std::size_t startIndex, std::size_t& numGot);
		unsigned long long int getLLong(std::size_t startIndex);
		unsigned long int getLong(std::size_t startIndex);
		unsigned short int getShort(std::size_t startIndex);
		unsigned char getChar(std::size_t startIndex);

		bool setBytes(std::vector<char> bytesIn, std::size_t atIndex);
		bool setLLong(unsigned long long int valueIn, std::size_t atIndex);
		bool setLong(unsigned long int valueIn, std::size_t atIndex);
		bool setShort(unsigned short int valueIn, std::size_t atIndex);
		bool setChar(unsigned char valueIn, std::size_t atIndex);

		bool insertBytes(std::vector<char> bytesIn, std::size_t atIndex);
		bool insertLLong(unsigned long long int valueIn, std::size_t atIndex);
		bool insertLong(unsigned long int valueIn, std::size_t atIndex);
		bool insertShort(unsigned short int valueIn, std::size_t atIndex);
		bool insertChar(unsigned char valueIn, std::size_t atIndex);

		std::size_t search(const std::vector<char>& searchCriteria, std::size_t startItr = 0, std::size_t endItr = SIZE_MAX);
		std::size_t searchLLong(unsigned long long int searchCriteria, std::size_t startItr = 0, std::size_t endItr = SIZE_MAX);
		std::size_t searchLong(unsigned long int searchCriteria, std::size_t startItr = 0, std::size_t endItr = SIZE_MAX);
		std::size_t searchShort(unsigned short int searchCriteria, std::size_t startItr = 0, std::size_t endItr = SIZE_MAX);
		std::size_t searchChar(unsigned char searchCriteria, std::size_t startItr = 0, std::size_t endItr = SIZE_MAX);

		std::vector<std::size_t> searchMultiple(const std::vector<char>& searchCriteria, std::size_t startItr = 0, std::size_t endItr = SIZE_MAX);
		std::vector<std::size_t> searchMultipleLLong(unsigned long long int searchCriteria, std::size_t startItr = 0, std::size_t endItr = SIZE_MAX);
		std::vector<std::size_t> searchMultipleLong(unsigned long int searchCriteria, std::size_t startItr = 0, std::size_t endItr = SIZE_MAX);
		std::vector<std::size_t> searchMultipleShort(unsigned short int searchCriteria, std::size_t startItr = 0, std::size_t endItr = SIZE_MAX);
		std::vector<std::size_t> searchMultipleChar(unsigned char searchCriteria, std::size_t startItr = 0, std::size_t endItr = SIZE_MAX);

		bool dumpToFile(std::string targetPath);
	};
}

#endif