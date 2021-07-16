# HUDColor

### Change Sven Co-op HUD color

### It's basically a product of learning metabook

### most code is copied from [Renderer.dll](https://github.com/hzqst/MetaHookSv/tree/main/Plugins/Renderer)

### To use this plugin, 
1. Install [this](https://github.com/hzqst/MetaHookSv)
2. Download `HUDColor.dll` from realse page or build it by your self.
3. Put `HUDColor.dll` into `svencoop\metahook\plugins` directory.
4. Add `HUDColor.dll` to `metahook\configs\plugins.lst` as last line.

# New CVar
|CVar|Value|Comment|
|---|---|---|
|hud_color_r|0~255|change main color of HUD|
|hud_color_g|0~255|change main color of HUD|
|hud_color_b|0~255|change main color of HUD|
|hud_color_pain_r|0~255|change dying color of HUD|
|hud_color_pain_g|0~255|change dying color of HUD|
|hud_color_pain_b|0~255|change dying color of HUD|
|hud_color_dizzy|0/1|Enable HUD dizzy when get hurt|
|hud_color_dizzy_step|0~9999999|Speed of dizzy disappearance|