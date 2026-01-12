set(CTL_SOURCES
    src/camera_intrinsics.cpp
    src/cashed_undistort.cpp
    src/chessboard_calibrator.cpp
    src/line_based_calibrator.cpp
)

add_library(calib_tools_lib STATIC
    ${CTL_SOURCES}
)

target_link_libraries(calib_tools_lib PUBLIC
    ${OpenCV_LIBS}
    ${CERES_LIBRARIES}
)

target_include_directories(calib_tools_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
    ${CERES_INCLUDE_DIRS}
)

target_include_directories(calib_tools_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})