set(src_files)
set(hdr_files)

# kiconutils library:
add_library(kiconutils SHARED
    kiconutils.cpp
)

set_target_properties(kiconutils PROPERTIES
    AUTMOC TRUE                          # enable MOC even if it is not needed here
    COMPILE_DEFINITIONS Q_DECL_OVERRIDE= # does not exist in Qt4
)

# set variables for parent CMakeLists.txt:
set(3rdparty_SRCS ${src_files} PARENT_SCOPE)
set(3rdparty_HDRS ${hdr_files} PARENT_SCOPE)
