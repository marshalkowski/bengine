#include "engine/Engine.hpp"

#include <string>

int main() {
    constexpr engine::Key demoKey = engine::Key::Up;

    engine::Engine app({.width = 800, .height = 450, .title = "06 - Input States"});

    int pressCount = 0;
    std::string lastEventText = "Last event: none yet";

    while (!app.ShouldClose()) {
        const bool isDown = app.IsKeyDown(demoKey);

        if (app.IsKeyPressed(demoKey)) {
            ++pressCount;
            lastEventText = "Last event: pressed";
        } else if (app.IsKeyReleased(demoKey)) {
            lastEventText = "Last event: released";
        }

        app.BeginFrame();
        app.Clear(engine::colors::White);

        app.DrawText("Hold, press, and release the Up arrow key", 20, 20, 20, engine::colors::DarkGray);
        app.DrawText(std::string("Held: ") + (isDown ? "yes" : "no"), 20, 60, 20, engine::colors::DarkGray);
        app.DrawText(lastEventText, 20, 90, 20, engine::colors::DarkGray);
        app.DrawText("Press count: " + std::to_string(pressCount), 20, 120, 20, engine::colors::DarkGray);

        app.EndFrame();
    }

    return 0;
}
