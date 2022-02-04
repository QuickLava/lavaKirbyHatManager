#include "lavaKirbyHatManager.h"


int main()
{
	lava::byteArray moduleFile;
	std::ifstream moduleFileIn("ft_kirby.rel", std::ios_base::in | std::ios_base::binary);
	moduleFile.populate(moduleFileIn);
	lava::brawl::kirbyhat::setup(moduleFile);
	lava::brawl::kirbyhat::addHatTransactor(moduleFile, 0x53, 0x0001FFEC);
	moduleFile.dumpToFile("ft_kirby_edit.rel");
	return 0;
}