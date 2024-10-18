#include <SFML/Graphics.hpp>
#include "tools.h"
#include "cstdint"
#include <string>
#include <algorithm>

#define BLOCK_SIZE 100

enum moveType {
    Bad,
    Regular,
    Castle,
    EnPessant,
    Promotion
};

bool checkCheck(Tool* king, std::vector<std::vector<Tool*>>& matrix);

inline int validCord(int x, int y) {
    return (x >= 0) and (x <= 7) and (y >= 0) and (y <= 7);
}

int checkBlocking(Tool* toolToMove, int targetX, int targetY, std::vector<std::vector<Tool*>> matrix) {
    int x = toolToMove->x;
    int y = toolToMove->y;
    if (toolToMove->tooltype != Knight) {
        if (x == targetX) {
            for (int i = 1; i < abs(targetY - y); i++) {
                if (matrix[x][y + i * ((targetY - y) / abs(targetY - y))] != nullptr) {
                    return -1;
                }
            }
            if (toolToMove->tooltype == Pawn && matrix[targetX][targetY] != nullptr) {
                return -1;
            }
        }
        else if (y == targetY) {
            for (int i = 1; i < abs(targetX - x); i++) {
                if (matrix[x + i * ((targetX - x) / abs(targetX - x))][y] != nullptr) {
                    return -1;
                }
            }
        }
        else { // has to be a diagnol move, and there for abs(targetY - y) = abs(targetX - x)
            for (int i = 1; i < abs(targetY - y); i++) {
                if (matrix[x + i * ((targetX - x) / abs(targetX - x))][y + i * ((targetY - y) / abs(targetY - y))] != nullptr) {
                    return -1;
                }
            }
        }
    }
    if (matrix[targetX][targetY] != nullptr) {
        if (matrix[targetX][targetY]->color == toolToMove->color) {
            return -2; //might be castle
        }
    }
    return 1;
}

moveType checkValidityOfMove(Tool* toolToMove, int targetX, int targetY, std::vector<std::vector<Tool*>> matrix, int* enPessant) { //returns 1 for ok move, and -1 for failure.
    int x = toolToMove->x;
    int y = toolToMove->y;
    enPessant[2] = enPessant[0];
    enPessant[3] = enPessant[1];
    enPessant[0] = -2;
    enPessant[1] = -2;
    int returnVal = checkBlocking(toolToMove, targetX, targetY, matrix);
    switch (returnVal)
    {
    case -1:
        return Bad;
    case -2:
        if (toolToMove->tooltype == King && matrix[targetX][targetY]->tooltype == Rook) {
            if (!toolToMove->moved && !matrix[targetX][targetY]->moved) {
                return Castle; //special return for Castle
            }
        }
        return Bad;
    case 1:
        switch (toolToMove->tooltype)
        {
        case Pawn:
            if (targetX == x && targetY == (y + 1 - 2 * (1 - toolToMove->color))) {
                return Regular;
            }
            else if (!toolToMove->moved && targetX == x && targetY == (y - 2 + 4 * toolToMove->color)) {
                enPessant[0] = targetX;
                enPessant[1] = targetY + (1 - 2 * toolToMove->color);
                return Regular;
            }
            else if (abs(targetX - x) == 1 && (-1 + 2 * toolToMove->color) * (targetY - y) == 1) {
                if (matrix[targetX][targetY] != nullptr) {
                    return Regular;
                }
                else if (targetX == enPessant[2] && targetY == enPessant[3]) {
                    return EnPessant; //special return for enPessant
                }
            }
            break;
        case Knight:
            if ((abs(x - targetX) == 1 && abs(y - targetY) == 2) || (abs(x - targetX) == 2 && abs(y - targetY) == 1)) {
                return Regular;
            }
            break;
        case Bishoup:
            if (abs(x - targetX) == abs(y - targetY)) {
                return Regular;
            }
            break;
        case Rook:
            if (abs(x - targetX) == 0 || abs(y - targetY) == 0) {
                return Regular;
            }
            break;
        case Queen:
            if (abs(x - targetX) == 0 || abs(y - targetY) == 0) {
                return Regular;
            }
            else if (abs(x - targetX) == abs(y - targetY)) {
                return Regular;
            }
            break;
        case King:
            if (abs(x - targetX) <= 1 && abs(y - targetY) <= 1) {
                return Regular;
            }
            break;
        default:
            break;
        }
        enPessant[0] = enPessant[2];
        enPessant[1] = enPessant[3];
        return Bad;
    }
}

