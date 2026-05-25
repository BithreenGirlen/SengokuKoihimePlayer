cmake_minimum_required (VERSION 3.8)

set(NLOHMANN_JSON_VERSION "3.12.0")

set(NLOHMANN_JSON_DIR ${CMAKE_CURRENT_LIST_DIR}/nlohmann)

# Donwload JSON for Modern C++
if (NOT EXISTS "${NLOHMANN_JSON_DIR}")
	set(SINGLE_HEADER_FILENAME "json.hpp")
	set(NLOHMANN_JSON_URL "https://github.com/nlohmann/json/releases/download/v${NLOHMANN_JSON_VERSION}/${SINGLE_HEADER_FILENAME}")
	file(DOWNLOAD "${NLOHMANN_JSON_URL}" "${NLOHMANN_JSON_DIR}/${SINGLE_HEADER_FILENAME}")
endif()

