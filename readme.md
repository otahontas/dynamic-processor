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

### Debug mode

(allows debug symbols + extra warnings for debug builds)
Run:

```sh
./configure.sh Debug
./build.sh dynamics_processor [Standalone/AU/VST3]
```

### Release mode

Run:

```sh
./configure.sh Release
./build.sh dynamics_processor [Standalone/AU/VST3]
```
