# RMOD
## THIS WAS FOR A LEARNING EXPERIENCE. EXPECT THINGS TO NOT BE PERFECT.

> [!WARNING]
> ### SOME FEATURES ARE BUGGY OR UNSTABLE

This is for [Garry's Mod](https://store.steampowered.com/app/4000/Garrys_Mod/) x64

## Core Features

### Aimbot
- **Silent Aim**: Precision targeting without visible crosshair movement
- **Lock System**: Maintain target lock with configurable parameters
- **Automatic Fire**: Intelligent firing system
- **Visual Feedback**: Line to shoot position
- **Customization**:
  - Adjustable smoothing
  - Multiple hitbox targeting (Head, Chest, Stomach, Hitscan)
  - Priority systems (FOV, Distance, Health)
  - Spread prediction ([m9k](https://steamcommunity.com/sharedfiles/filedetails/?id=128089118), [swb](https://steamcommunity.com/sharedfiles/filedetails/?id=2279720120), [ptp](https://steamcommunity.com/sharedfiles/filedetails/?id=187933083), cw2, hl2)
- **Backtrack**:
  - 1 second backtrack without fakeping ([cl_interp abuse](https://www.unknowncheats.me/forum/garry-s-mod/414371-cl_interp-abuse-1-backtrack-fakeping.html)) (not 100% accuracy)
  - Visualisation in the form of dots and lines
  - Customizable colors

### ESP
- **Player Information**:
  - Name display
  - Origin points
  - Bounding boxes
  - Health indicators
  - Weapon display
  - Distance markers
- **Visual Elements**:
  - Skeletal tracking with customizable thickness
  - Configurable snaplines (Top, Center, Bottom)
  - Chams with material selection
  - Wall penetration options
- **Customization**:
  - Adjustable render distance (Default: 15000 units)
  - Individual color settings for all elements
  - Team ESP toggle
  - NPC ESP toggle

### Chams
- Ignore walls
- Draw original model
- player colour
- Material type
- **False Model**
  - Colour
  - Material type

### Movement
- Bunny hop assistance
- Air strafe optimization

### Interface Features
- Custom crosshair system with:
  - Adjustable size and offset
  - Outline options
  - Color customization
- FOV circle visualization
- Third person view:
  - Adjustable distance
  - Hotkey toggle
- Custom FOV settings:
  - View model FOV adjustment
  - World FOV adjustment
- Player and spectator lists
- Console with color-coded logging:
  - Info, Warning, Error, Fatal, Success messages
  - Customizable colors
- Loading screen customization

- **Stream Protection**:
  - Anti-OBS capabilities
  - Clean model rendering for streaming

### LUA Integration
- Dual-state support:
  - Client state
  - Menu state
- Custom script input and execution

## Known Issues
- Skeleton system may experience stability issues

## Upcoming Features
- Team recognition system
- General bug fixes and stability improvements

## Notes
- Some features may require additional configuration

## Sources
- [ImGui](https://github.com/ocornut/imgui)
- [MinHook](https://github.com/TsudaKageyu/minhook)
- [Json](https://github.com/nlohmann/json)
- [xorstr](https://github.com/JustasMasiulis/xorstr)
- [CLion](https://www.jetbrains.com/clion/)
- [UnknownCheats](https://www.unknowncheats.me/forum/garry-s-mod/)
- [pandora.uno UI](https://www.unknowncheats.me/forum/members/5364226.html)

## Showcase

### Visuals
<p float="left">
  <img src="https://github.com/raicug/rmod/blob/master/images/Visuals%20-%20FOV.png?raw=true" width="200" />
  <img src="https://github.com/raicug/rmod/blob/master/images/Visuals%20-%20Crosshair.png?raw=true" width="200" />
  <img src="https://github.com/raicug/rmod/blob/master/images/Visuals%20-%20ESP.png?raw=true" width="200" />
  <img src="https://github.com/raicug/rmod/blob/master/images/Visuals%20-%20Chams.png?raw=true" width="200" />
</p>

### Appearance
<img src="https://github.com/raicug/rmod/blob/master/images/Appearance.png?raw=true" width="400" />

### LUA
<img src="https://github.com/raicug/rmod/blob/master/images/LUA.png?raw=true" width="400" />

### World
<p float="left">
  <img src="https://github.com/raicug/rmod/blob/master/images/World%20-%20Aimbot.png?raw=true" width="200" />
  <img src="https://github.com/raicug/rmod/blob/master/images/World%20-%20Movement.png?raw=true" width="200" />
  <img src="https://github.com/raicug/rmod/blob/master/images/World%20-%20Players.png?raw=true" width="200" />
  <img src="https://github.com/raicug/rmod/blob/master/images/World%20-%20Misc.png?raw=true" width="200" />
</p>

### Game
<img src="https://github.com/raicug/rmod/blob/master/images/Game.png?raw=true" width="400" />
