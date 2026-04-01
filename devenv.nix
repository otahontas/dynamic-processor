{ pkgs, lib, ... }:
{
  # C++ build tools
  packages = with pkgs; [
    cmake
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

  # Symlink compile_commands.json on shell entry
  enterShell = ''
    if [ -f build/compile_commands.json ] && [ ! -L compile_commands.json ]; then
      ln -sf build/compile_commands.json ./compile_commands.json
    fi
  '';
}
