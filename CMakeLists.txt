#==============================
# Requirements
#==============================

cmake_minimum_required(VERSION 3.10)

#==============================
# Project Settings
#==============================

if (NOT PROJECT_DECLARED)
    project("Project Icarus" LANGUAGES CXX)
    set(PROJECT_DECLARED)
endif()

if (NOT PROJECT_SUIT)
    set(PROJECT_SUIT ON)
endif()

#==============================
# Project Modules
#==============================

add_subdirectory("IcarusDetector")
add_subdirectory("IcarusChallenger")
add_subdirectory("SignalSender")

if (WITH_TEST)
endif()