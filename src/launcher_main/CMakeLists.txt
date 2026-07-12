add_executable(game_launcher)

set_target_properties(game_launcher PROPERTIES
    PREFIX ""
)

target_include_directories(game_launcher
    PRIVATE
        ${CMAKE_SOURCE_DIR}/common
        ${CMAKE_SOURCE_DIR}/public
)

target_link_libraries(game_launcher
    PRIVATE
    tier0::tier0
    tier1::tier1
    tier2::tier2
    mathlib::mathlib
    vstdlib::vstdlib
    matsys_controls::matsys_controls
)

if (OS_LINUX)
    target_link_libraries(game_launcher
        PRIVATE
        dl
        m
        pthread
        rt
        z
        GL
        SDL2
    )
endif ()

target_link_directories(game_launcher
    PRIVATE
    ${LIBCOMMON}
    ${LIBPUBLIC}
)

target_sources_grouped(
    TARGET game_launcher
    NAME "Source Files"
    FILES
    main.cpp
    resource.h
)


