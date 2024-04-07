# Stork Editor

After reverse engineering a portion of Yoshi's Island DS, I wanted to have something to show for it. So I created Stork Editor, a graphic level editor for the game.

It uses the USA version 1.0. Do NOT use 1.1 or "rev 1." SHA256 Sum: `c75bf32c31f7b5f02c4ed08102c421d243cfb3226d4f89d2819e645b531f6507`

### Features:
1. View and edit 95% of maps
2. 80% of Sprites documented, ability to add and remove them to maps
2. Easily create and use stamps to quickly create natural-looking levels
3. Modify and add collision to layers
4. Render support for 256 color mode
5. Entrance and Exit modification and creation
6. **Export fully functional ROM**

### Linux run instructions:
1. Download release ZIP
2. Extract
3. Install libraries: `sudo apt install libxcb-cursor0 libpcre2-dev` (requires Universe repositories)
4. Run StorkEditor.AppImage

### Linux build instructions: (update to QT6 plox)
1. Install QT 5.15
2. Install devkitPro for the GBA (https://devkitpro.org/wiki/devkitPro_pacman)
3. Extract a USA YIDS 1.0 rom
4. Run defaultRun.sh

### Todo:
- Line following implementation
- Gradient and Alpha BG support
- New map creation
- Soft rock editing
- Graphics editing