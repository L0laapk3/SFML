#include <SFML/Window/Monitor.hpp>
#include <SFML/Window/VideoMode.hpp>

#include <catch2/catch_test_macros.hpp>

#include <WindowUtil.hpp>
#include <algorithm>
#include <type_traits>

TEST_CASE("[Window] sf::VideoMode", runDisplayTests())
{
    SECTION("Type traits")
    {
        STATIC_CHECK(std::is_copy_constructible_v<sf::VideoMode>);
        STATIC_CHECK(std::is_copy_assignable_v<sf::VideoMode>);
        STATIC_CHECK(std::is_nothrow_move_constructible_v<sf::VideoMode>);
        STATIC_CHECK(std::is_nothrow_move_assignable_v<sf::VideoMode>);
    }

    SECTION("Construction")
    {
        SECTION("Default constructor")
        {
            const sf::VideoMode videoMode;
            CHECK(videoMode.size == sf::Vector2u(0, 0));
            CHECK(videoMode.bitsPerPixel == 0);
        }

        SECTION("Width, height constructor")
        {
            const sf::VideoMode videoMode({800, 600}, 60);
            CHECK(videoMode.size == sf::Vector2u(800, 600));
            CHECK(videoMode.bitsPerPixel == 32);
        }

        SECTION("Width, height, bit depth constructor")
        {
            const sf::VideoMode videoMode({800, 600}, 60, 24);
            CHECK(videoMode.size == sf::Vector2u(800, 600));
            CHECK(videoMode.bitsPerPixel == 24);
        }
    }

    SECTION("getFullscreenModes()")
    {
        const auto& modes = sf::Monitor::getPrimaryMonitor().getFullscreenModes();
        CHECK(std::is_sorted(modes.begin(), modes.end(), std::greater<>()));
    }

    SECTION("isValid()")
    {
        const sf::VideoMode videoMode;
        CHECK(!sf::Monitor::getPrimaryMonitor().isValidMode(videoMode));
    }

    SECTION("Operators")
    {
        SECTION("operator==")
        {
            CHECK(sf::VideoMode() == sf::VideoMode());
            CHECK(sf::VideoMode({0, 0}, 0) == sf::VideoMode({0, 0}, 0));
            CHECK(sf::VideoMode({1080, 1920}, 64) == sf::VideoMode({1080, 1920}, 64));
        }

        SECTION("operator!=")
        {
            CHECK(sf::VideoMode() != sf::VideoMode({1, 0}, 60));
            CHECK(sf::VideoMode() != sf::VideoMode({0, 1}, 60));
            CHECK(sf::VideoMode() != sf::VideoMode({0, 0}, 60, 1));
            CHECK(sf::VideoMode({720, 720}, 60) != sf::VideoMode({720, 720}, 60, 24));
            CHECK(sf::VideoMode({1080, 1920}, 60, 16) != sf::VideoMode({400, 600}, 60));
        }

        SECTION("operator<")
        {
            CHECK(sf::VideoMode() < sf::VideoMode({0, 0}, 0, 1));
            CHECK(sf::VideoMode({800, 800}, 60, 24) < sf::VideoMode({1080, 1920}, 60, 48));
            CHECK(sf::VideoMode({400, 600}, 60, 48) < sf::VideoMode({600, 400}, 60, 48));
            CHECK(sf::VideoMode({400, 400}, 60, 48) < sf::VideoMode({400, 600}, 60, 48));
        }

        SECTION("operator>")
        {
            CHECK(sf::VideoMode({1, 0}, 60, 1) > sf::VideoMode({0, 0}, 60, 1));
            CHECK(sf::VideoMode({800, 800}, 60, 48) > sf::VideoMode({1080, 1920}, 60, 24));
            CHECK(sf::VideoMode({600, 400}, 60, 48) > sf::VideoMode({400, 600}, 60, 48));
            CHECK(sf::VideoMode({400, 600}, 60, 48) > sf::VideoMode({400, 400}, 60, 48));
        }

        SECTION("operator<=")
        {
            CHECK(sf::VideoMode() <= sf::VideoMode({0, 0}, 60, 1));
            CHECK(sf::VideoMode({800, 800}, 60, 24) <= sf::VideoMode({1080, 1920}, 60, 48));
            CHECK(sf::VideoMode({400, 600}, 60, 48) <= sf::VideoMode({600, 400}, 60, 48));
            CHECK(sf::VideoMode({400, 400}, 60, 48) <= sf::VideoMode({400, 600}, 60, 48));
            CHECK(sf::VideoMode() <= sf::VideoMode());
            CHECK(sf::VideoMode({0, 0}, 0, 0) <= sf::VideoMode({0, 0}, 0, 0));
            CHECK(sf::VideoMode({1080, 1920}, 60, 64) <= sf::VideoMode({1080, 1920}, 60, 64));
        }

        SECTION("operator>=")
        {
            CHECK(sf::VideoMode({1, 0}, 60) >= sf::VideoMode({0, 0}, 60, 1));
            CHECK(sf::VideoMode({800, 800}, 60, 48) >= sf::VideoMode({1080, 1920}, 60, 24));
            CHECK(sf::VideoMode({600, 400}, 60, 48) >= sf::VideoMode({400, 600}, 60, 48));
            CHECK(sf::VideoMode({400, 600}, 60, 48) >= sf::VideoMode({400, 400}, 60, 48));
            CHECK(sf::VideoMode() >= sf::VideoMode());
            CHECK(sf::VideoMode({0, 0}, 60, 0) >= sf::VideoMode({0, 0}, 60, 0));
            CHECK(sf::VideoMode({1080, 1920}, 60, 64) >= sf::VideoMode({1080, 1920}, 60, 64));
        }
    }
}
