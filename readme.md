# Dynamics processor

Noise gate + compressor

## Pre-requisites

**MacOS**:
 - [XCode](https://developer.apple.com/xcode/)
 - [Developer Command Line Tools](https://www.youtube.com/watch?v=sF9UszljnZU)
 - [CMake](https://cmake.org/) - Can also be installed with [Homebrew](https://formulae.brew.sh/formula/cmake)

**Windows**:
 - [MS Visual Studio](https://visualstudio.microsoft.com/vs/community/) (2019/2022) - Community Edition is free.
 - [Git Bash](https://gitforwindows.org/)
 - [CMake](https://cmake.org/)

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