void move(Tool* toolToMove, int targetX, int targetY, std::vector<std::vector<Tool*>>& matrix, std::list<Tool*>& toolsList, int* enPessant, bool& turn,int& checkFlag) {
    if (toolToMove->color != turn)
        return;
    moveType returnValue = checkValidityOfMove(toolToMove, targetX, targetY, matrix, enPessant);
    int x = toolToMove->x;
    int y = toolToMove->y;
    int howMuchKing = 0;
    int howMuchRook = 0;
    Tool* tempTool = nullptr;
    Tool* myKing = nullptr;
    Tool* enemyKing = nullptr;
    bool hasToolMoved = toolToMove->moved;
    for (auto tool : toolsList) {
        if (tool->tooltype == King) {
            if (tool->color == toolToMove->color) {
                myKing = tool;
            }
            else {
                enemyKing = tool;
            }
        }
    }
    switch (returnValue)
    {
    case Bad:
        return;
    case Regular:
        if (matrix[targetX][targetY] != nullptr) {
            tempTool = matrix[targetX][targetY];
            toolsList.remove(matrix[targetX][targetY]);
        }
        matrix[x][y] = nullptr;
        toolToMove->move(targetX, targetY);
        matrix[targetX][targetY] = toolToMove;
        if (checkCheck(myKing, matrix)) {
            matrix[x][y] = toolToMove;
            toolToMove->move(x, y);
            toolToMove->moved = hasToolMoved;
            matrix[targetX][targetY] = tempTool;
            if (tempTool)
                toolsList.push_back(tempTool);
            enPessant[0] = enPessant[2];
            enPessant[1] = enPessant[3];
            return;
        }
        break;
    case Castle:
        if (targetX == 0) { //queen side castle
            howMuchKing = -1;
            howMuchRook = 3;
        }
        else { //king side castle
            howMuchKing = 1;
            howMuchRook = -2;
        }
        matrix[x][y] = nullptr;
        toolToMove->move(x + howMuchKing, y);
        matrix[x + howMuchKing][y] = toolToMove;
        if (checkCheck(myKing, matrix)) {
            matrix[x][y] = toolToMove;
            matrix[x + howMuchKing][y] = nullptr;
            toolToMove->move(x, y);
            toolToMove->moved = hasToolMoved;
            enPessant[0] = enPessant[2];
            enPessant[1] = enPessant[3];
            return;
        }
        toolToMove->move(x + 2 * howMuchKing, y);
        matrix[x + howMuchKing][y] = nullptr;
        matrix[x + 2 * howMuchKing][y] = toolToMove;
        if (checkCheck(myKing, matrix)) {
            matrix[x][y] = toolToMove;
            matrix[x + 2 * howMuchKing][y] = nullptr;
            toolToMove->move(x, y);
            toolToMove->moved = hasToolMoved;
            enPessant[0] = enPessant[2];
            enPessant[1] = enPessant[3];
            return;
        }
        matrix[targetX][targetY]->move(targetX + howMuchRook, targetY);
        matrix[targetX + howMuchRook][targetY] = matrix[targetX][targetY];
        matrix[targetX][targetY] = nullptr;
        break;
    case EnPessant:
        tempTool = matrix[targetX][targetY + (1 - 2 * toolToMove->color)];
        toolsList.remove(matrix[targetX][targetY + (1 - 2 * toolToMove->color)]);
        matrix[targetX][targetY + (1 - 2 * toolToMove->color)] = nullptr;
        matrix[x][y] = nullptr;
        toolToMove->move(targetX, targetY);
        matrix[targetX][targetY] = toolToMove;
        if (checkCheck(myKing, matrix)) {
            matrix[x][y] = toolToMove;
            toolToMove->move(x, y);
            toolToMove->moved = hasToolMoved;
            matrix[targetX][targetY] = nullptr;
            toolsList.push_back(tempTool);
            matrix[targetX][targetY + (1 - 2 * toolToMove->color)] = tempTool;
            enPessant[0] = enPessant[2];
            enPessant[1] = enPessant[3];
            return;
        }
        break;
    case Promotion:
        break;
    default:
        break;
    }
    if (checkCheck(enemyKing, matrix)) {
        checkFlag = 1 - toolToMove->color;
        enemyKing->border.setOutlineColor(sf::Color::Red);
    }
    else {
        checkFlag = -1;
    }
    myKing->border.setOutlineColor(sf::Color::Transparent);
    turn = !turn;
}

