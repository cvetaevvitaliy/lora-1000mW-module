##
# @brief This is part of the project 'LoRaTelemetryModules'
# @file CMakeLists.txt
# Copyright (c) Vitaliy Nimych <vitaliy.nimych@gmail.com>
# Created 18.10.2021
# License-Identifier: ???
##

cmake_minimum_required(VERSION 3.20.0)
set(BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/custom_board)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(LoRaTelemetryModules)

target_sources(app PRIVATE src/main.c)
