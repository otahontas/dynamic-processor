#!/bin/sh

# This command should be run to create the build folder.
# After that, just run the build.sh script specifying
# the target, build config (Debug/Release) and format (VST3/AU/Standalone).

# First argument: Build type (Debug, Release, RelWithDebInfo, MinSizeRel)
# Defaults to Debug if not provided
BUILD_TYPE=${1:-Debug}

echo "Configuring for build type: ${BUILD_TYPE}"

# Export compile commands AND set the build type
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -S . -B build

# link compile commands
# Only link if the build directory was successfully created
if [ -d "build" ]; then
  if [ -f "compile_commands.json" ]; then
    rm compile_commands.json # Remove existing symlink or file to avoid error
  fi
  ln -s build/compile_commands.json ./
else
  echo "Build directory 'build' not created. Configuration failed."
  exit 1
fi
