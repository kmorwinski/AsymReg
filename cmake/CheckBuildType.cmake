macro(check_build_type)

    # user has not defined CMAKE_BUILD_TYPE:
    if("${CMAKE_BUILD_TYPE}" STREQUAL "")
        message(FATAL_ERROR "You have to set CMAKE_BUILD_TYPE. If you have no clue, then use RelWithDebInfo: \n\"cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..\"")
    endif()

    # user has defined CMAKE_BUILD_TYPE. Let's see if we know and support it
    string(TOUPPER ${CMAKE_BUILD_TYPE} type)
    if((NOT "${type}" STREQUAL "DEBUG")          AND
       (NOT "${type}" STREQUAL "RELEASE")        AND
       (NOT "${type}" STREQUAL "RELWITHDEBINFO") AND
       (NOT "${type}" STREQUAL "MINSIZEREL"))
        # ... no we dont!!!
        message(FATAL_ERROR "Your selected CMAKE_BUILD_TYPE: \"${CMAKE_BUILD_TYPE}\" is wrong or unkwon. Please use \"Debug\", \"Relase\", \"RelWithDebInfo\" or \"MinSizeRel\".")
    endif()

endmacro(check_build_type)
