<pre>
  _      __         __   __           ___  ___           __
 | | /| / /__  ____/ /__/ /__   ___  / _/ / _ \___ _____/ /__ ___  ___ ___ ___
 | |/ |/ / _ \/ __/ / _  (_-<  / _ \/ _/ / // / _ `/ __/  '_// _ \/ -_|_-<(_-<
 |__/|__/\___/_/ /_/\_,_/___/  \___/_/  /____/\_,_/_/ /_/\_\/_//_/\__/___/___/
                                                                         
                           version 0.62 (unfinished)
                   Copyright (c) 2005-25 Elwynor Technologies

</pre>

## WHAT IS WORLDS OF DARKNESS?

 Worlds of Darkness is an unfinished and unreleased ANSI 3-D role playing game.
 During the course of the game many races, classes, and items can be found
 throughout the game world. 

## WHAT IS WDMAPUTL?

 This is the Map Utility that "squeezes" the map data for parsing by the game.
 You need to do this before installing the game unless you are distributing
 a version that has the map data squeezed already.

## HOW TO USE WDMAPUTL

 Create a folder withe wdmaputl.exe and a subfolder MAP.
 Place the 32 un-squeezed MAP files in the MAP folder. They're all about
 3430 bytes. Then execute wdmaputl - it will create a SQZMAP folder
 with the 'squeezed' maps for use with the game.

## MODULE HISTORY
 
 Worlds of Darkness was developed between late 1994 and mid 1995 by Zark 
 Wizard and Alf, the tag team behind the GraphicWares/Wizard Software combined
 ISV. The game was never finished, but what is in the game is working and 
 playable. 
 
 GraphicWares and Wizard Software were acquired by Wilderland Software, which
 was then acquired by Elwynor Technologies in 2005.

 Elwynor Technologies ported the module to Worldgroup 3.2 in February 2021, but
 did not finish the missing pieces. As part of the Major BBS Restoration Project,
 a team asked to work on the module to finish it, which hopefully will happen
 in 2024-2025. See the devnotes folder for more information about the plans and
 status. We will track issues, requests, and ideas in github going forward.

 In December 2025, Elwynor Technologies ported the unfinished module to
 The Major BBS V10 and posted it to github.
 
## LICENSE

 This project is licensed under the AGPL v3. Additional terms apply to 
 contributions and derivative projects. Please see the LICENSE file for 
 more details.

## CONTRIBUTING

 We welcome contributions from the community. By contributing, you agree to the
 terms outlined in the CONTRIBUTING file.

## CREATING A FORK

 If you create an entirely new project based on this work, it must be licensed 
 under the AGPL v3, assign all right, title, and interest, including all 
 copyrights, in and to your fork to Rick Hadsall and Elwynor Technologies, and 
 you must include the additional terms from the LICENSE file in your project's 
 LICENSE file.

## COMPILATION

 This can be compiled using Borland  C/C++ 5.0 or Visual Studio 2022. 
 To use Borland, just use the c.cmd batch file (edit if needed).
 To use VS2022, include this as an existing project like you would for a 
 module. It will build the EXE.
 
## PACKING UP

 The DIST folder includes all of the files needed to run the utility.
 Note that this doesn't have to be distributed unless you want people to
 edit the maps.
 
