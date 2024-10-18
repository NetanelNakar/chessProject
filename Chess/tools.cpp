#include "tools.h"
#include <SFML/Graphics.hpp>

#define TOOL_SIZE 60
#define BLOCK_SIZE 100
#define  BORDER_LEN 2

Tool::Tool(int x, int y, bool color, ToolType tooltype) : x(x), y(y), color(color), tooltype(tooltype) {
	this->moved = false;
	if ((x < 0 || x > 7) || (y < 0 || y > 7)){ //check validity of cordinates 
		throw std::out_of_range("Coordinates are out of range.");
		}

	this->shape.setRadius(TOOL_SIZE / 2);
	this->shape.setPosition(x * BLOCK_SIZE + (BLOCK_SIZE - TOOL_SIZE) / 2, 
							y * BLOCK_SIZE + (BLOCK_SIZE - TOOL_SIZE) / 2);
	if (color)
		this->shape.setFillColor(sf::Color::Black);
	else
		this->shape.setFillColor(sf::Color::White);

	this->border.setRadius(TOOL_SIZE / 2 + BORDER_LEN);
	this->border.setPosition(x * BLOCK_SIZE + (BLOCK_SIZE - TOOL_SIZE - 2*BORDER_LEN) / 2, 
							 y * BLOCK_SIZE + (BLOCK_SIZE - TOOL_SIZE - 2*BORDER_LEN) / 2);
	if (!color)
		this->border.setFillColor(sf::Color::Black);
	else
		this->border.setFillColor(sf::Color::White);
	this->border.setOutlineThickness(2);
	this->border.setOutlineColor(sf::Color::Transparent);

	if (this->font.loadFromFile("arial.ttf")) {
		this->fontLoaded = true;
		type.setFont(font);
		switch (tooltype)
		{
		case Pawn:
			type.setString("P");
			break;
		case Knight:
			type.setString("N");
			break;
		case Bishoup:
			type.setString("B");
			break;
		case Rook:
			type.setString("R");
			break;
		case Queen:
			type.setString("Q");
			break;
		case King:
			type.setString("K");
			break;
		default:
			break;
		}
		type.setCharacterSize(TOOL_SIZE / 2);
		if (!color)
			type.setFillColor(sf::Color::Black);
		else
			type.setFillColor(sf::Color::White);
		type.setPosition(x * BLOCK_SIZE + (BLOCK_SIZE - TOOL_SIZE),
						 y * BLOCK_SIZE + (BLOCK_SIZE - TOOL_SIZE) * 2 / 3);
	}
	else {
		this->fontLoaded = false;
	}
}


Tool::~Tool() = default;

void Tool::drawTool(sf::RenderWindow& window) {
	window.draw(this->border);
	window.draw(this->shape);
	if (this->fontLoaded)
		window.draw(this->type);
}

void Tool::move(int x, int y) {
	if ((x < 0 || x > 7) || (y < 0 || y > 7)) { //check validity of cordinates 
		throw std::out_of_range("Coordinates are out of range.");
	}

	this->x = x;
	this->y = y;
	this->moved = true;
	this->shape.setPosition(x * BLOCK_SIZE + (BLOCK_SIZE - TOOL_SIZE) / 2,
		y * BLOCK_SIZE + (BLOCK_SIZE - TOOL_SIZE) / 2);

	this->border.setPosition(x * BLOCK_SIZE + (BLOCK_SIZE - TOOL_SIZE - 2 * BORDER_LEN) / 2,
		y * BLOCK_SIZE + (BLOCK_SIZE - TOOL_SIZE - 2 * BORDER_LEN) / 2);

	type.setPosition(x * BLOCK_SIZE + (BLOCK_SIZE - TOOL_SIZE),
		y * BLOCK_SIZE + (BLOCK_SIZE - TOOL_SIZE) * 2 / 3);
}