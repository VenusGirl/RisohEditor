# How to build RisohEditor

## Requirements

- Visual Studio 2017 or 2019
- MSYS2
- CMake
- Inno Setup 5.6.1 (u)

ATL support is for Visual C++ only.

## Pre-task

- Load `src/RisohEditor_res.rc` by RisohEditor and save it as UTF-16 if you are using Visual Studio.

## Build task

- Open Visual Studio Command Prompt or MSYS2 and go to `RisohEditor` directory by `cd` command.
- Execute a `cmake` command line using CMake.
    - Please specify `-G "(generator)"`. To get the generator list, please specify `-G` only.
    - If ATL support is needed, then add `-DATL_SUPPORT=ON`.
    - If you want XP support, use `v141_xp` toolset (`-T v141_xp`).
    - `-A Win32` might be needed if your VS uses `x64` as default.
- Build it by using the solution file, Makefile or Ninja.
- Execute do_sign_all.bat .

## Making an installer

- Open `installer.iss` with Inno Setup.
- Build it.
