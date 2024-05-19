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
#include <SFML/Window/Display.hpp>
#include <SFML/Window/DisplayImpl.hpp>
#include <SFML/Window/VideoMode.hpp>

#include <algorithm>
#include <functional>
#include <memory>


#if defined(SFML_SYSTEM_WINDOWS)

#include <SFML/Window/Win32/DisplayImplWin32.hpp>
using DisplayImplType = sf::priv::DisplayImplWin32;

#elif defined(SFML_SYSTEM_LINUX) || defined(SFML_SYSTEM_FREEBSD) || defined(SFML_SYSTEM_OPENBSD) || \
    defined(SFML_SYSTEM_NETBSD)

#if defined(SFML_USE_DRM)

#include <SFML/Window/DRM/DisplayImplDRM.hpp>
using DisplayImplType = sf::priv::DisplayImplDRM;

#else

#include <SFML/Window/Unix/DisplayImplX11.hpp>
using DisplayImplType = sf::priv::DisplayImplX11;

#endif

#elif defined(SFML_SYSTEM_MACOS)

#include <SFML/Window/macOS/DisplayImplCocoa.hpp>
using DisplayImplType = sf::priv::DisplayImplCocoa;

#elif defined(SFML_SYSTEM_IOS)

#include <SFML/Window/iOS/DisplayImplUIKit.hpp>
using DisplayImplType = sf::priv::DisplayImplUIKit;

#elif defined(SFML_SYSTEM_ANDROID)

#include <SFML/Window/Android/DisplayImplAndroid.hpp>
using DisplayImplType = sf::priv::DisplayImplAndroid;

#endif


namespace sf
{


////////////////////////////////////////////////////////////
Display::Display(std::unique_ptr<priv::DisplayImpl>&& impl) : m_impl(std::move(impl))
{
}

////////////////////////////////////////////////////////////
// Must be in this compilation unit, not inferred from header
Display::~Display() = default;

////////////////////////////////////////////////////////////
Display Display::getPrimaryDisplay()
{
    // Call OS-specific implementation creator for primary display
    return DisplayImplType::createPrimaryDisplay();
}


////////////////////////////////////////////////////////////
VideoMode Display::getDesktopMode()
{
    // Directly forward to the OS-specific implementation
    return static_cast<DisplayImplType*>(m_impl.get())->getDesktopMode();
}


////////////////////////////////////////////////////////////
bool Display::isValid(const VideoMode& mode)
{
    const std::vector<VideoMode>& modes = getFullscreenModes();

    return std::find(modes.begin(), modes.end(), mode) != modes.end();
}


////////////////////////////////////////////////////////////
const std::vector<VideoMode>& Display::getFullscreenModes()
{
    static const auto modes = [&]
    {
        std::vector<VideoMode> result = static_cast<DisplayImplType*>(m_impl.get())->getFullscreenModes();
        std::sort(result.begin(), result.end(), std::greater<>());
        return result;
    }();

    return modes;
}

} // namespace sf
