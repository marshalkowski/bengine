#include "engine/Engine.hpp"

#include <algorithm>
#include <array>
#include <optional>
#include <random>
#include <string>
#include <vector>

// A small Catan-inspired resource/building loop: this prototype is a
// deliberate cross-genre pressure test, not another action game. Its
// purpose is to see which engine capabilities transfer to a
// point-and-click, turn-less, discrete-action game and which don't. Two
// small, evidence-driven additions were made to the engine to support
// this: engine::MouseButton/Engine::MouseX/MouseY/IsMouseButtonPressed
// (pointer input didn't exist at all) and engine::Contains (point-in-rect
// hit-testing -- engine::Intersects' positive-area-overlap rule can't
// express "is this zero-area point inside this rect"). Everything else
// here -- the board, dice, resources, building rule, and the Button
// helper below -- is ordinary game code. See
// docs/prototype_05_board_game_review.md for what this exercise revealed.

namespace {

constexpr int windowWidth = 920;
constexpr int windowHeight = 520;

// ---------------------------------------------------------------------
// Resources and the board.
// ---------------------------------------------------------------------

enum class ResourceType { Wood, Stone, Food };
constexpr int resourceTypeCount = 3;

const char* ResourceName(ResourceType type) {
    switch (type) {
        case ResourceType::Wood: return "Wood";
        case ResourceType::Stone: return "Stone";
        case ResourceType::Food: return "Food";
    }
    return "?";
}

engine::Color ResourceColor(ResourceType type) {
    switch (type) {
        case ResourceType::Wood: return engine::Color{120, 90, 50, 255};
        case ResourceType::Stone: return engine::Color{140, 140, 150, 255};
        case ResourceType::Food: return engine::Color{195, 170, 60, 255};
    }
    return engine::colors::DarkGray;
}

// Lightened toward white -- an unbuilt space still shows its resource
// type, just visually muted compared to a built one.
engine::Color UnbuiltTint(engine::Color color) {
    const auto lighten = [](unsigned char channel) {
        return static_cast<unsigned char>(channel + (255 - channel) * 0.65f);
    };
    return engine::Color{lighten(color.r), lighten(color.g), lighten(color.b), 255};
}

constexpr float tileWidth = 150.0f;
constexpr float tileHeight = 100.0f;

// What a producing board space is at setup time: position, resource, and
// the two-dice total that activates it. Deliberately not loaded from
// external data -- see the architecture review for why hardcoding this
// small, fixed layout was the right call for this experiment.
struct BoardSpaceSetup {
    float x, y;
    ResourceType resource;
    int productionNumber;
};

// spaces[0] and spaces[1] are always pre-built (see MakeFreshBoardGameState)
// so both resources the one building type costs are always producible from
// turn one, regardless of which spaces the player later chooses to build.
const std::vector<BoardSpaceSetup> boardLayout = {
    {30.0f, 60.0f, ResourceType::Wood, 6},
    {190.0f, 60.0f, ResourceType::Stone, 8},
    {350.0f, 60.0f, ResourceType::Food, 5},
    {510.0f, 60.0f, ResourceType::Wood, 9},
    {30.0f, 170.0f, ResourceType::Stone, 4},
    {190.0f, 170.0f, ResourceType::Food, 10},
    {350.0f, 170.0f, ResourceType::Wood, 3},
    {510.0f, 170.0f, ResourceType::Stone, 11},
};

constexpr int buildCostWood = 1;
constexpr int buildCostStone = 1;
constexpr int spacesToWin = 5;

// Runtime board space: setup data plus the one mutable fact ("built or
// not") the game actually needs. A plain struct in a plain vector -- see
// the architecture review's ECS counter-test for whether that held up.
struct BoardSpace {
    engine::Rect bounds;
    ResourceType resource;
    int productionNumber;
    bool built = false;
};

// ---------------------------------------------------------------------
// Dice. Ordinary game code on top of <random> -- no engine RNG exists or
// was added; see the architecture review's randomness section for why.
// ---------------------------------------------------------------------

int RollDie() {
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<int> distribution(1, 6);
    return distribution(rng);
}

// ---------------------------------------------------------------------
// Board game runtime state: everything one playthrough needs, grouped
// the same way CombatState/HazardState were -- see MakeFreshBoardGameState.
// ---------------------------------------------------------------------

struct BoardGameState {
    std::array<int, resourceTypeCount> resources{};
    std::vector<BoardSpace> spaces;
    int selectedSpace = -1;
    int lastDieA = 0;
    int lastDieB = 0;
    bool hasRolled = false;
    std::vector<int> lastProducedSpaces;
    bool gameWon = false;
};

BoardGameState MakeFreshBoardGameState() {
    BoardGameState state;
    for (const BoardSpaceSetup& setup : boardLayout) {
        BoardSpace space;
        space.bounds = engine::Rect{setup.x, setup.y, tileWidth, tileHeight};
        space.resource = setup.resource;
        space.productionNumber = setup.productionNumber;
        state.spaces.push_back(space);
    }
    // Bootstrap: one pre-built Wood space and one pre-built Stone space
    // (the only two resources the building costs), zero starting
    // resources -- the player must roll at least once before their
    // first build, which exercises the full roll -> produce -> build
    // loop immediately rather than skipping the first step.
    state.spaces[0].built = true;
    state.spaces[1].built = true;
    return state;
}

// ---------------------------------------------------------------------
// Button: a tiny game-side helper, not an engine abstraction. Bundles a
// clickable engine::Rect with a label; hover/click are both just
// engine::Contains(bounds, mouseX, mouseY) plus (for click) an edge-
// triggered mouse button check -- the same primitive board-space
// selection uses below. See the architecture review's UI-pressure
// section for whether this deserved to be more than this.
// ---------------------------------------------------------------------

struct Button {
    engine::Rect bounds;
    std::string label;
};

bool IsHovered(const Button& button, float mouseX, float mouseY) {
    return engine::Contains(button.bounds, mouseX, mouseY);
}

bool IsClicked(const Button& button, engine::Engine& app) {
    return IsHovered(button, app.MouseX(), app.MouseY()) &&
           app.IsMouseButtonPressed(engine::MouseButton::Left);
}

constexpr engine::Color buttonColor{210, 210, 215, 255};
constexpr engine::Color buttonHoverColor{180, 205, 235, 255};
constexpr engine::Color buttonDisabledColor{228, 228, 228, 255};

void DrawButton(engine::Engine& app, const Button& button, bool enabled) {
    const bool hovered = enabled && IsHovered(button, app.MouseX(), app.MouseY());
    const engine::Color background = !enabled ? buttonDisabledColor : hovered ? buttonHoverColor : buttonColor;
    app.DrawRectangle(button.bounds.x, button.bounds.y, button.bounds.width, button.bounds.height, background);
    app.DrawText(button.label, static_cast<int>(button.bounds.x) + 14, static_cast<int>(button.bounds.y) + 9, 18,
                  engine::colors::DarkGray);
}

// Fixed layout, chosen once by hand -- no text measurement was needed
// because every label here is short and every button is a fixed size.
const Button rollButton{engine::Rect{700.0f, 330.0f, 190.0f, 36.0f}, "Roll Dice"};
const Button buildButton{engine::Rect{700.0f, 378.0f, 190.0f, 36.0f}, "Build"};
const Button newGameButton{engine::Rect{700.0f, 460.0f, 190.0f, 32.0f}, "New Game"};
const Button startButton{engine::Rect{380.0f, 280.0f, 160.0f, 40.0f}, "Start Game"};
const Button playAgainButton{engine::Rect{380.0f, 280.0f, 160.0f, 40.0f}, "Play Again"};

constexpr float panelX = 700.0f;

bool CanBuild(const BoardGameState& state) {
    return state.selectedSpace >= 0 && !state.spaces[state.selectedSpace].built &&
           state.resources[static_cast<int>(ResourceType::Wood)] >= buildCostWood &&
           state.resources[static_cast<int>(ResourceType::Stone)] >= buildCostStone;
}

// All discrete player actions for one frame: select a space, roll, or
// build. Nothing here runs unless the player clicks something -- there
// is no continuous per-frame simulation in this scene at all. See the
// architecture review's "discrete vs. continuous" section.
void UpdateBoardGame(BoardGameState& state, engine::Engine& app) {
    const float mouseX = app.MouseX();
    const float mouseY = app.MouseY();
    const bool clicked = app.IsMouseButtonPressed(engine::MouseButton::Left);

    if (clicked) {
        for (std::size_t i = 0; i < state.spaces.size(); ++i) {
            if (engine::Contains(state.spaces[i].bounds, mouseX, mouseY)) {
                state.selectedSpace = static_cast<int>(i);
                break;
            }
        }
    }

    if (IsClicked(rollButton, app)) {
        state.lastDieA = RollDie();
        state.lastDieB = RollDie();
        state.hasRolled = true;
        const int total = state.lastDieA + state.lastDieB;

        state.lastProducedSpaces.clear();
        for (std::size_t i = 0; i < state.spaces.size(); ++i) {
            BoardSpace& space = state.spaces[i];
            if (space.built && space.productionNumber == total) {
                ++state.resources[static_cast<int>(space.resource)];
                state.lastProducedSpaces.push_back(static_cast<int>(i));
            }
        }
    }

    if (CanBuild(state) && IsClicked(buildButton, app)) {
        state.resources[static_cast<int>(ResourceType::Wood)] -= buildCostWood;
        state.resources[static_cast<int>(ResourceType::Stone)] -= buildCostStone;
        state.spaces[state.selectedSpace].built = true;

        const int builtCount = static_cast<int>(
            std::count_if(state.spaces.begin(), state.spaces.end(), [](const BoardSpace& s) { return s.built; }));
        if (builtCount >= spacesToWin) {
            state.gameWon = true;
        }
    }
}

void DrawBoardGame(engine::Engine& app, const BoardGameState& state) {
    const float mouseX = app.MouseX();
    const float mouseY = app.MouseY();

    for (std::size_t i = 0; i < state.spaces.size(); ++i) {
        const BoardSpace& space = state.spaces[i];
        const bool hovered = engine::Contains(space.bounds, mouseX, mouseY);
        const bool selected = state.selectedSpace == static_cast<int>(i);
        const bool produced = std::find(state.lastProducedSpaces.begin(), state.lastProducedSpaces.end(),
                                          static_cast<int>(i)) != state.lastProducedSpaces.end();

        const engine::Color borderColor = selected  ? engine::Color{230, 180, 60, 255}
                                           : produced ? engine::Color{90, 200, 110, 255}
                                           : hovered  ? engine::Color{120, 170, 220, 255}
                                                      : engine::Color{205, 205, 205, 255};
        app.DrawRectangle(space.bounds.x - 4.0f, space.bounds.y - 4.0f, space.bounds.width + 8.0f,
                           space.bounds.height + 8.0f, borderColor);

        const engine::Color fill =
            space.built ? ResourceColor(space.resource) : UnbuiltTint(ResourceColor(space.resource));
        app.DrawRectangle(space.bounds.x, space.bounds.y, space.bounds.width, space.bounds.height, fill);

        const int textX = static_cast<int>(space.bounds.x) + 8;
        app.DrawText(ResourceName(space.resource), textX, static_cast<int>(space.bounds.y) + 8, 16,
                      engine::colors::DarkGray);
        app.DrawText(std::to_string(space.productionNumber), textX, static_cast<int>(space.bounds.y) + 32, 22,
                      engine::colors::DarkGray);
        app.DrawText(space.built ? "Built" : "Empty", textX, static_cast<int>(space.bounds.y) + 64, 14,
                      engine::colors::DarkGray);
    }

    int y = 20;
    app.DrawText("Resources", static_cast<int>(panelX), y, 18, engine::colors::DarkGray);
    y += 26;
    app.DrawText("Wood:  " + std::to_string(state.resources[static_cast<int>(ResourceType::Wood)]),
                  static_cast<int>(panelX), y, 16, engine::colors::DarkGray);
    y += 22;
    app.DrawText("Stone: " + std::to_string(state.resources[static_cast<int>(ResourceType::Stone)]),
                  static_cast<int>(panelX), y, 16, engine::colors::DarkGray);
    y += 22;
    app.DrawText("Food:  " + std::to_string(state.resources[static_cast<int>(ResourceType::Food)]),
                  static_cast<int>(panelX), y, 16, engine::colors::DarkGray);
    y += 32;

    if (state.hasRolled) {
        app.DrawText("Dice: " + std::to_string(state.lastDieA) + " + " + std::to_string(state.lastDieB) + " = " +
                          std::to_string(state.lastDieA + state.lastDieB),
                      static_cast<int>(panelX), y, 16, engine::colors::DarkGray);
    } else {
        app.DrawText("Roll to begin", static_cast<int>(panelX), y, 16, engine::colors::DarkGray);
    }
    y += 30;

    const int builtCount = static_cast<int>(
        std::count_if(state.spaces.begin(), state.spaces.end(), [](const BoardSpace& s) { return s.built; }));
    app.DrawText("Built: " + std::to_string(builtCount) + " / " + std::to_string(spacesToWin) + " to win",
                  static_cast<int>(panelX), y, 16, engine::colors::DarkGray);
    y += 30;

    if (state.selectedSpace >= 0) {
        const BoardSpace& selected = state.spaces[state.selectedSpace];
        app.DrawText(std::string("Selected: ") + ResourceName(selected.resource) + " (" +
                          std::to_string(selected.productionNumber) + ")",
                      static_cast<int>(panelX), y, 16, engine::colors::DarkGray);
        y += 22;
        if (selected.built) {
            app.DrawText("Already built", static_cast<int>(panelX), y, 16, engine::colors::DarkGray);
        } else {
            app.DrawText("Cost: 1 Wood, 1 Stone", static_cast<int>(panelX), y, 16, engine::colors::DarkGray);
            y += 20;
            app.DrawText(CanBuild(state) ? "Affordable" : "Need more resources", static_cast<int>(panelX), y, 16,
                          engine::colors::DarkGray);
        }
    } else {
        app.DrawText("No space selected", static_cast<int>(panelX), y, 16, engine::colors::DarkGray);
    }

    DrawButton(app, rollButton, true);
    DrawButton(app, buildButton, CanBuild(state));
    DrawButton(app, newGameButton, true);
}

void DrawTitle(engine::Engine& app) {
    app.DrawText("PROTOTYPE 05: BOARD GAME", 300, 160, 28, engine::colors::DarkGray);
    app.DrawText("Roll dice, gather resources, build on the board.", 260, 200, 16, engine::colors::DarkGray);
    DrawButton(app, startButton, true);
}

void DrawWin(engine::Engine& app) {
    app.DrawText("You built enough to win!", 320, 200, 24, engine::colors::DarkGray);
    DrawButton(app, playAgainButton, true);
}

// ---------------------------------------------------------------------
// Scene flow: same enum-tag + optional-session-container + two-switch
// shape as prototype_03_brawler and prototype_04_scene_data, now around
// a scene whose internal structure has nothing in common with either.
// New this time: "New Game" reconstructs BoardGameState fresh WITHOUT an
// AppState transition -- see the architecture review for why that's a
// meaningfully different operation from every prior prototype's fresh-
// state construction, which always coincided with entering a scene.
// ---------------------------------------------------------------------

enum class AppState { Title, BoardGame, Win };

} // namespace

