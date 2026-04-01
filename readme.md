# Dynamics processor

Noise gate + compressor

## Pre-requisites

All build dependencies (CMake, Ninja) are provided via [devenv](https://devenv.sh/). Install:

- [Nix](https://nixos.org/download/)
- [devenv](https://devenv.sh/getting-started/)
- (optional) [direnv](https://direnv.net/) for automatic shell activation

**MacOS** additionally requires:

- [Xcode](https://developer.apple.com/xcode/) + Command Line Tools (for the system compiler and SDK headers)

**Windows**:

- [MS Visual Studio](https://visualstudio.microsoft.com/vs/community/) (2019/2022) - Community Edition is free.
- [Git Bash](https://gitforwindows.org/)

**Linux**:

- [Linux instructions](readme-linux.md)

## Cloning the Repository

If you have a [github account](https://docs.github.com/en/get-started/signing-up-for-github/signing-up-for-a-new-github-account) with a [SSH key setup](https://docs.github.com/en/authentication/connecting-to-github-with-ssh/adding-a-new-ssh-key-to-your-github-account):

```sh
git clone --recurse-submodules git@github.com:otahontas/dynamic-processor.git
```

or with https:

```sh
git clone --recurse-submodules https://github.com/otahontas/dynamic-processor.git
```

## Building

All commands run inside the devenv shell (`devenv shell` or auto-activated via direnv).

### Debug mode

```sh
devenv tasks run project:configure
devenv tasks run project:build
```

### Release mode

```sh
devenv tasks run project:configure --input build_type=Release
devenv tasks run project:build --input config=Release
```

### Other options

```sh
# Build a specific format (VST3, AU, Standalone)
devenv tasks run project:build --input format=VST3

# Override multiple inputs
devenv tasks run project:build --input target=dynamics_processor --input format=Standalone --input config=Debug --input jobs=8
```