std::vector<std::pair<int, int>> getAllowedMoves(Tool* tool, const std::vector<std::vector<Tool*>>& matrix, const int* enPassant) {
    std::vector<std::pair<int, int>> allowedMoves;
    int directions[8][2] = { {0,1}, {0,-1}, {1,0}, {-1,0}, {1,1}, {1,-1}, {-1,1}, {-1,-1} };
    int knightMoves[8][2] = { {2,1}, {2,-1}, {-2,1}, {-2,-1}, {1,2}, {1,-2}, {-1,2}, {-1,-2} };

    for (int targetX = 0; targetX < 8; targetX++) {
        for (int targetY = 0; targetY < 8; targetY++) {
            int enPassantTemp[2] = { enPassant[0], enPassant[1] };
            moveType moveValidity = checkValidityOfMove(tool, targetX, targetY, matrix, enPassantTemp);

            if (moveValidity != Bad) {
                allowedMoves.push_back(std::make_pair(targetX, targetY));
            }
        }
    }

    return allowedMoves;
}

bool checkCheck(Tool* king, std::vector<std::vector<Tool*>>& matrix) {
    if (king->tooltype != King)
        return false;
    int x = king->x, y = king->y;
    x++;
    while (validCord(x, y)) {
        if (matrix[x][y] != nullptr) {
            if (
                matrix[x][y]->color != king->color and
                (matrix[x][y]->tooltype == Rook or matrix[x][y]->tooltype == Queen)
                )
                return true;
            break;
        }
        x++;
    }
    x = king->x;

    x--;
    while (validCord(x, y)) {
        if (matrix[x][y] != nullptr) {
            if (
                matrix[x][y]->color != king->color and
                (matrix[x][y]->tooltype == Rook or matrix[x][y]->tooltype == Queen)
                )
                return true;
            break;
        }
        x--;
    }
    x = king->x;

    y++;
    while (validCord(x, y)) {
        if (matrix[x][y] != nullptr) {
            if (
                matrix[x][y]->color != king->color and
                (matrix[x][y]->tooltype == Rook or matrix[x][y]->tooltype == Queen)
                )
                return true;
            break;
        }
        y++;
    }
    y = king->y;

    y--;
    while (validCord(x, y)) {
        if (matrix[x][y] != nullptr) {
            if (
                matrix[x][y]->color != king->color and
                (matrix[x][y]->tooltype == Rook or matrix[x][y]->tooltype == Queen)
                )
                return true;
            break;
        }
        y--;
    }
    y = king->y;

    x++; y++;
    while (validCord(x, y)) {
        if (matrix[x][y] != nullptr) {
            if (
                matrix[x][y]->color != king->color and
                (matrix[x][y]->tooltype == Bishoup or matrix[x][y]->tooltype == Queen)
                )
                return true;
            break;
        }
        x++; y++;
    }
    x = king->x; y = king->y;

    x++; y--;
    while (validCord(x, y)) {
        if (matrix[x][y] != nullptr) {
            if (
                matrix[x][y]->color != king->color and
                (matrix[x][y]->tooltype == Bishoup or matrix[x][y]->tooltype == Queen)
                )
                return true;
            break;
        }
        x++; y--;
    }
    x = king->x; y = king->y;

    x--; y++;
    while (validCord(x, y)) {
        if (matrix[x][y] != nullptr) {
            if (
                matrix[x][y]->color != king->color and
                (matrix[x][y]->tooltype == Bishoup or matrix[x][y]->tooltype == Queen)
                )
                return true;
            break;
        }
        x--; y++;
    }
    x = king->x; y = king->y;

    x--; y--;
    while (validCord(x, y)) {
        if (matrix[x][y] != nullptr) {
            if (
                matrix[x][y]->color != king->color and
                (matrix[x][y]->tooltype == Bishoup or matrix[x][y]->tooltype == Queen)
                )
                return true;
            break;
        }
        x--; y--;
    }

    x = king->x + 2; y = king->y + 1;
    if (validCord(x, y)) 
        if (matrix[x][y] != nullptr) 
            if (
                matrix[x][y]->color != king->color and
                matrix[x][y]->tooltype == Knight
                )
                return true;
    x = king->x + 2; y = king->y - 1;
    if (validCord(x, y))
        if (matrix[x][y] != nullptr)
            if (
                matrix[x][y]->color != king->color and
                matrix[x][y]->tooltype == Knight
                )
                return true;
    x = king->x - 2; y = king->y + 1;
    if (validCord(x, y))
        if (matrix[x][y] != nullptr)
            if (
                matrix[x][y]->color != king->color and
                matrix[x][y]->tooltype == Knight
                )
                return true;
    x = king->x - 2; y = king->y - 1;
    if (validCord(x, y))
        if (matrix[x][y] != nullptr)
            if (
                matrix[x][y]->color != king->color and
                matrix[x][y]->tooltype == Knight
                )
                return true;
    x = king->x + 1; y = king->y + 2;
    if (validCord(x, y))
        if (matrix[x][y] != nullptr)
            if (
                matrix[x][y]->color != king->color and
                matrix[x][y]->tooltype == Knight
                )
                return true;
    x = king->x + 1; y = king->y - 2;
    if (validCord(x, y))
        if (matrix[x][y] != nullptr)
            if (
                matrix[x][y]->color != king->color and
                matrix[x][y]->tooltype == Knight
                )
                return true;
    x = king->x - 1; y = king->y + 2;
    if (validCord(x, y))
        if (matrix[x][y] != nullptr)
            if (
                matrix[x][y]->color != king->color and
                matrix[x][y]->tooltype == Knight
                )
                return true;
    x = king->x - 1; y = king->y - 2;
    if (validCord(x, y))
        if (matrix[x][y] != nullptr)
            if (
                matrix[x][y]->color != king->color and
                matrix[x][y]->tooltype == Knight
                )
                return true;
    x = king->x + 1; y = king->y - 1 + 2 * king->color;
    if (validCord(x, y))
        if (matrix[x][y] != nullptr)
            if (
                matrix[x][y]->color != king->color and
                matrix[x][y]->tooltype == Pawn
                )
                return true;
    x = king->x - 1; y = king->y - 1 + 2 * king->color;
    if (validCord(x, y))
        if (matrix[x][y] != nullptr)
            if (
                matrix[x][y]->color != king->color and
                matrix[x][y]->tooltype == Pawn
                )
                return true;
    return false;
}

