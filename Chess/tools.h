#ifndef TOOLS_H
#define TOOLS_H

#include <SFML/Graphics.hpp>
#include <stdexcept>
#include <vector>
#include <list>

enum ToolType
{
    Rook,
    Knight,
    Bishoup,
    Queen,
    King,
    Pawn
};

class Tool {
public:
    int x, y;
    bool color; //1 for black, 0 for white
    sf::CircleShape shape;
    sf::CircleShape border;
    sf::Text type;
    sf::Font font;
    bool fontLoaded;
    ToolType tooltype;
    bool moved;

    Tool(int x, int y, bool color, ToolType tooltype);
    ~Tool();
    void drawTool(sf::RenderWindow& window);
    void move(int x, int y);
};

#endif // TOOLS_H
