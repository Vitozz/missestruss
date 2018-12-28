project( qmisestruss )
cmake_minimum_required( VERSION 3.1.0 )
set( CMAKE_AUTOMOC ON )
set( CMAKE_INCLUDE_CURRENT_DIR ON )
set( CMAKE_MODULE_PATH
	"${CMAKE_MODULE_PATH}"
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
	"${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules"
)

option( BUILD_TRANSLATIONS "Build project translations" ON )
option( USE_QT5 "Use Qt5" ON )
option( USE_CPACK "Use cpack to create packages" OFF )

find_package(Qt5Core QUIET)

if(NOT Qt5Core_FOUND AND(USE_QT5))
    set( USE_QT5 OFF )
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug" OR ( "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebugInfo"))
    add_definitions(-DIS_DEBUG)
else()
    add_definitions( -DQT_NO_DEBUG )
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${PROJECT_NAME}" )

include("${PROJECT_SOURCE_DIR}/version.cmake")
if(USE_CPACK)
    include("${PROJECT_SOURCE_DIR}/use_cpack.cmake")
endif()

include_directories(
	${CMAKE_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/gui
    ${CMAKE_SOURCE_DIR}/core
)
set( misestruss_SRC
	main.cpp
	gui/mainwindow.cpp
	core/misescalcnelast.cpp
	core/misescalcelast.cpp
)

set( misestruss_HDR
	gui/mainwindow.h
	core/misescalcnelast.h
	core/misescalcelast.h
	defines.h
)
set( misestruss_UI
	gui/mainwindow.ui
)
set( misestruss_QRC
	mises1.qrc
)
if(WIN32)
set( misestruss_RC
	Mises.rc
)
endif()
set( LANG_PATH langs )
set( misestruss_TRS
	${LANG_PATH}/${PROJECT_NAME}_ru.ts
	${LANG_PATH}/${PROJECT_NAME}_uk.ts
)
set(LICENSE_FILES
	COPYING
)

IF( WIN32 )
	set( QWT_ROOT "C:\\build\\qwt-6.1.2\\i386" CACHE STRING "Path to libgcrypt library" )
ENDIF()

if( USE_QT5 )
	find_package( Qt5 COMPONENTS Widgets Svg REQUIRED )
	find_package( LibQwt5 REQUIRED )
	include_directories( ${Qt5Gui_INCLUDE_DIRS} )
	qt5_add_resources( RSCS ${misestruss_QRC} )
    if( BUILD_TRANSLATIONS )
		find_package( Qt5LinguistTools REQUIRED )
		qt5_add_translation( TRS ${misestruss_TRS} )
	endif()
	qt5_wrap_ui( UIS ${misestruss_UI} )
	set( QT_DEPLIBS
		Qt5::Widgets
		Qt5::Svg
	)
else()
	find_package( Qt4 COMPONENTS QtGui QtCore QtSvg REQUIRED )
	find_package( LibQwt4 REQUIRED )
	include( ${QT_USE_FILE} )
	QT4_WRAP_UI( UIS ${misestruss_UI} )
	QT4_ADD_RESOURCES( RSCS ${misestruss_QRC} )
	if( ${BUILD_TRANSLATIONS} )
		QT4_ADD_TRANSLATION( TRS ${misestruss_TRS} )
	endif()
	set( QT_DEPLIBS
		Qt4::QtCore
		Qt4::QtGui
		Qt4::QtSvg
	)
endif()
add_definitions( ${QWT_DEFINITIONS} )
include_directories( ${QWT_INCLUDE_DIR} )

add_executable( ${PROJECT_NAME}
        WIN32
        ${misestruss_SRC}
        ${UIS}
        ${RSCS}
        ${TRS}
        ${misestruss_RC}
)

set(CMAKE_CXX_STANDARD 14)

target_link_libraries ( ${PROJECT_NAME}
                        ${QT_DEPLIBS}
                        ${QWT_LIBRARY}
)

if( UNIX AND NOT (APPLE OR CYGWIN) )
	set( DATADIR_PATH ${CMAKE_INSTALL_PREFIX}/share )
	set( APPDATA_PATH ${DATADIR_PATH}/applications )
	set( ICON_FILES 
		icons/f\(x\)48.png
	)
	set( ICON_PATH
		${DATADIR_PATH}/${PROJECT_NAME}/icons
	)
	set( DESKTOP_FILES
		${PROJECT_NAME}.desktop
	)
	INSTALL( TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin )
	INSTALL( FILES ${ICON_FILES} DESTINATION ${ICON_PATH} )
	INSTALL( FILES ${DESKTOP_FILES} DESTINATION ${APPDATA_PATH} )
    if(BUILD_TRANSLATIONS)
        INSTALL( FILES ${TRS} DESTINATION ${DATADIR_PATH}/${PROJECT_NAME}/${LANG_PATH} )
    endif()
endif()
if( WIN32 )
	set ( CMAKE_BUILD_WITH_INSTALL_RPATH FALSE )
	INSTALL( TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX} )
    if(BUILD_TRANSLATIONS)
        INSTALL( FILES ${TRS} DESTINATION ${CMAKE_INSTALL_PREFIX}/langs )
    endif()
	INSTALL( FILES ${LICENSE_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX} )
endif()
