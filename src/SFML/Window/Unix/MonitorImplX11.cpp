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
#include <SFML/Window/Monitor.hpp>
#include <SFML/Window/VideoModeDesktop.hpp>

#include <SFML/System/Err.hpp>

#include <X11/Xlib.h>

#include <algorithm>
#include <memory>
#include <ostream>


namespace sf::priv
{

////////////////////////////////////////////////////////////
void XDeleter<XRRCrtcInfo>::operator()(XRRCrtcInfo* info) const
{
	XRRFreeCrtcInfo(info);
}


////////////////////////////////////////////////////////////
std::shared_ptr<Display> MonitorImplX11::openXDisplay() {
    // Open a connection with the X server
    auto display = openDisplay();
    if (!display)
    {
        err() << "Failed to connect to the X server while trying to get the supported video modes" << std::endl;
        throw Monitor::MonitorException("Failed to connect to the X server");
    }

    // Check if the XRandR extension is present
    int version = 0;
    if (!XQueryExtension(display.get(), "RANDR", &version, &version, &version))
    {
        // XRandr extension is not supported: we cannot get the video modes
        err() << "Failed to use the XRandR extension while trying to get the supported video modes" << std::endl;
        throw Monitor::MonitorException("Failed to use the XRandR extension");
    }

	return display;
}


////////////////////////////////////////////////////////////
std::shared_ptr<XRRScreenConfiguration> MonitorImplX11::getScreenConfig(const std::shared_ptr<Display>& display, int screen) {
	auto config = std::shared_ptr<XRRScreenConfiguration>(
		XRRGetScreenInfo(display.get(), RootWindow(display.get(), screen)),
		[](auto* cfg) { XRRFreeScreenConfigInfo(cfg); }
	);

	if (!config)
	{
		err() << "Failed to retrieve the screen configuration" << std::endl;
		throw Monitor::MonitorException("Failed to retrieve the screen configuration");
	}

	return config;
}

////////////////////////////////////////////////////////////
std::shared_ptr<XRRScreenResources> MonitorImplX11::getScreenResources(const std::shared_ptr<Display>& display, int screen) {
	auto resources = std::shared_ptr<XRRScreenResources>(
		XRRGetScreenResources(display.get(), RootWindow(display.get(), screen)),
		[](auto* res) { XRRFreeScreenResources(res); }
	);

	if (!resources)
	{
		err() << "Failed to retrieve the screen resources" << std::endl;
		throw Monitor::MonitorException("Failed to retrieve the screen resources");
	}

	return resources;
}



////////////////////////////////////////////////////////////
MonitorImplX11::MonitorImplX11(std::shared_ptr<Display> display, int screen, std::shared_ptr<XRRScreenConfiguration> screenConfig, std::shared_ptr<XRRScreenResources> screenResources, int monitor)
	: m_display(std::move(display))
	, m_screen(screen)
	, m_screenConfig(std::move(screenConfig))
	, m_screenResources(std::move(screenResources))
	, m_monitor(monitor)
	, m_crtcInfo(X11Ptr<XRRCrtcInfo>(
		XRRGetCrtcInfo(m_display.get(), m_screenResources.get(), m_screenResources->crtcs[m_monitor])))
{
	if (!m_crtcInfo)
	{
		err() << "Failed to retrieve the screen configuration" << std::endl;
		throw Monitor::MonitorException("Failed to retrieve the CRTC information");
	}
}


////////////////////////////////////////////////////////////
std::unique_ptr<MonitorImpl> MonitorImplX11::createPrimaryMonitor()
{
    auto display = openXDisplay();
    auto screen = DefaultScreen(display.get());

    // Get the primary output for the screen
    RROutput primaryOutput = XRRGetOutputPrimary(display.get(), RootWindow(display.get(), screen));

    auto screenResources = getScreenResources(display, screen);

    int monitorIndex = 0; // If the primary output is not found, default to the first monitor
    for (int i = 0; i < screenResources->noutput; ++i)
        if (screenResources->outputs[i] == primaryOutput) {
            monitorIndex = i;
            break;
        }

    // Create the MonitorImplX11 object
    return std::make_unique<MonitorImplX11>(std::move(display), screen, getScreenConfig(display, screen), std::move(screenResources), monitorIndex);
}

////////////////////////////////////////////////////////////
std::vector<std::unique_ptr<MonitorImpl>> MonitorImplX11::createAllMonitors()
{
	auto display = openXDisplay();

	const auto numScreens = ScreenCount(display.get());

	std::vector<std::unique_ptr<MonitorImpl>> monitors;

	// iterate over screens
	for (int screen = 0; screen < numScreens; ++screen)
	{
		auto config          = getScreenConfig(display, screen);
		auto screenResources = getScreenResources(display, screen);

		monitors.reserve(monitors.size() + static_cast<size_t>(screenResources->ncrtc));

		for (int monitor = 0; monitor < screenResources->ncrtc; ++monitor)
			monitors.push_back(std::make_unique<MonitorImplX11>(display, screen, config, screenResources, monitor));
	}

	return monitors;
}

////////////////////////////////////////////////////////////
VideoMode MonitorImplX11::getVideoMode(int depth) const {
	// Get the mode info for the current mode
	XRRModeInfo* modeInfo = nullptr;
	for (int i = 0; i < m_screenResources->nmode; ++i) {
		if (m_screenResources->modes[i].id == m_crtcInfo->mode) {
			modeInfo = &m_screenResources->modes[i];
			break;
		}
	}

	if (!modeInfo) {
		err() << "Failed to find the mode info for the current mode" << std::endl;
		throw Monitor::MonitorException("Failed to find the mode info for the current mode");
	}

	// Calculate the refresh rate
	float refreshRate = static_cast<float>(modeInfo->dotClock) / static_cast<float>(modeInfo->hTotal * modeInfo->vTotal);

	VideoMode mode = VideoMode({
			static_cast<unsigned int>(m_crtcInfo->width),
			static_cast<unsigned int>(m_crtcInfo->height),
		},
		refreshRate,
		static_cast<unsigned int>(depth)
	);

	if (m_crtcInfo->rotation == RR_Rotate_90 || m_crtcInfo->rotation == RR_Rotate_270)
		std::swap(mode.size.x, mode.size.y);

	return mode;
}

////////////////////////////////////////////////////////////
std::vector<VideoMode> MonitorImplX11::getFullscreenModes()
{
	std::vector<VideoMode> modes;

	// Get the list of supported depths
	int        nbDepths = 0;
	const auto depths   = X11Ptr<int[]>(XListDepths(m_display.get(), m_screen, &nbDepths));
	if (depths && (nbDepths > 0))
	{
		// Combine depths and sizes to fill the array of supported modes
		for (std::size_t i = 0; i < static_cast<std::size_t>(nbDepths); ++i)
		{
			auto mode = getVideoMode(depths[i]);

			// Add it only if it is not already in the array
			if (std::find(modes.begin(), modes.end(), mode) == modes.end())
				modes.push_back(mode);
		}
	}

	return modes;
}


////////////////////////////////////////////////////////////
VideoModeDesktop MonitorImplX11::getDesktopMode()
{
	return VideoModeDesktop{
		getVideoMode(DefaultDepth(m_display.get(), m_screen)),
		sf::Vector2i(m_crtcInfo->x, m_crtcInfo->y)
	};
}

} // namespace sf::priv
