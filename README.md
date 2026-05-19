# Virtual Desktop Switcher for Windows 11
A tool to switch between virtual desktops on Windows 11 by using the mouse.

[Japanese README](README.ja.md)


## Installation
- Download the zip file from https://github.com/saltpp/VirtualDesktopSwitcher/releases and then extract it
- Place VDS.exe, VDS.ini and VirtualDesktopAccessor.dll in any directory
- Launch VDS.exe, a ↔ icon will be added in the task tray


## Uninstallation
- Remove VDS.exe, VDS.ini and VirtualDesktopAccessor.dll
- This app doesn't use the registry


## Usage
- You can switch between virtual desktops by placing the mouse cursor on the left/right edge of the screen
- You need to create several virtual desktops before using this app. ([Win]+[Tab] → [New desktop] or [Ctrl]+[Win]+[d])


## Note
- Windows Defender SmartScreen may display a warning when launching the app for the first time. If the warning appears, click "More info" and then "Run anyway".


## Demo
- https://youtu.be/0UlkJ2dO8nw


## Acknowledgements
- This app uses the [VirtualDesktopAccessor](https://github.com/Ciantic/VirtualDesktopAccessor) library.


## History
- Ver.1.3.1
  - Fixed an issue where a pinned window became unpinned after switching virtual desktops while dragging it

- Ver.1.3.0
  - Support Windows 11 by using the [VirtualDesktopAccessor](https://github.com/Ciantic/VirtualDesktopAccessor) library

- Ver.1.2.0
  - Modified log display area
  - Modified the README.md/.html

- Ver.1.1.0
  - Added function to switch virtual desktop while dragging window

- Ver.1.0.0
  - First release


## License
- MIT
