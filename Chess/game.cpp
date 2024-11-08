#include "game.h"
#include "tools.h"
#include <fstream>

void Game::initalBoard(std::list<Tool*>& toolsList, std::vector<std::vector<Tool*>>& matrix) {
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
}

inline int Game::validCord(int x, int y) {
    return (x >= 0) and (x <= 7) and (y >= 0) and (y <= 7);
}

int Game::checkBlocking(Tool* toolToMove, int targetX, int targetY, std::vector<std::vector<Tool*>> matrix) {
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
        else if(abs(targetY - y) == abs(targetX - x)){
            for (int i = 1; i < abs(targetY - y); i++) {
                if (matrix[x + i * ((targetX - x) / abs(targetX - x))][y + i * ((targetY - y) / abs(targetY - y))] != nullptr) {
                    return -1;
                }
            }
        }
        else {
            return -1;
        }
    }
    if (matrix[targetX][targetY] != nullptr) {
        if (matrix[targetX][targetY]->color == toolToMove->color) {
            return -2; //might be castle
        }
    }
    return 1;
}

moveType Game::checkValidityOfMove(Tool* toolToMove, int targetX, int targetY, std::vector<std::vector<Tool*>> matrix, int* enPessant) { //returns 1 for ok move, and -1 for failure.
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

bool Game::moveWithRollBack(Tool* toolToMove,bool rollBack,Tool* myKing,int targetX, int targetY) {
    int x = toolToMove->x;
    int y = toolToMove->y;
    //std::ofstream outFile("coordinates2.txt", std::ios::app);
    //outFile << x << " " << y << "->" << targetX << " " << targetY << "\n";
    int howMuchKing = 0;
    int howMuchRook = 0;
    bool hasToolMoved = toolToMove->moved;
    bool checkReturnVal = false;
    Tool* tempTool = nullptr;
    moveType returnValue = checkValidityOfMove(toolToMove, targetX, targetY, matrix, enPessant);
    switch (returnValue)
    {
    case Bad:
        return false;
    case Regular:
        if (matrix[targetX][targetY] != nullptr) {
            tempTool = matrix[targetX][targetY];
            toolsList.remove(matrix[targetX][targetY]);
        }
        matrix[x][y] = nullptr;
        toolToMove->move(targetX, targetY);
        matrix[targetX][targetY] = toolToMove;
        if ((checkReturnVal = checkCheck(myKing, matrix)) || rollBack) {
            matrix[x][y] = toolToMove;
            toolToMove->move(x, y);
            toolToMove->moved = hasToolMoved;
            matrix[targetX][targetY] = tempTool;
            if (tempTool)
                toolsList.push_back(tempTool);
            enPessant[0] = enPessant[2];
            enPessant[1] = enPessant[3];
            if(checkReturnVal) return false;
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
            return false;
        }
        toolToMove->move(x + 2 * howMuchKing, y);
        matrix[x + howMuchKing][y] = nullptr;
        matrix[x + 2 * howMuchKing][y] = toolToMove;
        if ((checkReturnVal = checkCheck(myKing, matrix)) || rollBack) {
            matrix[x][y] = toolToMove;
            matrix[x + 2 * howMuchKing][y] = nullptr;
            toolToMove->move(x, y);
            toolToMove->moved = hasToolMoved;
            enPessant[0] = enPessant[2];
            enPessant[1] = enPessant[3];
            if (checkReturnVal) return false;
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
        if ((checkReturnVal = checkCheck(myKing, matrix)) || rollBack) {
            matrix[x][y] = toolToMove;
            toolToMove->move(x, y);
            toolToMove->moved = hasToolMoved;
            matrix[targetX][targetY] = nullptr;
            toolsList.push_back(tempTool);
            matrix[targetX][targetY + (1 - 2 * toolToMove->color)] = tempTool;
            enPessant[0] = enPessant[2];
            enPessant[1] = enPessant[3];
            if (checkReturnVal) return false;
        }
        break;
    case Promotion:
        break;
    }
    return true;
}

void Game::move(Tool* toolToMove, std::vector<std::vector<Tool*>> &matrix,std::list<Tool*> &toolsList) {
    if (toolToMove->color != turn)
        return;
    Tool* myKing = nullptr;
    Tool* enemyKing = nullptr;
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
    if (!moveWithRollBack(toolToMove,false,myKing,targetX,targetY)) return;
    if (checkCheck(enemyKing, matrix)) {
        whoIsInCheck = 1 - toolToMove->color;
        enemyKing->border.setOutlineColor(sf::Color::Red);
    }
    else {
        whoIsInCheck = -1;
    }
    myKing->border.setOutlineColor(sf::Color::Transparent);
    turn = !turn;
    startX = -1; startY = -1; targetX = -1; targetY = -1;
}

bool Game::MateOrStale(Tool* king) {
    for (auto tool : toolsList) {
        if (tool->color == turn) {
            for(int targetX = 0;targetX < 8;targetX++){
                for (int targetY = 0; targetY < 8; targetY++) {
                    if (moveWithRollBack(tool, true, king, targetX, targetY)) {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

bool Game::checkCheck(Tool* king, std::vector<std::vector<Tool*>>& matrix) {
    if (king->tooltype != King)
        return false;
    int x = king->x, y = king->y;
    x++;
    while (Game::validCord(x, y)) {
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
    while (Game::validCord(x, y)) {
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
    while (Game::validCord(x, y)) {
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
    while (Game::validCord(x, y)) {
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
    while (Game::validCord(x, y)) {
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
    while (Game::validCord(x, y)) {
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
    while (Game::validCord(x, y)) {
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
    while (Game::validCord(x, y)) {
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
    if (Game::validCord(x, y))
        if (matrix[x][y] != nullptr)
            if (
                matrix[x][y]->color != king->color and
                matrix[x][y]->tooltype == Knight
                )
                return true;
    x = king->x + 2; y = king->y - 1;
    if (Game::validCord(x, y))
        if (matrix[x][y] != nullptr)
            if (
                matrix[x][y]->color != king->color and
                matrix[x][y]->tooltype == Knight
                )
                return true;
    x = king->x - 2; y = king->y + 1;
    if (Game::validCord(x, y))
        if (matrix[x][y] != nullptr)
            if (
                matrix[x][y]->color != king->color and
                matrix[x][y]->tooltype == Knight
                )
                return true;
    x = king->x - 2; y = king->y - 1;
    if (Game::validCord(x, y))
        if (matrix[x][y] != nullptr)
            if (
                matrix[x][y]->color != king->color and
                matrix[x][y]->tooltype == Knight
                )
                return true;
    x = king->x + 1; y = king->y + 2;
    if (Game::validCord(x, y))
        if (matrix[x][y] != nullptr)
            if (
                matrix[x][y]->color != king->color and
                matrix[x][y]->tooltype == Knight
                )
                return true;
    x = king->x + 1; y = king->y - 2;
    if (Game::validCord(x, y))
        if (matrix[x][y] != nullptr)
            if (
                matrix[x][y]->color != king->color and
                matrix[x][y]->tooltype == Knight
                )
                return true;
    x = king->x - 1; y = king->y + 2;
    if (Game::validCord(x, y))
        if (matrix[x][y] != nullptr)
            if (
                matrix[x][y]->color != king->color and
                matrix[x][y]->tooltype == Knight
                )
                return true;
    x = king->x - 1; y = king->y - 2;
    if (Game::validCord(x, y))
        if (matrix[x][y] != nullptr)
            if (
                matrix[x][y]->color != king->color and
                matrix[x][y]->tooltype == Knight
                )
                return true;
    x = king->x + 1; y = king->y - 1 + 2 * king->color;
    if (Game::validCord(x, y))
        if (matrix[x][y] != nullptr)
            if (
                matrix[x][y]->color != king->color and
                matrix[x][y]->tooltype == Pawn
                )
                return true;
    x = king->x - 1; y = king->y - 1 + 2 * king->color;
    if (Game::validCord(x, y))
        if (matrix[x][y] != nullptr)
            if (
                matrix[x][y]->color != king->color and
                matrix[x][y]->tooltype == Pawn
                )
                return true;
    return false;
}

bool Game::KingStatus(std::vector<std::vector<Tool*>>& matrix, std::list<Tool*>& toolsList, sf::Text& txt) {
    Tool* king = nullptr;
    for (auto i : toolsList) {
        if (i->color == turn and i->tooltype == King) {
            king = i;
        }
    }
    if (MateOrStale(king)) {
        std::string string;
        if (!turn)
            string = "Black Won";
        else
            string = "White Won";
        txt.setString(string);
        return 1;
    }
    return 0;
}

Game::Game() {
    std::vector<std::vector<Tool*>> matrix(8, std::vector<Tool*>(8, nullptr)); //the board. put in notice that it is transformed, meaning matrix[x = row][y = column] is the object in the column y and row x.
    this->startX = -1, this->startY = -1, this->targetX = -1, this->targetY = -1;
    this->turn = 0;
    this->whoIsInCheck = -1; //1 for black king in chck,0 for white.
    initalBoard(this->toolsList, matrix);
    this->matrix = matrix;

}

Game::~Game() = default;