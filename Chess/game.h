#ifndef GAME_H
#define GAME_H


#include <string>
#include <algorithm>
#include <vector>
#include <list>
#include "tools.h"

enum moveType {
    Bad,
    Regular,
    Castle,
    EnPessant,
    Promotion
};

class Game
{
public:
    std::list<std::pair<int, int>> xy;
    std::vector<std::vector<Tool*>> matrix; 
    std::list<Tool*> toolsList;
    int enPessant[4] = { -2, -2, -2, -2 };
    int startX, startY, targetX , targetY;
    bool turn ;
    int whoIsInCheck;

	Game();
	~Game();
    inline static int validCord(int x, int y);
    void move(Tool* toolToMove, std::vector<std::vector<Tool*>> &matrix, std::list<Tool*> &toolsList);
    bool KingStatus(std::vector<std::vector<Tool*>>& matrix, std::list<Tool*>& toolsList, sf::Text& txt);
private:
    void initalBoard(std::list<Tool*>& toolsList, std::vector<std::vector<Tool*>>& matrix);
    bool checkCheck(Tool* king, std::vector<std::vector<Tool*>>& matrix);
    int checkBlocking(Tool* toolToMove, int targetX, int targetY, std::vector<std::vector<Tool*>> matrix);
    moveType checkValidityOfMove(Tool* toolToMove, int targetX, int targetY, std::vector<std::vector<Tool*>> matrix, int* enPessant);
    bool MateOrStale(Tool* king);
    bool moveWithRollBack(Tool* toolToMove, bool rollBack, Tool* myKing, int targetX, int targetY);
};

#endif // TOOLS_H