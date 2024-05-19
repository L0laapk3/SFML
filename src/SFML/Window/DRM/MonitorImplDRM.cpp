////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2024 Andrew Mickelson
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
#include <SFML/Window/DRM/DRMContext.hpp>
#include <SFML/Window/DRM/MonitorImplDRM.hpp>
#include <SFML/Window/VideoModeDesktop.hpp>

#include <SFML/System/Err.hpp>


namespace sf::priv
{
////////////////////////////////////////////////////////////
MonitorImplDRM::MonitorImplDRM() = default;


////////////////////////////////////////////////////////////
std::unique_ptr<MonitorImpl> MonitorImplDRM::createPrimaryMonitor()
{
    return std::make_unique<MonitorImplDRM>();
}


////////////////////////////////////////////////////////////
std::vector<std::unique_ptr<MonitorImpl>> MonitorImplDRM::createAllMonitors()
{
	throw std::runtime_error("MonitorImplDRM::createAllMonitors() is not implemented");
}


////////////////////////////////////////////////////////////
std::vector<VideoMode> MonitorImplDRM::getFullscreenModes()
{
    std::vector<VideoMode> modes;

    const Drm&          drm  = DRMContext::getDRM();
    drmModeConnectorPtr conn = drm.savedConnector;

    if (conn)
    {
        for (int i = 0; i < conn->count_modes; i++)
            modes.push_back(VideoMode({conn->modes[i].hdisplay, conn->modes[i].vdisplay}));
    }
    else
        modes.push_back(getDesktopMode());

    return modes;
}


////////////////////////////////////////////////////////////
VideoModeDesktop MonitorImplDRM::getDesktopMode()
{
    const Drm&         drm = DRMContext::getDRM();
    drmModeModeInfoPtr ptr = drm.mode;
    if (ptr)
        return VideoModeDesktop({ptr->hdisplay, ptr->vdisplay}, sf::Vector2i());
    else
        return VideoModeDesktop({0, 0}, sf::Vector2i());
}

} // namespace sf::priv
