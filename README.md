# First-Time Chatter Dock for OBS

A dock plugin for OBS Studio that tracks first-time chatters in your Twitch stream and helps you greet them!

## Features

- **Twitch IRC Connection**: Connects directly to Twitch chat via WebSocket
- **First-Time Chatter Detection**: Tracks chatters who haven't chatted before in the current session
- **Bot Filtering**: Filters out common bots (customizable list)
- **Click-to-Mark**: Click on a chatter's name to mark them as greeted (strikethrough effect)
- **Reset Button**: Clear the list at any time
- **Daily Auto-Reset**: Automatically resets the session at midnight
- **Persistent State**: Remembers clicked chatters across sessions
- **Settings Dialog**: Easy configuration for channel, username, OAuth, and bot list
- **Connection Status Indicator**: Visual indicator showing connection state
- **Dark Theme**: Matches OBS Studio's dark theme

## Installation

1. Download the latest release for your platform
2. Extract the plugin to your OBS plugins folder:
   - **Windows**: `C:\Program Files\obs-studio\obs-plugins\64bit\`
   - **macOS**: `/Library/Application Support/obs-studio/plugins/`
   - **Linux**: `~/.config/obs-studio/plugins/`
3. Restart OBS Studio
4. Go to **View > Docks > First-Time Chatters** to show the dock

## Configuration

1. Click the ⚙ (gear) button in the dock to open settings
2. Enter your Twitch channel name (without #)
3. Enter your Twitch username
4. Get an OAuth token from [https://twitchapps.com/tmi/](https://twitchapps.com/tmi/)
5. (Optional) Customize the bot filter list
6. Click OK and then Connect

## Default Bot Filter List

The following bots are filtered by default:
- nightbot
- streamelements
- soundalerts
- moobot
- streamlabs
- tangiabot
- sery_bot
- mei_the_fox

You can customize this list in the settings dialog.

## Building from Source

### Requirements

| Platform | Tool |
|----------|------|
| Windows | Visual Studio 17 2022 |
| macOS | XCode 16.0 |
| Windows, macOS | CMake 3.30.5 |
| Ubuntu 24.04 | CMake 3.28.3 |
| Ubuntu 24.04 | `ninja-build` |
| Ubuntu 24.04 | `pkg-config` |
| Ubuntu 24.04 | `build-essential` |

### Build Steps

1. Clone the repository
2. Run CMake configure
3. Build the project

## License

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

## Screenshots

*Coming soon*
