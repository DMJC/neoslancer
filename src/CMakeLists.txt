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
    WinVfxPalette.cpp
    TgaImage.cpp
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
    gfx/WinVfxRenderer.cpp
    menu/MenuManager.cpp
    menu/MenuAssets.cpp
    menu/MenuLayout.cpp
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

add_executable(sprviewer
    tools/sprviewer.cpp
    Window.cpp
    gfx/Shader.cpp
    gfx/Font.cpp
    gfx/UIRenderer.cpp
    gfx/WinVfxRenderer.cpp
)

target_include_directories(sprviewer PRIVATE
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(sprviewer PRIVATE
    neoslancer_data
    ${SDL2_LIBRARIES}
    SDL2_ttf::SDL2_ttf
    OpenGL::GL
    GLEW::GLEW
)

target_compile_options(sprviewer PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
)

add_executable(tgaviewer
    tools/tgaviewer.cpp
    Window.cpp
    gfx/Shader.cpp
    gfx/Font.cpp
    gfx/UIRenderer.cpp
)

target_include_directories(tgaviewer PRIVATE
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(tgaviewer PRIVATE
    neoslancer_data
    ${SDL2_LIBRARIES}
    SDL2_ttf::SDL2_ttf
    OpenGL::GL
    GLEW::GLEW
)

target_compile_options(tgaviewer PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
)
