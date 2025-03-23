# Counter Strike 2 game mod
This CS2 educational game mod is designed to improve the gameplay experience for all players in Counter Strike 2. It includes both aim assist and visual aids to assist players in their gameplay. 

# Installation
You can build it yourself with Microsoft Visual Studio, or use the precompiled binaries already in the build folder. 

# How it works
When the injector is run, the dll is manually mapped into the cs2 game process, and a new thread is created for it to run. It still relies on the windows API to do this though, as it calls RPM, WPM, CRT, etc. It also runs in user mode.
Offsets do not update automatically, but if you want you can update them yourself by editing player.h.

# Dependencies
Menu made with ImGui: https://github.com/ocornut/imgui
