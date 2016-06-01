/** @file cmake_config.h
 *  @brief This file contains a definitions interface to CMake.
 *  @author Jan Voges (voges)
 *  @bug No known bugs
 */

/*
 *  Changelog
 *  YYYY-MM-DD: what (who)
 */

#ifndef CMAKE_CONFIG_H
#define CMAKE_CONFIG_H

// these header files are generated by CMake
#include "git.h"
#include "timestamp.h"
#include "version.h"

#define BUILD_YEAR \
    ( \
        (TIMESTAMP_UTC[ 0] - '0') * 1000 +\
        (TIMESTAMP_UTC[ 1] - '0') *  100 +\
        (TIMESTAMP_UTC[ 2] - '0') *   10 +\
        (TIMESTAMP_UTC[ 3] - '0')\
    )

// this is a list of the defines generated by CMake; uncomment and set these
// to build without CMake
//#define TIMESTAMP_UTC "timestamp_utc"
//#define BUILD_YEAR 1977
//#define GIT_BRANCH "master""
//#define GIT_COMMIT_HASH_SHORT "git_commit_hash_short"
//#define GIT_COMMIT_HASH_LONG "git_commit_hash_long"
//#define VERISON "1.0.0""
//#define VERSION_MAJOR 1
//#define VERSION_MINOR 0
//#define VERSION_PATCH 0

#endif // CMAKE_CONFIG_H
