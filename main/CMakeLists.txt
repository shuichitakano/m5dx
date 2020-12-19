
#project(main CXX)

set(CMAKE_CXX_STANDARD 17)
set(COMPONENT_SRCDIRS . system io graphics mxdrv audio sound_sys music_player ui util)
set(COMPONENT_ADD_INCLUDEDIRS .)
list(APPEND COMPONENT_REQUIRES "arduino" "m5stack")


idf_component_register(
    SRC_DIRS 
        ${COMPONENT_SRCDIRS}
    INCLUDE_DIRS
        "."
    EMBED_FILES
        data/m5dx_material.bmp data/_4x8_font.bin data/misaki_font.bin
)

target_compile_options(${COMPONENT_LIB} PRIVATE --exec-charset=cp932)

