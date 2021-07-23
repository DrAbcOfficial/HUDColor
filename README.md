# HUDColor

## Change Sven Co-op HUD color

### It's basically a product of learning metabook

### most code is copied from [Renderer.dll](https://github.com/hzqst/MetaHookSv/tree/main/Plugins/Renderer)

# Install

1. Download and install [MetaHookSv](https://github.com/hzqst/MetaHookSv).

2. Build or download .dll file into `svencoop/metahook/plugins` directory.

3. Add HUDColor.dll in `svencoop/metahook/configs/plugins.lst` as a newline.

4. Enjoy.

# New CVar
|CVar|Value|Comment|
|---|---|---|
|hud_color_r|0~255|change main color of HUD|
|hud_color_g|0~255|change main color of HUD|
|hud_color_b|0~255|change main color of HUD|
|hud_color_pain_r|0~255|change dying color of HUD|
|hud_color_pain_g|0~255|change dying color of HUD|
|hud_color_pain_b|0~255|change dying color of HUD|
|hud_color_dizzy|0/1/2|Enable HUD dizzy when get hurt|
|hud_color_dizzy_time|0~9999999|time of dizzy disappearance|

# Build

1. Clone or download [MetaHookSv](https://github.com/hzqst/MetaHookSv) source code.

2. Clone or download `HUDColor` source code

3. Put `HUDColor` directory into `MetaHookSv/Plugins`

4. Open `.sln` with visual studio.

5. Build.
