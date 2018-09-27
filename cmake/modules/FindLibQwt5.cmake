if( QWT_INCLUDE_DIR AND QWT_LIBRARY )
	# in cache already
	set(QWT_FIND_QUIETLY TRUE)
endif( QWT_INCLUDE_DIR AND QWT_LIBRARY )

set( QWT_ROOT "" CACHE STRING "Path to libqwt library" )

find_path(
	QWT_INCLUDE_DIR qwt.h
	HINTS
	/usr/include/qwt
	/usr/include/qwt6
	${QWT_ROOT}/include
)

find_library(
	QWT_LIBRARY
	NAMES qwt qwtd qwt6 qwt6-qt5
	HINTS
	${QWT_ROOT}/lib
	${QWT_ROOT}/bin
)
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
				LibQwt
				DEFAULT_MSG
				QWT_LIBRARY
				QWT_INCLUDE_DIR
)
if( QWT_FOUND )
	set( QWT_LIBRARIES ${QWT_LIBRARY} )
	set( QWT_INCLUDE_DIRS ${QWT_INCLUDE_DIR} )
endif( QWT_FOUND )

mark_as_advanced( QWT_INCLUDE_DIR QWT_LIBRARY )
