# lavaKirbyHatManager
A program to help simplify the process of setting up Kirby Hats for EX Characters in Project+Ex builds, based on [DesiacX's EX Kirby Hat Quick Start Guide](https://docs.google.com/document/d/17B462eugiS45PcSsie1iIr8gDl-bQM-gjIT17TIfl6Q/).

Note that this program currently only performs Steps 1, 2, 3, and 5 of the linked guide, and that's about it. As a result, you'll still need to manually perform any other steps involved in the process of getting your hats up and running.
Importantly, that means you'll still need to manually edit your characters' "FighterXX.dat" files, copy and rename your source character's hat files, edit "Source/ProjectM/CloneEngine.asm" (if you're working with a Lucario or Samus hat), and compile your GCTs with GCTRM yourself. See included "readme.txt" for more specific use instructions. Additionally, like is mentioned in the guide, there *is* a limit to how many hats you can add this way. I don't know what that limit is, but it DOES exist, so be mindful of that if you start running into issues.

***Important Note:*** *Kirby Hat compatibility is still* ***extremely*** *spotty, most source characters simply don't work. While this may change as fixes are developed for other hats, things are (and will remain) remarkably limited for the time being. For preliminary compatibility information, see [this chart](https://docs.google.com/spreadsheets/d/1rlw8wemQ849w-6AgGISXATPv6-pfVnTTQcQqd6N-crI/edit?usp=sharing).*

## Instructions for Automatic File Placement and GCTRM

1. Download the most recent release (Version 0.8.0, as of writing this).

2. Extract the zipped folder into your build's base folder. For instance, P+EX builds would place it into the "Project+" folder, so that the program executables are located in "Project+/lavaKirbyHatManager - Version 0.8.0/".

3. Copy your "KirbyHatEX.asm", "KirbyHat.kbx", and "ft_kirby.rel" files into the program's folder. Ideally, start from fresh, stock P+EX versions of these.

4. Add entries to the included "EX_KirbyHats.txt", as specified in that file.

5. Run the appropriate version of the program for your build (use the "NETPLAY" version for builds using "NETPLAY.GCT" and "NETBOOST.GCT").

6. For each file that the program produces, it will search for that file in the appropriate location in your build. If it's found, it will offer to backup the existing file and copy over the newly edited one. Additionally, if it replaces your "KirbyHatEX.asm" file, and also detects the GCTRM executable, it will offer to run that as well.

Note: If the program replaces your "KirbyHatEX.asm" but *doesn't* prompt you to run GCTRM, it's because it couldn't find "RSBE01.txt" and "BOOST.txt" (or "NETPLAY.txt" and "NETBOOST.txt", when using the "DOLPHIN" version). Make sure those exist in your "Project+" folder, then run the program again.
