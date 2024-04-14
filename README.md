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
1. Install QT 6.6 c++ libraries directly or install QT Creator
2. Install devkitPro nds-dev (https://devkitpro.org/wiki/devkitPro_pacman)
3. Extract a USA YIDS 1.0 rom (see details above)

### Todo
See documentation for latest: https://docs.google.com/spreadsheets/d/14wyO5R_ZZthzR4zfV5cDCKMmF-06exZupx3A8-u-xac/edit?usp=sharing