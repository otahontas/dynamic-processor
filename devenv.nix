{ pkgs, lib, ... }:
{
  # C++ build tools
  packages = with pkgs; [
    cmake
    jq
    ninja
  ];

  # Nix clang only supports native arm64, not universal binary
  env.CMAKE_OSX_ARCHITECTURES = lib.mkDefault "arm64";

  devenv-base = {
    # Extend treefmt with clang-format for C++
    treefmt = {
      programs.clang-format.enable = true;
    };

    # Extra gitignore entries for this project
    gitignore.extraEntries = [
      ".DS_Store"
      ".cache"
      ".vscode"
      "Builds"
      "JuceLibraryCode"
      "build"
      "compile_commands.json"
    ];

    # Extra nvim LSPs for C++ development
    nvim.extraLsps = [ "clangd" ];
  };

  tasks = {
    "project:configure" = {
      description = "Configure cmake build (input: build_type, default Debug)";
      input.build_type = "Debug";
      exec = ''
        BUILD_TYPE=$(echo "''$DEVENV_TASK_INPUT" | jq -r '.build_type // "Debug"')
        echo "Configuring for build type: ''$BUILD_TYPE"
        if [ -n "''${CMAKE_OSX_ARCHITECTURES}" ]; then
          cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE="''$BUILD_TYPE" -DCMAKE_OSX_ARCHITECTURES="''${CMAKE_OSX_ARCHITECTURES}" -S . -B build
        else
          cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE="''$BUILD_TYPE" -S . -B build
        fi
        if [ -d "build" ]; then
          ln -sf build/compile_commands.json ./compile_commands.json
        else
          echo "Build directory 'build' not created. Configuration failed."
          exit 1
        fi
      '';
    };
    "project:build" = {
      description = "Build target (inputs: target, format, config, jobs)";
      after = [ "project:configure" ];
      input = {
        target = "dynamics_processor";
        format = "Standalone";
        config = "Debug";
        jobs = 4;
      };
      exec = ''
        TARGET=$(echo "''$DEVENV_TASK_INPUT" | jq -r '.target // "dynamics_processor"')
        FORMAT=$(echo "''$DEVENV_TASK_INPUT" | jq -r '.format // "Standalone"')
        CONFIG=$(echo "''$DEVENV_TASK_INPUT" | jq -r '.config // "Debug"')
        JOBS=$(echo "''$DEVENV_TASK_INPUT" | jq -r '.jobs // 4')
        cmake --build build --target "''${TARGET}_''${FORMAT}" --config "''${CONFIG}" --parallel "''${JOBS}"
      '';
    };
  };
}
