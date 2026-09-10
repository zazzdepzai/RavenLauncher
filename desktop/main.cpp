// Desktop preview of the ravenxd mini HUD (same facet layout/colors as
// ios/MiniHUDView.mm), built with SFML so it runs on Windows and Linux
// without Xcode. This is a standalone visual/interaction demo, not the
// real launcher -- it simulates an update cycle rather than hitting a
// real manifest URL.
//
// Controls:
//   F      - toggle Forge / Vanilla badge
//   Space  - simulate an update-check run (status text + progress bar)
//   Esc    - quit

#include <SFML/Graphics.hpp>
#include <atomic>
#include <mutex>
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <cmath>

struct FacetColors {
    static sf::Color at(int i) {
        static const sf::Color palette[5] = {
            sf::Color(133, 183, 235), // #85B7EB
            sf::Color(55, 138, 221),  // #378ADD
            sf::Color(24, 95, 165),   // #185FA5
            sf::Color(12, 68, 124),   // #0C447C
            sf::Color(4, 44, 83),     // #042C53
        };
        return palette[i % 5];
    }
};

// Same facet point sets as MiniHUDView.mm, in "unit" space.
static const std::vector<std::vector<sf::Vector2f>> kFacets = {
    {{0,30},{25,0},{55,10},{45,45},{10,55}},
    {{25,0},{55,10},{65,35},{45,45}},
    {{10,55},{45,45},{50,75},{20,80}},
    {{45,45},{65,35},{70,65},{50,75}},
    {{0,30},{10,55},{20,80},{-10,75},{-15,45}},
};

sf::ConvexShape MakeFacet(const std::vector<sf::Vector2f>& pts, float ox, float oy, float k, sf::Color color) {
    sf::ConvexShape shape;
    shape.setPointCount(pts.size());
    for (size_t i = 0; i < pts.size(); i++) {
        shape.setPoint(i, sf::Vector2f(ox + pts[i].x * k, oy + pts[i].y * k));
    }
    shape.setFillColor(color);
    return shape;
}

sf::Font LoadFont() {
    sf::Font font;
    const char* candidates[] = {
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
    };
    for (const char* path : candidates) {
        if (font.loadFromFile(path)) return font;
    }
    return font; // empty font still lets the app run; text just won't render
}

// Background "network" simulation: cycles through a few status messages
// while advancing progress from 0 to 1, so you can see the HUD respond
// without needing real update servers.
struct UpdateSim {
    std::atomic<bool> running{false};
    std::atomic<float> progress{0.f};
    std::mutex textMutex;
    std::string statusText = "ready";

    void start() {
        if (running.exchange(true)) return;
        progress = 0.f;
        std::thread([this]() {
            const std::vector<std::string> steps = {
                "checking launcher update...",
                "launcher up to date",
                "checking mod updates...",
                "updating optifine...",
                "updating inventory tweaks...",
                "mods up to date"
            };
            for (size_t i = 0; i < steps.size(); i++) {
                {
                    std::lock_guard<std::mutex> lock(textMutex);
                    statusText = steps[i];
                }
                progress = static_cast<float>(i + 1) / steps.size();
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
            running = false;
        }).detach();
    }

    std::string text() {
        std::lock_guard<std::mutex> lock(textMutex);
        return statusText;
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(300, 100), "ravenxd mini HUD (desktop preview)",
                             sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);

    sf::Font font = LoadFont();
    bool isForge = false;
    UpdateSim sim;

    sf::RectangleShape panel(sf::Vector2f(260.f, 68.f));
    panel.setPosition(20.f, 16.f);
    panel.setFillColor(sf::Color(12, 42, 74, 235));
    panel.setOutlineThickness(1.f);
    panel.setOutlineColor(sf::Color(55, 138, 221, 90));

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) window.close();
                if (event.key.code == sf::Keyboard::F) isForge = !isForge;
                if (event.key.code == sf::Keyboard::Space) sim.start();
            }
        }

        window.clear(sf::Color(30, 30, 32));
        window.draw(panel);

        // Badge facets (unit space 70x80 scaled down to fit the ~44px badge box).
        float badgeSize = 44.f;
        float scale = badgeSize / 70.f;
        float bx = 32.f, by = 28.f;
        for (size_t i = 0; i < kFacets.size(); i++) {
            sf::ConvexShape facet = MakeFacet(kFacets[i], bx, by, scale, FacetColors::at((int)i));
            window.draw(facet);
        }

        // Monogram chip + "X" glyph on top of the facets.
        sf::RectangleShape chip(sf::Vector2f(28.f * scale * 1.6f, 22.f * scale * 1.6f));
        chip.setPosition(bx + 6.f * scale, by + 22.f * scale);
        chip.setFillColor(sf::Color(12, 42, 74));
        window.draw(chip);

        if (font.getInfo().family != "") {
            sf::Text glyph("X", font, 14);
            glyph.setFillColor(FacetColors::at(0));
            glyph.setPosition(bx + 12.f * scale, by + 24.f * scale);
            window.draw(glyph);

            sf::Text wordmark("ravenxd", font, 15);
            wordmark.setStyle(sf::Text::Bold);
            wordmark.setFillColor(FacetColors::at(0));
            wordmark.setPosition(96.f, 22.f);
            window.draw(wordmark);

            std::string status = sim.text();
            sf::Text statusText(status, font, 11);
            statusText.setFillColor(sf::Color(235, 235, 235));
            statusText.setPosition(96.f, 44.f);
            window.draw(statusText);
        }

        // Loader pill (Forge / Vanilla).
        sf::RectangleShape pill(sf::Vector2f(54.f, 20.f));
        pill.setPosition(214.f, 22.f);
        pill.setFillColor(isForge ? FacetColors::at(1) : FacetColors::at(3));
        window.draw(pill);
        if (font.getInfo().family != "") {
            sf::Text pillText(isForge ? "forge" : "vanilla", font, 10);
            pillText.setFillColor(isForge ? FacetColors::at(4) : FacetColors::at(0));
            pillText.setPosition(221.f, 27.f);
            window.draw(pillText);
        }

        // Progress bar.
        sf::RectangleShape track(sf::Vector2f(176.f, 4.f));
        track.setPosition(96.f, 64.f);
        track.setFillColor(sf::Color(255, 255, 255, 30));
        window.draw(track);

        sf::RectangleShape fill(sf::Vector2f(176.f * sim.progress.load(), 4.f));
        fill.setPosition(96.f, 64.f);
        fill.setFillColor(FacetColors::at(0));
        window.draw(fill);

        window.display();
    }

    return 0;
}
