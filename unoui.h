#ifndef UNOUI_H
#define UNOUI_H

#include "gameengine.h"
#include "colour_render.h"
#include <string>

class UnoUI {
private:
    GameEngine gameEngine;
    RenderCard cardRenderer;
    int selectedCardIndex;
    bool gameRunning;
    bool needsRedraw;
    std::string message;

    void clearScreen();
    void setCursorPosition(int x, int y);
    void drawText(int x, int y, const std::string& text);
    void drawCenteredText(int y, const std::string& text);
    int getConsoleWidth();
    int getConsoleHeight();
    void drawTopPanel();
    void drawMiddlePanel();
    void drawPlayerHand();
    void drawBottomPanel();
    void drawCharacterInfo();
    void drawSkillStatus();
    CardColor chooseColor();
    void handleLuckyStarSkill();
    void handleCollectorSkill();
    void handleThiefSkill();
    void showWinner();
    void drawGameState();
    void drawPlayerStatus();
    void renderSimpleCard(int x, int y, const Card& card, bool selected = false);
    void drawPlayableIndicator();
    int calculateOptimalCardsPerRow();

public:
    UnoUI();
    void drawGameScreen();
    void handleInput();
    bool isGameRunning() const;
    bool isCPUTurn() const { return gameEngine.getCurrentPlayerIndex() != 0; }
};

#endif