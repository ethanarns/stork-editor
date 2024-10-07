# Stork Editor

After reverse engineering a portion of Yoshi's Island DS, I wanted to have something to show for it. So I created Stork Editor, a graphic level editor for the game.

It uses the USA version 1.0. Do NOT use 1.1 or "rev 1." SHA256 Sum: `c75bf32c31f7b5f02c4ed08102c421d243cfb3226d4f89d2819e645b531f6507`

### Features
1. View 100% of maps
2. Edit 80% of maps
3. 80% of Sprites documented, ability to add and remove them to maps
4. Easily create and use stamps to quickly create natural-looking levels
5. Modify and add collision to layers
6. Render support for 256 color mode
7. Entrance and Exit modification and creation
8. **Export fully functional ROM**

### Linux run instructions
1. Download release ZIP
2. Extract
3. Install libraries: `sudo apt install libxcb-cursor0 libpcre2-dev` (requires Universe repositories)
4. Run StorkEditor.AppImage

### Linux build instructions
1. Install QT 6.7
2. Install clang
3. Install devkitPro nds-dev (https://devkitpro.org/wiki/devkitPro_pacman)
4. Extract a USA YIDS 1.0 rom (see details above)
5. Open the project in QT Creator
6. Build it, and attempt to run it
7. Create the `lib/` directory where it specifies (usually inside the build folder)
8. Place a copy of `ndstool` inside it (named that exactly)
9. Run it again, and let it unpack

### Windows build instructions
1. Open in QT Creator
2. Build a release version (should be ~1MB)
3. Creating a staging folder and copy the `stork.exe` in the `release` folder into it
4. Copy the `lib` folder into the staging folder
5. (Optional) Copy `sample_brushes`, `README.md`, and `UserGuide.md` into the staging folder
6. Locate `windeployqt6.exe` (on my computer, it is in `C:\Qt\6.6.2\mingw_64\bin`)
7. Run `windeployqt6.exe <path\to\staging\folder>` and wait for it to populate with dependencies

### Todo
See documentation for latest: https://docs.google.com/spreadsheets/d/14wyO5R_ZZthzR4zfV5cDCKMmF-06exZupx3A8-u-xac/edit?usp=sharing

