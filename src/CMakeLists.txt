add_library(neoslancer_data STATIC
    IniFile.cpp
    GameConfig.cpp
    GamePaths.cpp
    CommandLine.cpp
    BigFile.cpp
    RefPack.cpp
    ResourceLoader.cpp
    WinVfxFont.cpp
    WinVfxSprite.cpp
    vr/VRRoomGraph.cpp
)

target_include_directories(neoslancer_data PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_options(neoslancer_data PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
)

add_executable(neoslancer
    main.cpp
    Application.cpp
    Window.cpp
    gfx/Shader.cpp
    gfx/Font.cpp
    gfx/UIRenderer.cpp
    menu/MenuManager.cpp
    menu/ButtonList.cpp
    menu/AdjustableList.cpp
    menu/SimpleMenuScreen.cpp
    menu/MainMenuScreen.cpp
    menu/OptionsMenuScreen.cpp
    menu/PlaceholderScreen.cpp
    menu/IntroScreen.cpp
    menu/SoundOptionsScreen.cpp
    menu/VideoOptionsScreen.cpp
    menu/ControlsOptionsScreen.cpp
    menu/NetworkDisconnectScreen.cpp
    video/BinkVideoPlayer.cpp
    vr/VRRoomScreen.cpp
)

target_include_directories(neoslancer PRIVATE
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(neoslancer PRIVATE
    neoslancer_data
    ${SDL2_LIBRARIES}
    SDL2_ttf::SDL2_ttf
    OpenGL::GL
    GLEW::GLEW
    PkgConfig::FFMPEG
)

target_compile_options(neoslancer PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
)

add_executable(hogdump
    tools/hogdump.cpp
)

target_link_libraries(hogdump PRIVATE
    neoslancer_data
)

target_compile_options(hogdump PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
)
