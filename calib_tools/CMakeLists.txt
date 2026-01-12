cmake_minimum_required(VERSION 3.21)

project(
    calib_tools
    VERSION 1.0.0
    DESCRIPTION "Camera Calibration Tools application"
    LANGUAGES CXX
)

# Global settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies
find_package(Qt6 REQUIRED COMPONENTS Widgets Concurrent PrintSupport LinguistTools)
find_package(OpenCV REQUIRED)
find_package(Ceres REQUIRED)

qt_standard_project_setup(
    I18N_SOURCE_LANGUAGE en
    I18N_TRANSLATED_LANGUAGES ru
)

# Subprojects
add_subdirectory(calib_tools_lib)
add_subdirectory(app)

# Packaging (CPack)
include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME               ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION            ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_HOMEPAGE_URL       "https://github.com/KirillHit/calib_tools")

# Debian
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Kirill Hitushkin <khitushkink@mail.ru>")
set(CPACK_DEBIAN_PACKAGE_SECTION    "devel")
set(CPACK_DEBIAN_PACKAGE_PRIORITY   "optional")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS  ON)

# Windows
set(CPACK_NSIS_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_NSIS_DISPLAY_NAME "Camera Calibration Tools")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}")
set(CPACK_NSIS_CREATE_ICONS_EXTRA
    "CreateShortCut '$SMPROGRAMS\\\\Calib Tools.lnk' '$INSTDIR\\\\bin\\\\calib_tools.exe'"
)
set(CPACK_NSIS_CREATE_ICONS
    "CreateShortCut '$DESKTOP\\\\Calib Tools.lnk' '$INSTDIR\\\\bin\\\\calib_tools.exe'"
)
set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/app/icons/calib_tools.ico")

include(CPack)