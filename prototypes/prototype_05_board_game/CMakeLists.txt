add_executable(prototype_05_board_game
    main.cpp
)

target_link_libraries(prototype_05_board_game
    PRIVATE
        engine
)

# Unlike every previous prototype, this one loads no textures and no
# external files -- the board, tiles, and UI are drawn entirely from
# engine rendering primitives and text (DrawRectangle/DrawText), so
# there is no asset directory to bake in at compile time.
