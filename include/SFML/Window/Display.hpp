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

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/Export.hpp>

#include <SFML/System/Vector2.hpp>

#include <memory>
#include <vector>


namespace sf
{
class VideoMode;

namespace priv
{
class DisplayImpl;
}

////////////////////////////////////////////////////////////
/// \brief Display represents a display made available by the OS
///
////////////////////////////////////////////////////////////
class SFML_WINDOW_API Display
{
    ////////////////////////////////////////////////////////////
    /// \brief Construct display with a DisplayImpl pointer
    ///
    ////////////////////////////////////////////////////////////
    Display(std::unique_ptr<priv::DisplayImpl>&& impl);

public:
    ~Display();

    ////////////////////////////////////////////////////////////
    /// \brief Get the primary display
    ///
    /// \return Primary display
    ///
    ////////////////////////////////////////////////////////////
    static Display getPrimaryDisplay();

    ////////////////////////////////////////////////////////////
    /// \brief Retrieve all the fullscreen video modes this display supports
    ///
    /// When creating a fullscreen window, the video mode is restricted
    /// to be compatible with what the graphics driver and display
    /// support. This function returns the complete list of all video
    /// modes that can be used in fullscreen mode.
    /// The returned array is sorted from best to worst, so that
    /// the first element will always give the best mode (higher
    /// width, height and bits-per-pixel).
    ///
    /// \return Array containing all  the fullscreen video modes this display supports
    ///
    ////////////////////////////////////////////////////////////
    const std::vector<VideoMode>& getFullscreenModes();

    ////////////////////////////////////////////////////////////
    /// \brief Get the current desktop video mode of this display
    ///
    /// \return Current desktop video mode of this display
    ///
    ////////////////////////////////////////////////////////////
    VideoMode getDesktopMode();

    ////////////////////////////////////////////////////////////
    /// \brief Tell whether or not the video mode is valid on this display
    ///
    /// The validity of video modes is only relevant when using
    /// fullscreen windows; otherwise any video mode can be used
    /// with no restriction.
    ///
    /// \return True if the video mode is valid for fullscreen mode on this display
    ///
    ////////////////////////////////////////////////////////////
    bool isValid(const VideoMode& mode);

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    const std::unique_ptr<priv::DisplayImpl> m_impl; //!< Platform-specific implementation of the display
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::Display
/// \ingroup window
///
/// A display is an abstraction to represent a display.
///
/// A display object of the primary can be obtained using
/// the static function getPrimaryDisplay().
///
/// sf::Display provides a function for retrieving the list of
/// all the video modes supported by that display and the system:
/// getFullscreenModes().
///
/// A custom video mode can also be checked directly for
/// fullscreen compatibility with its isValid(sf::VideoMode) function.
///
/// Additionally, sf::Display provides a function to get the mode
/// currently used by the desktop for that display: getDesktopMode().
/// This allows to build windows with the same size or pixel
/// depth as the current resolution.
///
/// Usage example:
/// \code
/// // Display the list of all the video modes available for fullscreen
/// std::vector<sf::VideoMode> modes = sf::Display::getPrimaryDisplay().getFullscreenModes();
/// for (std::size_t i = 0; i < modes.size(); ++i)
/// {
///     sf::VideoMode mode = modes[i];
///     std::cout << "Mode #" << i << ": "
///               << mode.width << "x" << mode.height << " - "
///               << mode.bitsPerPixel << " bpp" << std::endl;
/// }
///
/// // Create a window with the same pixel depth as the desktop
/// sf::VideoMode desktop = sf::Display::getPrimaryDisplay().getDesktopMode();
/// window.create(sf::VideoMode({1024, 768}, desktop.bitsPerPixel), "SFML window");
/// \endcode
///
////////////////////////////////////////////////////////////

///
/// Usage example:
/// \code
/// // Display the list of all the video modes available for fullscreen for the primary display
/// sf::Display display = sf::Display::getPrimaryDisplay();
///
/// std::vector<sf::VideoMode> modes = display.getFullscreenModes();
/// for (std::size_t i = 0; i < modes.size(); ++i)
/// {
///     sf::VideoMode mode = modes[i];
///     std::cout << "Mode #" << i << ": "
///               << mode.width << "x" << mode.height << " - "
///               << mode.bitsPerPixel << " bpp" << std::endl;
/// }
///
/// // Create a window with the same pixel depth as the desktop
/// sf::VideoMode desktopMode = display.getDesktopMode();
/// window.create(sf::VideoMode(desktopMode, "SFML window");
/// \endcode
///
////////////////////////////////////////////////////////////
