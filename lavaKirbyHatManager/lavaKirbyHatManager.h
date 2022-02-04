#ifndef LAVA_KIRBY_HAT_MANAGER_H_V1
#define LAVA_KIRBY_HAT_MANAGER_H_V1

#include "lavaByteArray.h"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

namespace lava
{
	namespace brawl
	{
		namespace kirbyhat
		{
			inline constexpr std::size_t sectionListBeginOffset = 0x4C;
			inline constexpr std::size_t transactorSectionIndex = 0x04;
			extern std::size_t transactorSectionAddress;
			extern std::size_t transactorSectionLength;
			inline constexpr std::size_t transactorNullCommand =	0x60000000;
			inline constexpr std::size_t transactorEntryTag =		0x48000001;

			inline constexpr std::size_t importSectionOffsetHeaderOffset = 0x28;
			extern std::size_t importSectionAddress;
			inline constexpr std::size_t importSectionOffsetLengthOffset = 0x14;

			inline constexpr unsigned long long int mewtwoTransactorLinkEntry = 0x00D00A010002060C;
			extern std::size_t mewtwoTransactorLinkEntryAddress;

			bool setup(lava::byteArray& moduleIn);

			bool addHatTransactor(lava::byteArray& moduleIn, std::size_t charID, std::size_t mapFuncID);
		}
	}
}

#endif