int KingStatus(std::vector<std::vector<Tool*>>& matrix, std::list<Tool*>& toolsList, bool& turn, sf::Text& txt) {
    for (auto i : toolsList) {
        if (i->color == turn and i->tooltype == King) {
            checkCheck(i, matrix);
            return 0;
        }
    }
    std::string string;
    if (!turn)
        string = "Black Won";
    else
        string = "White Won";
    txt.setString(string);
    return 1;
}

int WinMain()
{
    sf::RenderWindow window(sf::VideoMode(800, 800), "CH3SS");
    sf::View view(sf::FloatRect(0, 0, 800, 800));
    sf::RectangleShape shape;
    shape.setSize(sf::Vector2f(BLOCK_SIZE, BLOCK_SIZE));
    std::vector<std::vector<Tool*>> matrix(8, std::vector<Tool*>(8, nullptr)); //the board. put in notice that it is transformed, meaning matrix[x = row][y = column] is the object in the column y and row x.
    std::list<Tool*> toolsList; 
    int enPessant[4] = { -2,-2, -2, -2 };
    int startX=-1, startY=-1, endX=-1, endY=-1;
    bool turn = 0;
    int whoIsInCheck = -1; //1 for black king in chck,0 for white.
    sf::Text txt;
    txt.setPosition(20, 300);
    txt.setScale(5.f, 5.f);
    txt.setFillColor(sf::Color::Red);
    sf::Font font;
    font.loadFromFile("arial.ttf");
    txt.setFont(font);
    bool vic = 0;
    for (size_t i = 0; i < 4; i++) { //creating the pieces in the board
        switch (i) {
        case 0:
            for (size_t j = 0; j < 8; j++) {
                Tool* tool = (j <= 4)
                    ? new Tool(j, 0, 1, static_cast<ToolType>(j))
                    : new Tool(j, 0, 1, static_cast<ToolType>(7 - j));

                toolsList.push_back(tool);
                matrix[j][0] = tool;  
            }
            break;

        case 1:
            for (size_t j = 0; j < 8; j++) {
                Tool* tool = new Tool(j, 1, 1, Pawn);
                toolsList.push_back(tool);
                matrix[j][1] = tool;
            }
            break;

        case 2:
            for (size_t j = 0; j < 8; j++) {
                Tool* tool = new Tool(j, 6, 0, Pawn);
                toolsList.push_back(tool);
                matrix[j][6] = tool;  
            }
            break;

        case 3:
            for (size_t j = 0; j < 8; j++) {
                Tool* tool = (j <= 4)
                    ? new Tool(j, 7, 0, static_cast<ToolType>(j))
                    : new Tool(j, 7, 0, static_cast<ToolType>(7 - j));

                toolsList.push_back(tool);
                matrix[j][7] = tool;  
            }
            break;

        default:
            break;
        }
    }
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
                startX = static_cast<int>(worldPos.x) / 100;
                startY = static_cast<int>(worldPos.y) / 100;
            }
        }
        if (event.type == sf::Event::MouseButtonReleased) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2i pixelPos = sf::Mouse::getPosition(window);

                // Convert the pixel position to view coordinates
                sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos);

                // Now `worldPos` contains the mouse position relative to the view
                if (startX != -1 and startY != -1) {
                    endX = static_cast<int>(worldPos.x) / 100;
                    endY = static_cast<int>(worldPos.y) / 100;
                }
                if (!validCord(startX, startY) or !validCord(endX, endY)) {
                    continue;
                }
                else if (matrix[startX][startY] != nullptr) {
                    move(matrix[startX][startY], endX, endY, matrix,toolsList,enPessant, turn,whoIsInCheck);
                }
                vic = KingStatus(matrix, toolsList, turn, txt) ? 1 : vic;
                startX = -1; startY = -1; endX = -1; endY = -1;
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
        for (auto i : toolsList) {
            i->drawTool(window);
        }

        if (vic)
            window.draw(txt);

        window.display();
    }

    return 0;
}