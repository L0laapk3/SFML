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
#include "SFML/System/Vector2.hpp"
#include <SFML/Window/Unix/Display.hpp>
#include <SFML/Window/Unix/MonitorImplX11.hpp>
#include <SFML/Window/Unix/Utils.hpp>
#include <SFML/Window/VideoModeDesktop.hpp>

#include <SFML/System/Err.hpp>

#include <X11/Xlib.h>

#include <algorithm>
#include <memory>
#include <ostream>


namespace sf::priv
{

////////////////////////////////////////////////////////////
void XDeleter<XRRScreenConfiguration>::operator()(XRRScreenConfiguration* config) const
{
	XRRFreeScreenConfigInfo(config);
}


////////////////////////////////////////////////////////////
std::shared_ptr<Display> MonitorImplX11::openXDisplay() {
    // Open a connection with the X server
    auto display = openDisplay();
    if (!display)
    {
        err() << "Failed to connect to the X server while trying to get the supported video modes" << std::endl;
        throw X11Exception("Failed to connect to the X server");
    }

    // Check if the XRandR extension is present
    int version = 0;
    if (!XQueryExtension(display.get(), "RANDR", &version, &version, &version))
    {
        // XRandr extension is not supported: we cannot get the video modes
        err() << "Failed to use the XRandR extension while trying to get the supported video modes" << std::endl;
        throw X11Exception("Failed to use the XRandR extension");
    }

	return display;
}


////////////////////////////////////////////////////////////
MonitorImplX11::MonitorImplX11(std::shared_ptr<Display> display, int screen)
    : m_display(std::move(display))
    , m_screen(screen)
    , m_config(X11Ptr<XRRScreenConfiguration>(
        XRRGetScreenInfo(m_display.get(), RootWindow(m_display.get(), m_screen))))
{
    if (!m_config)
    {
        err() << "Failed to retrieve the screen configuration while trying to get the supported video modes"
                << std::endl;
        throw X11Exception("Failed to retrieve the screen configuration");
    }
}


////////////////////////////////////////////////////////////
std::unique_ptr<MonitorImpl> MonitorImplX11::createPrimaryMonitor()
{
	auto display = openXDisplay();

    auto screen = DefaultScreen(display.get());

    // Retrieve the default screen number
    return std::make_unique<MonitorImplX11>(std::move(display), screen);
}

////////////////////////////////////////////////////////////
std::vector<std::unique_ptr<MonitorImpl>> MonitorImplX11::createAllMonitors()
{
	auto display = openXDisplay();

	const auto numScreens = ScreenCount(display.get());

	std::vector<std::unique_ptr<MonitorImpl>> monitors;
	monitors.reserve(static_cast<size_t>(numScreens));

	for (int i = 0; i < numScreens; ++i)
	{
		monitors.push_back(std::make_unique<MonitorImplX11>(display, i));
	}

	return monitors;
}

////////////////////////////////////////////////////////////
std::vector<VideoMode> MonitorImplX11::getFullscreenModes()
{
    std::vector<VideoMode> modes;

    // Get the available screen sizes
    int            nbSizes = 0;
    XRRScreenSize* sizes   = XRRConfigSizes(m_config.get(), &nbSizes);
    if (sizes && (nbSizes > 0))
    {
        // Get the list of supported depths
        int        nbDepths = 0;
        const auto depths   = X11Ptr<int[]>(XListDepths(m_display.get(), m_screen, &nbDepths));
        if (depths && (nbDepths > 0))
        {
            // Combine depths and sizes to fill the array of supported modes
            for (std::size_t i = 0; i < static_cast<std::size_t>(nbDepths); ++i)
            {
                for (int j = 0; j < nbSizes; ++j)
                {
                    // Convert to VideoMode
                    VideoMode mode({
						static_cast<unsigned int>(sizes[j].width),
						static_cast<unsigned int>(sizes[j].height)
					}, static_cast<unsigned int>(depths[i]));

                    Rotation currentRotation = 0;
                    XRRConfigRotations(m_config.get(), &currentRotation);

                    if (currentRotation == RR_Rotate_90 || currentRotation == RR_Rotate_270)
                        std::swap(mode.size.x, mode.size.y);

                    // Add it only if it is not already in the array
                    if (std::find(modes.begin(), modes.end(), mode) == modes.end())
                        modes.push_back(mode);
                }
            }
        }
    }

    return modes;
}


////////////////////////////////////////////////////////////
[[deprecated("Warning: the location attribute has not yet been implemented on this platform.")]]
VideoModeDesktop MonitorImplX11::getDesktopMode()
{
    VideoMode desktopMode;

    // Get the current video mode
    Rotation  currentRotation = 0;
    const int currentMode     = XRRConfigCurrentConfiguration(m_config.get(), &currentRotation);

    // Get the available screen sizes
    int            nbSizes = 0;
    XRRScreenSize* sizes   = XRRConfigSizes(m_config.get(), &nbSizes);
    if (sizes && (nbSizes > 0))
    {
        desktopMode = VideoMode({
			static_cast<unsigned int>(sizes[currentMode].width),
			static_cast<unsigned int>(sizes[currentMode].height)
		}, static_cast<unsigned int>(DefaultDepth(m_display.get(), m_screen)));

        Rotation modeRotation = 0;
        XRRConfigRotations(m_config.get(), &modeRotation);

        if (modeRotation == RR_Rotate_90 || modeRotation == RR_Rotate_270)
            std::swap(desktopMode.size.x, desktopMode.size.y);
    }

    return VideoModeDesktop{ desktopMode, sf::Vector2i() };
}

} // namespace sf::priv
