# User Guide

## Getting Started
1. Acquire a Yoshi's Island DS USA 1.0 ROM. There are tutorials online on how to extract this legally
2. Install any neccesary dependencies. The README.md will have the most up-to-date instructions on this
3. Once Stork Editor has loaded (`Ready` is the status in the bottom left), go to Menu Bar > File > Open, and select your .nds ROM file
4. Wait for it to load everything for the first time. It will need to extract files. Wait for the status `ROM Loaded`
5. It should load World 1 Level 1's first map. You'll need to scroll down to see the level itself
6. To remove the green collision overlay, you can either click the menu button with the eye and orange arrow, check off Menu Bar > View > Show Collision, or press `Control + 5`
7. Backup your working files (by default `yromfs`) by copying it to a different folder periodically

## Sprites (Level Objects)
1. Switch your layer mode to Sprites. This is a dropdown on the toolbar at the top of the window. It should start in this mode, but you can return to it via this dropdown
2. Make sure sprite rendering is enabled. Menu Bar -> View -> Show Sprites should be checked
3. Open the Sprite Picker window. This is a button on the toolbar that looks like a yellow coin
4. Click to select the sprite you wish to place. You can search by both name and hexadecimal ID
5. **Right click on the map to place the sprite**. Remember that you must be in the Sprites layer mode (see step 1)
6. You can select placed sprites with left click
7. You can move selected sprites by either dragging or by editing the X and Y position in the top left window pane
8. You can delete selected sprites by pressing the delete or backspace key
9. You can edit sprite settings by changing the hex values in the top left window pane. See the Sprite IDs section of the documentation spreadsheet for more info

## Basic Background Editing (BG 1-3)
1. Switch your layer mode to background you wish to edit. You can find out which background you want to edit by toggling them on and off in the Menu Bar > View menu
2. Open the BG Tile Brush window. This is represented by a painbrush icon in the top menu bar
3. (Optional unless making new brushes) Open the BG Tiles window. This is represented by a graph with blue blocks foriming a T icon in the top menu bar
4. Find out what your current background's tileset is by looking at the bottom right corner of the Tile Brush Window. Tileset names look like `char01a`, the number being the preset theme
5. Load some sample brushes. Navigate to the StorkEditor > sample_brushes directory, and find the folder that matches your tileset name. It may not be there yet, but you can make some! Go into that folder, and select all the brushes (Control + A)
6. You should see a bunch of brush names appear in the right side of the window. If the background is correct, they should be bright white, not greyed out. Try clicking on one
7. Once you have selected a brush, return to the map view and try left clicking. You should see the graphics appear on the map!
8. You can use Control + Z to undo any placement that doens't look right, or left click drag select and press delete. You can also hold right click and drag to delete everything in the selection immediately

## Collision Editing
1. Switch your layer mode to Colliders via the toolbar dropdown
2. Open the Collision Tile window. This is represented by an orange arrow hitting a red block with a pencil near it
3. Make sure collision rendering is enabled. Ensure Menu Bar > View > Show Collision is checked on
4. Select the collision tile you wish to draw with. You can do this by either selecting a tile from the Collision Tile window, or by right clicking an existing collision tile while in the Colliders layer mode
5. Once you have selected the tile you wish to place, you can simply click or drag and draw tiles
6. To delete tiles, simply select "Clear/Erase" in the Collision Tile window, or right click an empty collision space

## Brush Creation
1. Make sure you are on the desired BG layer mode
2. Open the BG Tile Brush window (Paintbrush Icon)
3. Open the BG Tiles window (Blue T icon)
4. Select an existing section of the background you wish to turn into a brush. Make sure it is 2x2 aligned (see later instructions for more info)
5. Click Load Selected in the Tile Brush Window. You should see your selection appear in the window
6. Make sure the width and height of the brush is even. You can trim down extra rows and columns by clicking the blank tile in the BG Tiles window (top left). Also make sure it is 2x2 aligned. You can check this by trying to draw over what you just selected. If you can't get it to draw in the exact same place, you are not 2x2 aligned
7. Name the brush something very specific and detailed, then click Save Brush
8. Once it has appeared in the right side of the window, you can click Export to save the brush to your file system
9. If there is not already a folder for it in `sample_brushes`, create a new folder with the name `{description}_{tileset}`. For example, `jungle_char16a`
10. Save your new brush to that folder. You can now load it again later for use with that same tileset!

## Portals (Entrances and Exits)
0. **Warning**: portal connections are very fragile and glitchy. Make sure to check all your connections are accurate when modifying connections, and especially when deleting an entrance or exit! **I highly recommend backing up your data directory (by default `yromfs`) before making any connection changes**
1. Open Menu Bar > Tools > Level Settings or press Control + L
2. You can select any existing Entrances or Exit and change all its values, add new portals, or delete portals (be careful with deletion)
3. There is a planned Portals layer mode, but it is currently not implemented. The only way to change portal positions as of writing this guide is within the Level Window


## Troubleshooting
If none of the following tips resolve your problem, the best thing to do is to post the `stork.log` and a description of the problem in this Discord server: [Yoshi's Island DS Hacking](https://discord.gg/Fy4za2WsT6). You can also DM Zolarch on [SMW Central ](https://www.smwcentral.net/?p=viewthread&t=127068). Make absolutely sure you upload or copy the contents of `stork.log`, as this is vital for diagnosing the problem.

- Nothing happens when I click
  - Check you are on the correct layer mode. This is the dropdown in the toolbar at the top of the window
- The program will not open at all
  - You may be missing a dependency. See the main README.md for requirements
- Sprites are invisible
  - Make sure Menu Bar -> View -> Show Sprites is checked on
  - If it is still checked, but Sprites don't show up, toggle it off and on again
  - Some sprites may not render properly with invalid settings
