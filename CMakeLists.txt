CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(tuner)

SET(PACKAGE ${PROJECT_NAME})
SET(TUNERLIB "tunerapi")

INCLUDE(GNUInstallDirs)
INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED
	elementary
	glib-2.0 gio-2.0 gio-unix-2.0 evas edje
	ecore-x ecore ecore-input ecore-ipc cairo cairo-ft
	freetype2
)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)


SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -g -Wall")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -fdata-sections -ffunction-sections -Wl,--gc-sections -fpie")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CFLAGS} -g -Wall -Werror=overloaded-virtual -std=c++11")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fdata-sections -ffunction-sections -Wl,--gc-sections -fpie -Wno-unused-parameter")

INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_SOURCE_DIR}/../inc
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}
)

ADD_DEFINITIONS("-DLOG_PRINTF")
ADD_DEFINITIONS("-DHOST")
#ADD_DEFINITIONS("-DFEATURE_TTEXT")
ADD_DEFINITIONS("-DFEATURE_TBUILDER")
ADD_DEFINITIONS("-DEXPORT_API=__attribute__((visibility(\"default\")))")

SET(SRCS
	src/TLibrary.cpp
	src/TObject.cpp
	src/TWindow.cpp
	src/TContainer.cpp
	src/TTransition.cpp
	src/TImage.cpp
	src/TTextBuilder.cpp
  src/TText.cpp
	src/TMenu.cpp
	src/TScroller.cpp
	src/TVerticalList.cpp
	src/TSimpleList.cpp
	src/TScaleRotationMenu.cpp
)

ADD_LIBRARY(${TUNERLIB} SHARED ${SRCS})
TARGET_LINK_LIBRARIES(${TUNERLIB} ${pkgs_LDFLAGS} "-ldl -L${CMAKE_BINARY_DIR}")

SET(TARGETS
	vlist
	slist
)

FOREACH(target ${TARGETS})
  ADD_EXECUTABLE(${target} app/${target}.cpp app/main.c)
		TARGET_LINK_LIBRARIES(${target} ${pkgs_LDFLAGS} "-pie -ldl -L${CMAKE_BINARY_DIR} -Wl,-rpath=${CMAKE_BINARY_DIR} -l${TUNERLIB}")
		ADD_DEPENDENCIES(${target} ${TUNERLIB})
ENDFOREACH(target)

