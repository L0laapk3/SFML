////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/VideoMode.hpp"
#include <SFML/Window/VideoModeDesktop.hpp>
#include <SFML/Window/Win32/MonitorImplWin32.hpp>

#include <algorithm>
#include <stdexcept>


namespace sf::priv
{


////////////////////////////////////////////////////////////
MonitorImplWin32::MonitorImplWin32(std::optional<StringType>&& deviceName) : m_deviceName(deviceName)
{
}


////////////////////////////////////////////////////////////
std::unique_ptr<MonitorImpl> MonitorImplWin32::createPrimaryMonitor()
{
    return std::make_unique<MonitorImplWin32>(std::nullopt);
}


////////////////////////////////////////////////////////////
std::vector<std::unique_ptr<MonitorImpl>> MonitorImplWin32::createAllMonitors()
{
	std::vector<std::unique_ptr<MonitorImpl>> monitors;

	DISPLAY_DEVICE displayDevice;
	displayDevice.cb = sizeof(displayDevice);
	for (DWORD i = 0; EnumDisplayDevices(nullptr, i, &displayDevice, 0); ++i)
	{
		if (displayDevice.StateFlags & DISPLAY_DEVICE_ACTIVE)
		{
			monitors.push_back(std::make_unique<MonitorImplWin32>(displayDevice.DeviceName));
		}
	}

	return monitors;
}


////////////////////////////////////////////////////////////
MonitorImplWin32::WinStringType MonitorImplWin32::getDeviceNameCStr() const {
	return m_deviceName ? m_deviceName->c_str() : nullptr;
}


////////////////////////////////////////////////////////////
std::optional<VideoModeDesktop> MonitorImplWin32::getVideoMode(DWORD modeIndex) const {
	DEVMODE win32Mode;
	win32Mode.dmSize = sizeof(win32Mode);
	win32Mode.dmDriverExtra = 0;
	if (!EnumDisplaySettings(getDeviceNameCStr(), modeIndex, &win32Mode))
		return std::nullopt;
	return VideoModeDesktop(
		VideoMode({ win32Mode.dmPelsWidth, win32Mode.dmPelsHeight }, win32Mode.dmDisplayFrequency, win32Mode.dmBitsPerPel),
		sf::Vector2i{ win32Mode.dmPosition.x, win32Mode.dmPosition.y }
	);
}


////////////////////////////////////////////////////////////
std::vector<VideoMode> MonitorImplWin32::getFullscreenModes()
{
    std::vector<VideoMode> modes;

    // Enumerate all available video modes for the primary display adapter
    for (DWORD modeIndex = 0; auto mode = getVideoMode(modeIndex); ++modeIndex)
    {
        // Add it only if it is not already in the array
        if (std::find(modes.begin(), modes.end(), *mode) == modes.end())
            modes.push_back(*mode);
    }

    return modes;
}


////////////////////////////////////////////////////////////
VideoModeDesktop MonitorImplWin32::getDesktopMode()
{
	auto mode = getVideoMode(ENUM_CURRENT_SETTINGS);
	if (!mode)
		throw std::runtime_error("Failed to retrieve the desktop video mode");

	return *mode;
}

} // namespace sf::priv
