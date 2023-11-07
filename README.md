![Example](https://i.imgur.com/aLQOwy4.png)

[More screenshots](https://imgur.com/a/Z5LRbV0)

## Download
- [Stable release](https://github.com/KebsCS/KBotExt/releases/latest) (You will be notified when there's a new release)
- [Prerelease](https://github.com/KebsCS/KBotExt/releases/tag/prerelease)

## Features
* Works every patch
* Free ARAM boost
* Launch multiple clients
* Language changer
* Start any lobby/game
* Custom bots difficulty
* The fastest instalock, auto accept, instant message and auto ban
* Automatically pick a secondary, or dodge if your champion is banned
* Instantly mute everyone in champion select
* Dodge lobbies without closing the client
* Mass invite all friends to lobby
* Multi-search op.gg/u.gg/poro.gg etc. with all players in a champ select (works in ranked - lobby reveal)
* Set the best runes for your selected champion from op.gg (works even when runes aren't unlocked)
* Play any champion/skin for free (Refund exploit)
* Shows which map side you are on, in all gamemodes
* Set custom icon/background/status/rank/mastery visible for everyone
* Set glitched or empty challenge badges (tokens)
* Set invisible profile/lobby banner
* Info of any player using his nickname or id
* List of champions/skins with all info and ability to sort them
* Force close the client instantly
* Mass delete all friends sorted by folders
* Accept or delete all friend requests
* Set custom in-game minimap scale
* Disenchant any loot you want with 1 click (mass disenchant)
* Champion name to id lookup
* Send any custom request to LCU, Riot Client, RTMP, Store and Ledge
* Stream proof
* IFEO debugger for using HTTP debuggers (Fiddler, Charles, etc.)
* Log cleaner
* Automatically saves your preferences
* Unicode support and customizable window size
* 1 click login with automated client open
* Force client to run without admin
* (Partially patched) Check the email linked to account you're logged on
* (Patched) Free skin and free champion (Riot Girl Tristana)

## Development

- **Installing required libraries**
  1. Install [GIT for windows](https://git-scm.com/download/win)
  2. Open Windows Command Prompt (**CMD**)
  3. Run `git clone https://github.com/microsoft/vcpkg.git`
  4. `cd vcpkg`
  5. `bootstrap-vcpkg.bat`
  6. `vcpkg integrate install`
  7. `vcpkg install freetype:x64-windows-static`
  8. `vcpkg install cpr:x64-windows-static`
  9. `vcpkg install jsoncpp:x64-windows-static`
- **Building the project**
  1. Clone the repository
  2. Open **KBotExt.sln** in Visual Studio (Recommended Visual Studio 2022)
  3. Set the solution platform to x64 Release
  4. Build the project
  5. Feel free to make a pull request with your changes :-)
