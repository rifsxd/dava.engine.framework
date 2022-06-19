# SpineModule

Spine integration module for Dava Engine.

# Resources pipeline

> For current library version (3.4.02) correctly work only JSON format.

Spine atlases should been stored in `DataSource/Gfx` folder with `flags.txt`
config file which contains command argument `--split`. After build resources
converted textures with atlas files will be stored in `Data/Gfx` automatically.

Spine skeleton files can be stored in any location inside `Data` folder.

# Spine Component

For work with Spine animation add `UISpineComponent` and `UIControlBackground
(Background)` components to control. Setup draw type in background component to
`DRAW_BATCH`.