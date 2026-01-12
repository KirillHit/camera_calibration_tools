set(APP_SOURCES
    src/main.cpp
    src/mainwindow.cpp

    src/image_sources/image_source_worker.cpp
    src/image_sources/file_image_source.cpp
    src/image_sources/camera_image_source.cpp
    src/image_sources/stream_image_source.cpp

    src/dialogs/camera_settings_dialog.cpp
    src/dialogs/stream_settings_dialog.cpp

    src/tools/line_based/line_based_view.cpp
    src/tools/line_based/line_based_worker.cpp
    src/tools/chessboard/chessboard_worker.cpp
)

set(APP_HEADERS
    include/app/mainwindow.hpp

    include/app/dialogs/dialogs.hpp
    include/app/dialogs/camera_settings_dialog.hpp
    include/app/dialogs/stream_settings_dialog.hpp

    include/app/image_sources/image_sources.hpp
    include/app/image_sources/image_source_interface.hpp
    include/app/image_sources/image_source_worker.hpp
    include/app/image_sources/file_image_source.hpp
    include/app/image_sources/camera_image_source.hpp
    include/app/image_sources/stream_image_source.hpp

    include/app/tools/line_based/line_based_view.hpp
    include/app/tools/line_based/line_based_worker.hpp
    include/app/tools/chessboard/chessboard_worker.hpp
)

qt_add_executable(calib_tools
    ${APP_SOURCES}
    ${APP_HEADERS}
    mainwindow.ui
    icons/calib_tools.rc
)

qt_add_resources(calib_tools "icons"
    PREFIX "/icons"
    FILES icons/calib_tools.png
)

qt_add_translations(calib_tools
    TS_FILES translations/calib_tools_ru.ts
)

target_include_directories(calib_tools PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(calib_tools PRIVATE
    calib_tools_lib
    Qt6::Widgets
    Qt6::Concurrent
    Qt6::PrintSupport
    ${OpenCV_LIBS}
)

set_target_properties(calib_tools PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

install(TARGETS calib_tools
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)

if(UNIX AND NOT APPLE)
   install(FILES icons/calib_tools.desktop
        DESTINATION share/applications
    )

    install(FILES icons/calib_tools.png
        DESTINATION share/icons/hicolor/512x512/apps
    )
endif()

if (WIN32)
    install(FILES
        $<TARGET_RUNTIME_DLLS:calib_tools>
        DESTINATION bin
    )

    qt_generate_deploy_app_script(
        TARGET calib_tools
        OUTPUT_SCRIPT deploy_script
    )

    install(SCRIPT ${deploy_script})
endif()