#include <SFML/Graphics.hpp>
#include "tools.h"
#include "game.h"
#include "cstdint"

#define BLOCK_SIZE 100

int WinMain()
{
    sf::RenderWindow window(sf::VideoMode(800, 800), "CH3SS");
    sf::View view(sf::FloatRect(0, 0, 800, 800));
    sf::RectangleShape shape;
    shape.setSize(sf::Vector2f(BLOCK_SIZE, BLOCK_SIZE));
    sf::Text txt;
    txt.setPosition(20, 300);
    txt.setScale(5.f, 5.f);
    txt.setFillColor(sf::Color::Red);
    sf::Font font;
    font.loadFromFile("arial.ttf");
    txt.setFont(font);
    bool vic = 0;
    Game game;
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        if (event.type == sf::Event::Resized) {
            // Calculate the new aspect ratio
            float windowRatio = static_cast<float>(event.size.width) / static_cast<float>(event.size.height);
            float viewRatio = 800.0f / 800.0f;

            if (windowRatio > viewRatio) {
                // Window is wider than the view
                float newWidth = 800.0f * windowRatio;
                view.setSize(newWidth, 800.0f);
            }
            else {
                // Window is taller than the view
                float newHeight = 800.0f / windowRatio;
                view.setSize(800.0f, newHeight);
            }

            view.setViewport(sf::FloatRect(0, 0, 1, 1));
            window.setView(view);
        }

        if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2i pixelPos = sf::Mouse::getPosition(window);

                // Convert the pixel position to view coordinates
                sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos);

                // Now `worldPos` contains the mouse position relative to the view
                game.startX = static_cast<int>(worldPos.x) / 100;
                game.startY = static_cast<int>(worldPos.y) / 100;
            }
        }
        if (event.type == sf::Event::MouseButtonReleased) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2i pixelPos = sf::Mouse::getPosition(window);

                // Convert the pixel position to view coordinates
                sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos);

                // Now `worldPos` contains the mouse position relative to the view
                if (game.startX != -1 and game.startY != -1) {
                    game.targetX = static_cast<int>(worldPos.x) / 100;
                    game.targetY = static_cast<int>(worldPos.y) / 100;
                }
                if (!game.validCord(game.startX, game.startY) or !game.validCord(game.targetX, game.targetY)) {
                    continue;
                }
                else if (game.matrix[game.startX][game.startY] != nullptr) {
                    game.move(game.matrix[game.startX][game.startY], game.matrix, game.toolsList);
                }
                if (game.whoIsInCheck != -1) {
                    vic = game.KingStatus(game.matrix, game.toolsList, txt) ? 1 : vic;
                }
            }
        }

        window.clear();
        for (size_t i = 0; i < 8; i++)
        {
            for (size_t j = 0; j < 8; j++)
            {
                shape.setPosition(sf::Vector2f(i * BLOCK_SIZE, j * BLOCK_SIZE));
                if ((i + j) % 2 == 0) {
                    shape.setFillColor(sf::Color::White);
                }
                else {
                    shape.setFillColor(sf::Color::Black);
                }
                window.draw(shape);
            }
        }
        for (auto i : game.toolsList) {
            i->drawTool(window);
        }

        if (vic)
            window.draw(txt);

        window.display();
    }

    return 0;
}