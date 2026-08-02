#!/usr/bin/env bash
set -euo pipefail

# Build script for the C# CrazedCaver project.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CRAZED_DIR="${SCRIPT_DIR}/../CrazedCaver"
OUT_DIR="${CRAZED_DIR}/build"

if [ ! -d "${CRAZED_DIR}" ]; then
  echo "CrazedCaver project directory not found: ${CRAZED_DIR}" >&2
  exit 1
fi

mkdir -p "${OUT_DIR}"

CONTENT_DIR="${OUT_DIR}/Content"

echo "Publishing CrazedCaver to: ${OUT_DIR}"
dotnet publish "${CRAZED_DIR}/CrazedCaver.csproj" -c Release -o "${OUT_DIR}"

if [ -d "${CRAZED_DIR}/Content/bin/DesktopGL/Content" ]; then
  mkdir -p "${CONTENT_DIR}"
  cp -R "${CRAZED_DIR}/Content/bin/DesktopGL/Content/." "${CONTENT_DIR}/"
fi

echo "CrazedCaver publish finished. Artifacts are under ${OUT_DIR}."