int main() {
    engine::Engine app({.width = windowWidth, .height = windowHeight, .title = "Prototype 05 - Board Game"});

    AppState state = AppState::Title;
    std::optional<BoardGameState> boardGame;

    while (!app.ShouldClose()) {
        switch (state) {
        case AppState::Title:
            if (IsClicked(startButton, app)) {
                boardGame.emplace(MakeFreshBoardGameState());
                state = AppState::BoardGame;
            }
            break;
        case AppState::BoardGame:
            UpdateBoardGame(*boardGame, app);
            if (IsClicked(newGameButton, app)) {
                boardGame.emplace(MakeFreshBoardGameState());
            }
            if (boardGame->gameWon) {
                boardGame.reset();
                state = AppState::Win;
            }
            break;
        case AppState::Win:
            if (IsClicked(playAgainButton, app)) {
                boardGame.emplace(MakeFreshBoardGameState());
                state = AppState::BoardGame;
            }
            break;
        }

        app.BeginFrame();
        app.Clear(engine::colors::White);
        switch (state) {
        case AppState::Title:
            DrawTitle(app);
            break;
        case AppState::BoardGame:
            DrawBoardGame(app, *boardGame);
            break;
        case AppState::Win:
            DrawWin(app);
            break;
        }
        app.EndFrame();
    }

    return 0;
}
