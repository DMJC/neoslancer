add_executable(neoslancer
    main.cpp
    Application.cpp
    Window.cpp
)

target_include_directories(neoslancer PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(neoslancer PRIVATE
    ${SDL2_LIBRARIES}
    OpenGL::GL
    GLEW::GLEW
)

target_compile_options(neoslancer PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
)
