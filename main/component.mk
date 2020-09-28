
CXXFLAGS += --exec-charset=cp932 -std=c++1z

COMPONENT_SRCDIRS := . system io graphics mxdrv audio sound_sys music_player ui util
COMPONENT_ADD_INCLUDEDIRS := .

COMPONENT_EMBED_FILES := data/m5dx_material.bmp data/_4x8_font.bin data/misaki_font.bin
