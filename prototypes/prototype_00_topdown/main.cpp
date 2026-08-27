#include "engine/Engine.hpp"

#include <array>
#include <string>

// Everything in this file is game-specific: the grid, walls, goal, and
// traversability rules are rules of this particular prototype, not
// demonstrated engine capabilities. The engine only supplies window
// lifecycle, input, resources, and rendering.

namespace {

constexpr int cellSize = 64;
constexpr int gridCols = 10;
constexpr int gridRows = 7;
constexpr int hudHeight = 32;
constexpr int windowWidth = gridCols * cellSize;
constexpr int windowHeight = hudHeight + gridRows * cellSize;

enum class Cell {
    Floor,
    Wall,
    Goal,
};

// Grid cells are flat colors, drawn as rectangles through the engine's
// primitive API — no throwaway per-cell texture assets needed for this.
constexpr engine::Color floorColor{230, 220, 200, 255};
constexpr engine::Color wallColor{90, 74, 58, 255};
constexpr engine::Color goalColor{255, 196, 0, 255};

// '#' = wall, '.' = floor, 'G' = goal. Border is walled in on all sides.
constexpr std::array<const char*, gridRows> roomLayout = {
    "##########",
    "#........#",
    "#..##....#",
    "#..##....#",
    "#........#",
    "#.......G#",
    "##########",
};

Cell CellAt(int col, int row) {
    const char symbol = roomLayout[row][col];
    if (symbol == '#') {
        return Cell::Wall;
    }
    if (symbol == 'G') {
        return Cell::Goal;
    }
    return Cell::Floor;
}

bool IsTraversable(int col, int row) {
    return CellAt(col, row) != Cell::Wall;
}

// One grid step per key press, via the engine's IsKeyPressed.
struct MovementKey {
    engine::Key key;
    int deltaCol;
    int deltaRow;
};

} // namespace

int main() {
    engine::Engine app({.width = windowWidth, .height = windowHeight, .title = "Prototype 00 - Topdown"});

    const std::string assetDir = PROTOTYPE_TOPDOWN_ASSET_DIR;
    const engine::TextureHandle playerTexture = app.LoadTexture((assetDir + "/player.png").c_str());

    int playerCol = 1;
    int playerRow = 1;
    bool goalReached = false;

    constexpr std::array<MovementKey, 8> movementKeys = {{
        {engine::Key::Up, 0, -1},
        {engine::Key::W, 0, -1},
        {engine::Key::Down, 0, 1},
        {engine::Key::S, 0, 1},
        {engine::Key::Left, -1, 0},
        {engine::Key::A, -1, 0},
        {engine::Key::Right, 1, 0},
        {engine::Key::D, 1, 0},
    }};

    while (!app.ShouldClose()) {
        for (const MovementKey& moveKey : movementKeys) {
            if (!app.IsKeyPressed(moveKey.key) || goalReached) {
                continue;
            }

            const int targetCol = playerCol + moveKey.deltaCol;
            const int targetRow = playerRow + moveKey.deltaRow;
            if (IsTraversable(targetCol, targetRow)) {
                playerCol = targetCol;
                playerRow = targetRow;
                if (CellAt(playerCol, playerRow) == Cell::Goal) {
                    goalReached = true;
                }
            }
        }

        app.BeginFrame();
        app.Clear(engine::colors::White);

        for (int row = 0; row < gridRows; ++row) {
            for (int col = 0; col < gridCols; ++col) {
                const int x = col * cellSize;
                const int y = hudHeight + row * cellSize;
                switch (CellAt(col, row)) {
                    case Cell::Wall:
                        app.DrawRectangle(x, y, cellSize, cellSize, wallColor);
                        break;
                    case Cell::Goal:
                        app.DrawRectangle(x, y, cellSize, cellSize, goalColor);
                        break;
                    case Cell::Floor:
                        app.DrawRectangle(x, y, cellSize, cellSize, floorColor);
                        break;
                }
            }
        }

        app.DrawSprite(playerTexture, playerCol * cellSize, hudHeight + playerRow * cellSize);

        app.DrawText(goalReached ? "You reached the goal!" : "Arrow keys/WASD: reach the gold tile",
                     10, 6, 20, engine::colors::DarkGray);

        app.EndFrame();
    }

    return 0;
}
