#!/usr/bin/env bash
set -euo pipefail

# Build script for the C++ LevelEditor.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_SRC_DIR="${SCRIPT_DIR}/.."
BUILD_DIR="${PROJECT_SRC_DIR}/LevelEditor/build"
CACHE_FILE="${BUILD_DIR}/CMakeCache.txt"
EXPECTED_SOURCE_DIR="${PROJECT_SRC_DIR}"

if [ ! -f "${PROJECT_SRC_DIR}/CMakeLists.txt" ]; then
  echo "Project source directory not found: ${PROJECT_SRC_DIR}" >&2
  exit 1
fi

if [ -f "${CACHE_FILE}" ] && ! grep -q "^CMAKE_HOME_DIRECTORY:INTERNAL=${EXPECTED_SOURCE_DIR}$" "${CACHE_FILE}"; then
  echo "Existing CMake cache was generated from a different source directory. Recreating ${BUILD_DIR}."
  rm -rf "${BUILD_DIR}"
fi

mkdir -p "${BUILD_DIR}"

echo "Configuring CMake (source: ${PROJECT_SRC_DIR}, build: ${BUILD_DIR})"
cmake -S "${PROJECT_SRC_DIR}" -B "${BUILD_DIR}"

echo "Building LevelEditor target"
cmake --build "${BUILD_DIR}" --config Release --target LevelEditor

echo "LevelEditor build finished. Artifacts are under ${BUILD_DIR}."
