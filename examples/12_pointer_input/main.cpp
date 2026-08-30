#include "engine/Engine.hpp"

#include <string>

// Demonstrates the engine's pointer-input primitives and how they compose
// into point-and-click interaction:
//
//     mouse position (Engine::MouseX/MouseY)
//         +
//     engine::Contains(rect, mouseX, mouseY)   -- is the pointer over the box?
//         +
//     Engine::IsMouseButtonPressed(Left)        -- was the button just clicked?
//         =
//     hover + click, entirely composed here in example code.
//
// The engine has no notion of "hover" or "clicked" itself -- those are
// just this example's interpretation of the three primitives above,
// same as 06_input_states composes IsKeyDown/IsKeyPressed/IsKeyReleased
// into "held"/"press count" without the engine knowing what a "held key"
// means to the game using it.

int main() {
    constexpr int windowWidth = 800;
    constexpr int windowHeight = 450;

    constexpr engine::Rect box{300.0f, 150.0f, 200.0f, 150.0f};
    constexpr engine::Color normalColor{210, 210, 210, 255};
    constexpr engine::Color hoveredColor{160, 200, 235, 255};
    constexpr engine::Color clickedColor{130, 205, 140, 255};

    engine::Engine app({.width = windowWidth, .height = windowHeight, .title = "12 - Pointer Input"});

    bool clicked = false;
    int clickCount = 0;

    while (!app.ShouldClose()) {
        const float mouseX = app.MouseX();
        const float mouseY = app.MouseY();
        const bool hovered = engine::Contains(box, mouseX, mouseY);

        if (hovered && app.IsMouseButtonPressed(engine::MouseButton::Left)) {
            clicked = !clicked;
            ++clickCount;
        }

        app.BeginFrame();
        app.Clear(engine::colors::White);

        const engine::Color boxColor = clicked ? clickedColor : hovered ? hoveredColor : normalColor;
        app.DrawRectangle(box.x, box.y, box.width, box.height, boxColor);

        app.DrawText("Hover and click the box", 20, 20, 20, engine::colors::DarkGray);
        app.DrawText("Mouse: (" + std::to_string(static_cast<int>(mouseX)) + ", " +
                          std::to_string(static_cast<int>(mouseY)) + ")",
                      20, 60, 20, engine::colors::DarkGray);
        app.DrawText(std::string("State: ") + (clicked ? "Clicked" : hovered ? "Hovered" : "Normal"), 20, 90, 20,
                      engine::colors::DarkGray);
        app.DrawText("Clicks: " + std::to_string(clickCount), 20, 120, 20, engine::colors::DarkGray);

        app.EndFrame();
    }

    return 0;
}
