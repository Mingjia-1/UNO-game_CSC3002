#include "unoui.h"
#include <iostream>
#include <conio.h>
#include <windows.h>
#include <algorithm>
#include <random>
#include <sstream>
#include <iomanip>
#include "helpers.h" 

UnoUI::UnoUI() : selectedCardIndex(0), gameRunning(true), needsRedraw(true) {
    // Initialize backend with players
    gameEngine.addPlayer("You");
    gameEngine.addPlayer("CPU 1");
    gameEngine.addPlayer("CPU 2"); 
    gameEngine.addPlayer("CPU 3");
    gameEngine.startGame();
    
    message = "Your turn! Use A/D to navigate, ENTER to play, W to draw, S for skill";
}

void UnoUI::clearScreen() {
    system("cls");
}

void UnoUI::setCursorPosition(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void UnoUI::drawText(int x, int y, const std::string& text) {
    setCursorPosition(x, y);
    std::cout << text;
}

void UnoUI::drawCenteredText(int y, const std::string& text) {
    int width = getConsoleWidth();
    int x = (width - text.length()) / 2;
    drawText(std::max(0, x), y, text);
}

int UnoUI::getConsoleWidth() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
}

int UnoUI::getConsoleHeight() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
}

void UnoUI::drawCharacterInfo() {
    int width = getConsoleWidth();
    
    drawCenteredText(3, "CHARACTERS");
    drawCenteredText(4, "===========");
    
    // Get characters from backend
    std::vector<std::string> characters = gameEngine.getPlayerCharacters();
    
    for (int i = 0; i < 4 && i < characters.size(); i++) {
        int x = 5 + (i * 30);
        setCursorPosition(x, 6);
        
        std::string charInfo = (i == 0 ? "You: " : "CPU " + std::to_string(i) + ": ");
        charInfo += characters[i];
        
        // Add skill status for human player
        if (i == 0 && gameEngine.canUseSkill(0)) {
            charInfo += " (Ready)";
        } else if (i == 0) {
            charInfo += " (Used)";
        }
        
        std::cout << charInfo;
    }
}

void UnoUI::drawSkillStatus() {
    if (gameEngine.getCurrentPlayerIndex() == 0) {
        std::vector<std::string> characters = gameEngine.getPlayerCharacters();
        std::string yourCharacter = characters[0];
        std::string skillInfo;
        
        if (yourCharacter == "Lucky Star") {
            skillInfo = "Lucky Star Skill: Draw 3 cards, pick 1 (Ready)";
        }
        else if (yourCharacter == "Collector") {
            skillInfo = "Collector Skill: Steal from discard pile (Ready)";
        }
        else if (yourCharacter == "Thief") {
            skillInfo = "Thief Skill: Swap cards with opponent (Ready)";
        }
        else if (yourCharacter == "Defender") {
            // Check if Defender is revealed
            bool isRevealed = false; // You'd need to add a method to check this
            if (isRevealed) {
                skillInfo = "Defender Skill: PASSIVE (Revealed - Used this round)";
            } else {
                skillInfo = "Defender Skill: PASSIVE (Hidden - Ready to defend)";
            }
        }
        
        if (gameEngine.canUseSkill(0) && yourCharacter != "Defender") {
            skillInfo += " - PRESS S TO USE";
        } else if (yourCharacter == "Defender") {
            skillInfo += " - Auto-activates when targeted";
        } else {
            skillInfo += " - UNAVAILABLE";
        }
        
        drawCenteredText(8, skillInfo);
    }
}

void UnoUI::drawTopPanel() {
    int width = getConsoleWidth();
    
    // 第1行：游戏标题
    drawCenteredText(1, "┌─────────────────────────────────────────────┐");
    drawCenteredText(2, "│                 UNO GAME                     │");
    drawCenteredText(3, "└─────────────────────────────────────────────┘");
    
    // 第4-5行：游戏状态
    drawText(5, 5, "Turn: " + gameEngine.getCurrentPlayerName());
    drawText(width - 20, 5, "Deck: " + std::to_string(gameEngine.getDrawPileSize()));
    
    // 第6行：方向指示器
    std::string direction = gameEngine.getGameDirection() ? "→" : "←";
    drawCenteredText(6, "Direction: " + direction);
}

void UnoUI::drawMiddlePanel() {
    int width = getConsoleWidth();
    
    drawCenteredText(9, "CURRENT CARD");
    drawCenteredText(10, "============");
    std::cout << std::endl;
    
    // Get current card from backend
    Card currentCard = gameEngine.getCurrentCard();
    setCursorPosition(width/2 - 15, 12);
    
    // 显示Flash效果状态
    if (gameEngine.isFlashEffectActive()) {
        std::cout << "FLASH EFFECT ACTIVE! Must play: " 
                  << getColorString(gameEngine.getFlashColor()) 
                  << " (Chain: " << gameEngine.getFlashChainCount() << ")";
    }
    // Show current play color if the card is wild
    else if (currentCard.getColorString() == "WILD" || 
        currentCard.getColorString() == "WILD_DRAW4" || 
        currentCard.getColorString() == "FLASH") {
        std::cout << "Current Card: " << currentCard.getColorString() 
                  << " (Color: " << getColorString(gameEngine.getCurrentPlayColor()) << ")";
    } else {
        std::cout << "Current Card: " << currentCard.getColorString() << " " << currentCard.getValueString();
    }
    std::cout << std::endl;
    
    // Render the card using your renderer
    setCursorPosition(width/2 - 15, 13);
    
    if (currentCard.getColorString() != "UNKNOWN") {
        cardRenderer.render_card(
            currentCard.getColorString(),
            currentCard.getCardType(),
            currentCard.getCardNumber()
        );
    } else {
        std::cout << "No card played yet";
    }
    
    std::cout << std::endl;
    drawCenteredText(48, message);
    std::cout << std::endl;
    
    drawCenteredText(50, "====================");
    std::cout << std::endl;
}

void UnoUI::drawPlayerHand() {
    int width = getConsoleWidth();
    int height = getConsoleHeight();
    
    // 第20行：手牌标题
    drawCenteredText(20, "YOUR HAND");
    drawCenteredText(21, "─────────");
    
    // 获取玩家手牌
    std::vector<Card> playerHand = gameEngine.getPlayerHand(0);
    
    if (playerHand.empty()) {
        drawCenteredText(23, "No cards in hand");
        return;
    }
    
    // 计算每行最多显示多少张牌
    int maxCardsPerRow = calculateOptimalCardsPerRow();
    int cardSpacing = 12;
    int startY = 23;
    
    // 显示卡牌（简化版）
    for (int i = 0; i < playerHand.size(); i++) {
        int row = i / maxCardsPerRow;
        int col = i % maxCardsPerRow;
        
        int cardX = 10 + col * cardSpacing;
        int cardY = startY + row * 4;
        
        // 绘制简化卡牌
        renderSimpleCard(cardX, cardY, playerHand[i], i == selectedCardIndex);
        
        // 显示卡牌编号
        setCursorPosition(cardX + 1, cardY + 4);
        std::cout << "[" << (i + 1) << "]";
    }
    
    // 显示导航提示
    if (playerHand.size() > maxCardsPerRow) {
        setCursorPosition(10, startY + 8);
        std::cout << "Use A/D to navigate through cards";
    }
}

void UnoUI::drawBottomPanel() {
    std::cout << std::endl;
    drawCenteredText(94, "CONTROLS: ");
    
    // Only show skill button if skill is available
    if (gameEngine.getCurrentPlayerIndex() == 0 && gameEngine.canUseSkill(0)) {
        drawCenteredText(95, "A=Left  D=Right  W=Draw  S=Skill ");
    } else {
        drawCenteredText(95, "A=Left  D=Right  W=Draw ");
    }
    
    drawCenteredText(96, "ENTER=Play Card  Q=Quit ");
    drawCenteredText(97, "================================");
}

void UnoUI::drawGameScreen() {
    if (needsRedraw) {
        clearScreen();
        
        drawTopPanel();
        drawMiddlePanel(); 
        drawPlayerHand();
        drawBottomPanel();
        
        setCursorPosition(0, 100);
        needsRedraw = false;
    }
}

CardColor UnoUI::chooseColor() {
    int width = getConsoleWidth();
    int choice = 0;
    bool colorChosen = false;
    std::vector<std::string> colors = {"RED", "BLUE", "GREEN", "YELLOW"};
    std::vector<std::string> colorCodes = {"\033[91m", "\033[94m", "\033[92m", "\033[93m"};
    const std::string RESET = "\033[0m";
    
    while (!colorChosen) {
        clearScreen();
        drawTopPanel();
        
        drawCenteredText(15, "CHOOSE A COLOR");
        drawCenteredText(16, "==============");
        std::cout << std::endl;
        
        for (int i = 0; i < 4; i++) {
            int x = width/2 - 20 + (i * 10);
            setCursorPosition(x, 18);
            if (i == choice) {
                std::cout << ">>> " << colorCodes[i] << colors[i] << RESET << " <<<";
            } else {
                std::cout << "    " << colorCodes[i] << colors[i] << RESET << "    ";
            }
        }
        
        std::cout << std::endl << std::endl;
        drawCenteredText(20, "Use A/D to choose, ENTER to confirm");
        
        char ch = _getch();
        switch (ch) {
            case 'a':
            case 'A':
                if (choice > 0) choice--;
                break;
            case 'd':
            case 'D':
                if (choice < 3) choice++;
                break;
            case 13: // Enter
                colorChosen = true;
                break;
        }
    }
    
    // Convert choice to CardColor
    switch (choice) {
        case 0: return CardColor::RED;
        case 1: return CardColor::BLUE;
        case 2: return CardColor::GREEN;
        case 3: return CardColor::YELLOW;
        default: return CardColor::RED;
    }
}

void UnoUI::handleLuckyStarSkill() {
    std::vector<Card> topCards = gameEngine.useLuckyStarSkill(0);
    if (topCards.empty()) {
        message = "Cannot use Lucky Star skill now!";
        needsRedraw = true;
        return;
    }
    
    int choice = 0;
    bool cardChosen = false;
    
    while (!cardChosen) {
        clearScreen();
        
        // Simple header
        drawCenteredText(2, "LUCKY STAR SKILL - CHOOSE ONE CARD");
        drawCenteredText(3, "==================================");
        std::cout << std::endl;
        
        drawCenteredText(5, "SELECT A CARD TO ADD TO YOUR HAND:");
        std::cout << std::endl;
        
        // Display the three cards with simple layout
        for (int i = 0; i < topCards.size(); i++) {
            Card card = topCards[i];
            int yPos = 7 + (i * 3);
            
            // DEBUG: Show actual card values to identify the "0" issue
            std::string debugInfo = " (Value: " + std::to_string(static_cast<int>(card.value)) + ")";
            
            // Card info
            std::string cardInfo = "Card " + std::to_string(i + 1) + ": " + 
                      card.getColorString() + " " + card.getValueString() + debugInfo;
            drawCenteredText(yPos, cardInfo);
            
            // Selection indicator
            if (i == choice) {
                drawCenteredText(yPos + 1, ">>> SELECTED <<<");
            } else {
                drawCenteredText(yPos + 1, "   [SELECT]   ");
            }
            
            // Simple card preview
            std::string cardPreview = "[" + card.getColorString() + " " + card.getValueString() + "]";
            drawCenteredText(yPos + 2, cardPreview);
        }
        
        std::cout << std::endl;
        drawCenteredText(18, "CONTROLS: A=Up, D=Down, ENTER=Select, Q=Cancel");
        
        char ch = _getch();
        switch (ch) {
            case 'a':
            case 'A':
                if (choice > 0) choice--;
                break;
            case 'd':
            case 'D':
                if (choice < topCards.size() - 1) choice++;
                break;
            case 13: // Enter
                {
                    // Actually add the selected card to hand
                    if (gameEngine.completeLuckyStarSkill(0, choice, topCards)) {
                        Card selectedCard = topCards[choice];
                        message = "Used Lucky Star! Added: " + selectedCard.getColorString() + " " + selectedCard.getValueString();
                    } else {
                        message = "Failed to use Lucky Star skill!";
                    }
                    cardChosen = true;
                }
                break;
            case 'q':
            case 'Q':
                message = "Lucky Star skill cancelled";
                // Need to return the cards to draw pile if cancelled
                for (const Card& card : topCards) {
                    // This would need to be implemented in GameEngine
                }
                cardChosen = true;
                break;
        }
    }
    needsRedraw = true;
}

void UnoUI::handleCollectorSkill() {
    std::vector<Card> discardPile = gameEngine.getDiscardPileForCollector();
    if (discardPile.empty()) {
        message = "Discard pile is empty!";
        needsRedraw = true;
        return;
    }
    
    int choice = 0;
    bool cardChosen = false;
    int width = getConsoleWidth();
    
    while (!cardChosen) {
        clearScreen();
        
        // Simple header
        drawCenteredText(2, "COLLECTOR SKILL - STEAL FROM DISCARD PILE");
        drawCenteredText(3, "=========================================");
        std::cout << std::endl;
        
        drawCenteredText(5, "SELECT A CARD TO STEAL:");
        std::cout << std::endl;
        
        // Show 3 cards maximum with simple layout
        int maxCardsToShow = std::min(3, static_cast<int>(discardPile.size()));
        int startIndex = std::max(0, choice - 1);
        if (startIndex + maxCardsToShow > discardPile.size()) {
            startIndex = discardPile.size() - maxCardsToShow;
        }
        
        // Display cards in a compact horizontal layout
        for (int i = startIndex; i < startIndex + maxCardsToShow && i < discardPile.size(); i++) {
            Card card = discardPile[i];
            int yPos = 7;
            
            // Card label
            std::string cardLabel = "Card " + std::to_string(i + 1) + ": " + 
                                   card.getColorString() + " " + card.getValueString();
            drawCenteredText(yPos, cardLabel);
            
            // Selection indicator
            if (i == choice) {
                drawCenteredText(yPos + 1, ">>> SELECTED <<<");
            } else {
                drawCenteredText(yPos + 1, "   [SELECT]   ");
            }
            
            // Simple card preview (just the text, no ASCII art)
            std::string cardPreview = "[" + card.getColorString() + " " + card.getValueString() + "]";
            drawCenteredText(yPos + 2, cardPreview);
            
            std::cout << std::endl;
        }
        
        // Navigation info
        std::cout << std::endl;
        if (startIndex > 0) {
            drawCenteredText(18, "<-- [A] Previous Cards");
        }
        if (startIndex + maxCardsToShow < discardPile.size()) {
            drawCenteredText(19, "[D] Next Cards -->");
        }
        
        drawCenteredText(21, "CONTROLS: A=Left, D=Right, ENTER=Select, Q=Cancel");
        
        char ch = _getch();
        switch (ch) {
            case 'a':
            case 'A':
                if (choice > 0) choice--;
                break;
            case 'd':
            case 'D':
                if (choice < discardPile.size() - 1) choice++;
                break;
            case 13: // Enter
                {
                    Card selectedCard = gameEngine.useCollectorSkill(0, choice);
                    if (selectedCard.getColorString() != "UNKNOWN") {
                        message = "Used Collector! Stole: " + selectedCard.getColorString() + " " + selectedCard.getValueString();
                    } else {
                        message = "Failed to use Collector skill!";
                    }
                    cardChosen = true;
                }
                break;
            case 'q':
            case 'Q':
                message = "Collector skill cancelled";
                cardChosen = true;
                break;
        }
    }
    needsRedraw = true;
}

void UnoUI::handleThiefSkill() {
    std::vector<std::string> playerNames = gameEngine.getPlayerNames();
    int targetChoice = 0;
    int cardTypeChoice = 0; // 0 = Action cards, 1 = Number cards
    bool skillUsed = false;
    int width = getConsoleWidth();
    int height = getConsoleHeight();
    
    while (!skillUsed) {
        clearScreen();
        
        // Simple header
        drawCenteredText(2, "THIEF SKILL - STEAL FROM OPPONENT");
        drawCenteredText(3, "=================================");
        std::cout << std::endl;
        
        // Target selection with better spacing
        drawCenteredText(5, "SELECT TARGET PLAYER:");
        std::cout << std::endl;
        
        for (int i = 1; i < playerNames.size(); i++) { // Skip player 0 (self)
            int yPos = 7 + (i * 2);
            drawCenteredText(yPos, playerNames[i]);
            
            if (i-1 == targetChoice) {
                drawCenteredText(yPos + 1, ">>> SELECTED <<<");
            } else {
                drawCenteredText(yPos + 1, "   [SELECT]   ");
            }
        }
        
        std::cout << std::endl;
        
        // Card type selection
        drawCenteredText(15, "SELECT CARD TYPE TO STEAL:");
        if (cardTypeChoice == 0) {
            drawCenteredText(17, ">>> ACTION CARDS <<<");
            drawCenteredText(18, "    NUMBER CARDS    ");
        } else {
            drawCenteredText(17, "    ACTION CARDS    ");
            drawCenteredText(18, ">>> NUMBER CARDS <<<");
        }
        
        std::cout << std::endl;
        drawCenteredText(21, "CONTROLS: A=Up, D=Down, W=Switch Type");
        drawCenteredText(22, "ENTER=Use Skill, Q=Cancel");
        
        char ch = _getch();
        switch (ch) {
            case 'a':
            case 'A':
                if (targetChoice > 0) targetChoice--;
                break;
            case 'd':
            case 'D':
                if (targetChoice < playerNames.size() - 2) targetChoice++;
                break;
            case 'w':
            case 'W':
                cardTypeChoice = (cardTypeChoice == 0) ? 1 : 0;
                break;
            case 13: // Enter
                {
                    bool success = gameEngine.useThiefSkill(0, targetChoice + 1, cardTypeChoice == 0);
                    if (success) {
                        message = "Used Thief! Stole from " + playerNames[targetChoice + 1];
                    } else {
                        // Check if it was blocked by Defender
                        if (gameEngine.getPlayerCharacters()[targetChoice + 1] == "Defender") {
                            message = "Thief skill blocked by Defender!";
                        } else {
                            std::string cardType = (cardTypeChoice == 0 ? "action" : "number");
                            message = "Thief skill failed! Target has no " + cardType + " cards";
                        }
                    }
                    skillUsed = true;
                }
                break;
            case 'q':
            case 'Q':
                message = "Thief skill cancelled";
                skillUsed = true;
                break;
        }
    }
    needsRedraw = true;
}

void UnoUI::showWinner() {
    int winner = gameEngine.getWinner();
    if (winner == -1) return;
    
    clearScreen();
    drawCenteredText(10, "GAME OVER!");
    drawCenteredText(11, "==========");
    std::cout << std::endl;
    
    std::vector<std::string> playerNames = gameEngine.getPlayerNames();
    if (winner < playerNames.size()) {
        drawCenteredText(13, "WINNER: " + playerNames[winner] + "!");
    }
    
    drawCenteredText(15, "Final Card Counts:");
    std::vector<int> cardCounts = gameEngine.getPlayerCardCounts();
    for (int i = 0; i < playerNames.size(); i++) {
        std::stringstream ss;
        ss << playerNames[i] << ": " << cardCounts[i] << " cards";
        drawCenteredText(17 + i, ss.str());
    }
    
    std::cout << std::endl;
    drawCenteredText(25, "Press any key to exit...");
    _getch();
    gameRunning = false;
}

void UnoUI::handleInput() {
    // 检查Flash效果
    if (gameEngine.isFlashEffectActive() && gameEngine.getCurrentPlayerIndex() == 0) {
        std::string newMessage = "FLASH EFFECT! You must play a " + 
                                getColorString(gameEngine.getFlashColor()) + 
                                " card or draw " + std::to_string(gameEngine.getFlashChainCount()) + " cards!";
        
        // 只有当消息改变时才更新，避免频繁重绘
        if (message != newMessage) {
            message = newMessage;
            needsRedraw = true;
        }
    }
    
    // Check for winner at the start
    if (gameEngine.isGameOver()) {
        showWinner();
        return;
    }
    
    if (_kbhit()) {
        char ch = _getch();
        
        // Only process input if it's the human player's turn
        if (gameEngine.getCurrentPlayerIndex() != 0) {
            // It's CPU's turn, let them play automatically
            return;
        }
        
        switch (ch) {
            case 'a':
            case 'A':
                if (selectedCardIndex > 0) {
                    selectedCardIndex--;
                    message = "Selected Card " + std::to_string(selectedCardIndex + 1);
                    needsRedraw = true;
                }
                break;
                
            case 'd':
            case 'D':
                if (selectedCardIndex < gameEngine.getPlayerHand(0).size() - 1) {
                    selectedCardIndex++;
                    message = "Selected Card " + std::to_string(selectedCardIndex + 1);
                    needsRedraw = true;
                }
                break;
                
            case 'w':
            case 'W':
                {
                    // 在Flash效果中，按W键表示无法出牌，触发惩罚
                    if (gameEngine.isFlashEffectActive() && gameEngine.getCurrentPlayerIndex() == 0) {
                        int cardsToDraw = gameEngine.getFlashChainCount();
                        for (int i = 0; i < cardsToDraw; i++) {
                            gameEngine.drawCard(0);
                        }
                        message = "You drew " + std::to_string(cardsToDraw) + " cards due to Flash effect!";
                        gameEngine.endFlashEffect();
                        gameEngine.nextTurn();
                        needsRedraw = true;
                    }
                    // 正常抽牌逻辑
                    else if (gameEngine.getCurrentPlayerIndex() == 0) {
                        Card currentCard = gameEngine.getCurrentCard();
                        if (currentCard.value == CardValue::DRAW_TWO || currentCard.value == CardValue::WILD_DRAW_FOUR) {
                            // Forced draw - player must draw the required cards
                            int cardsToDraw = (currentCard.value == CardValue::DRAW_TWO) ? 2 : 4;
                            for (int i = 0; i < cardsToDraw; i++) {
                                gameEngine.drawCard(0);
                            }
                            message = "You drew " + std::to_string(cardsToDraw) + " cards due to action card!";
                            gameEngine.nextTurn(); // Skip turn after drawing
                        } else {
                            // Normal draw
                            Card drawnCard = gameEngine.drawCard(0);
                            message = "Drew: " + drawnCard.getColorString() + " " + drawnCard.getValueString();
                            gameEngine.nextTurn();
                        }
                        needsRedraw = true;
                    } else {
                        message = "Not your turn!";
                        needsRedraw = true;
                    }
                }
                break;
                
            case 's':
            case 'S':
                if (gameEngine.canUseSkill(0)) {
                    std::string character = gameEngine.getPlayerCharacters()[0];
                    if (character == "Lucky Star") {
                        handleLuckyStarSkill();
                    } else if (character == "Collector") {
                        handleCollectorSkill();
                    } else if (character == "Thief") {
                        handleThiefSkill();
                    } else if (character == "Defender") {
                        message = "Defender skill is passive and activates automatically!";
                    }
                    needsRedraw = true;
                } else {
                    message = "Cannot use skill now!";
                    needsRedraw = true;
                }
                break;
                
            case 13: // Enter - play card
                {
                    std::vector<Card> hand = gameEngine.getPlayerHand(0);
                    if (!hand.empty() && selectedCardIndex < hand.size()) {
                        Card selectedCard = hand[selectedCardIndex];
                        
                        // 在Flash效果中，检查是否允许出这张牌
                        if (gameEngine.isFlashEffectActive() && gameEngine.getCurrentPlayerIndex() == 0) {
                            if (!selectedCard.isWildCard() && selectedCard.color == gameEngine.getFlashColor()) {
                                // 允许出指定颜色的牌
                                if (gameEngine.playCard(0, selectedCard)) {
                                    message = "Played: " + selectedCard.getColorString() + " " + selectedCard.getValueString();
                                    gameEngine.nextTurn();
                                    
                                    // Reset selection if hand size changed
                                    if (selectedCardIndex >= gameEngine.getPlayerHand(0).size() && 
                                        !gameEngine.getPlayerHand(0).empty()) {
                                        selectedCardIndex = gameEngine.getPlayerHand(0).size() - 1;
                                    }
                                } else {
                                    message = "Failed to play card!";
                                }
                            } else {
                                message = "Flash effect! You can only play " + 
                                         getColorString(gameEngine.getFlashColor()) + " cards!";
                            }
                        }
                        // 正常出牌逻辑
                        else if (gameEngine.canPlayCard(0, selectedCard)) {
                            // 关键修复：Package卡不是Wild卡，应该进入else分支
                            if (selectedCard.isWildCard()) {
                                // 只有真正的Wild卡才需要选择颜色
                                CardColor chosenColor = chooseColor();
                                if (gameEngine.playCard(0, selectedCard, chosenColor)) {
                                    message = "Played: " + selectedCard.getColorString() + 
                                            " (Chose: " + getColorString(chosenColor) + ")";
                                    
                                    // Check if we played an action card that affects others
                                    if (selectedCard.value == CardValue::DRAW_TWO) {
                                        int nextPlayerIndex = gameEngine.getNextPlayerIndex();
                                        std::string nextPlayerName = gameEngine.getPlayerNames()[nextPlayerIndex];
                                        message += " - " + nextPlayerName + " draws 2 cards!";
                                    } else if (selectedCard.value == CardValue::WILD_DRAW_FOUR) {
                                        int nextPlayerIndex = gameEngine.getNextPlayerIndex();
                                        std::string nextPlayerName = gameEngine.getPlayerNames()[nextPlayerIndex];
                                        message += " - " + nextPlayerName + " draws 4 cards and is skipped!";
                                    } else if (selectedCard.value == CardValue::SKIP) {
                                        int nextPlayerIndex = gameEngine.getNextPlayerIndex();
                                        std::string nextPlayerName = gameEngine.getPlayerNames()[nextPlayerIndex];
                                        message += " - " + nextPlayerName + " is skipped!";
                                    } else if (selectedCard.value == CardValue::REVERSE) {
                                        message += " - Direction reversed!";
                                    }
                                    
                                    // For Wild Draw Four, don't call nextTurn() - it's handled in handleWildCard
                                    if (selectedCard.value != CardValue::WILD_DRAW_FOUR) {
                                        gameEngine.nextTurn();
                                    }
                                    
                                    // Reset selection if hand size changed
                                    if (selectedCardIndex >= gameEngine.getPlayerHand(0).size() && 
                                        !gameEngine.getPlayerHand(0).empty()) {
                                        selectedCardIndex = gameEngine.getPlayerHand(0).size() - 1;
                                    }
                                } else {
                                    message = "Failed to play card!";
                                }
                            } else {
                                // Package卡和其他普通卡在这里处理
                                if (gameEngine.playCard(0, selectedCard)) {
                                    message = "Played: " + selectedCard.getColorString() + " " + selectedCard.getValueString();
                                    
                                    // 特殊处理Package卡的消息
                                    if (selectedCard.value == CardValue::PACKAGE) {
                                        message += " - Discarded all " + selectedCard.getColorString() + " number cards!";
                                    }
                                    // 其他动作卡处理
                                    else if (selectedCard.value == CardValue::DRAW_TWO) {
                                        int nextPlayerIndex = gameEngine.getNextPlayerIndex();
                                        std::string nextPlayerName = gameEngine.getPlayerNames()[nextPlayerIndex];
                                        message += " - " + nextPlayerName + " draws 2 cards!";
                                    } else if (selectedCard.value == CardValue::SKIP) {
                                        int nextPlayerIndex = gameEngine.getNextPlayerIndex();
                                        std::string nextPlayerName = gameEngine.getPlayerNames()[nextPlayerIndex];
                                        message += " - " + nextPlayerName + " is skipped!";
                                    } else if (selectedCard.value == CardValue::REVERSE) {
                                        message += " - Direction reversed!";
                                    }
                                    
                                    gameEngine.nextTurn();
                                    
                                    // Reset selection if hand size changed
                                    if (selectedCardIndex >= gameEngine.getPlayerHand(0).size() && 
                                        !gameEngine.getPlayerHand(0).empty()) {
                                        selectedCardIndex = gameEngine.getPlayerHand(0).size() - 1;
                                    }
                                } else {
                                    message = "Failed to play card!";
                                }
                            }
                        } else {
                            message = "Cannot play that card! Current: " + 
                                    gameEngine.getCurrentCard().getColorString() + " " + 
                                    gameEngine.getCurrentCard().getValueString();
                        }
                        needsRedraw = true;
                    } else {
                        message = "No cards to play!";
                        needsRedraw = true;
                    }
                }
                break;
                
            case 'q':
            case 'Q':
                gameRunning = false;
                message = "Thanks for playing!";
                needsRedraw = true;
                break;
        }
    }
    
    // Handle CPU turns automatically (even without key press)
    if (gameEngine.getCurrentPlayerIndex() != 0) {
        // Small delay to make CPU turns visible
        Sleep(1500);
        
        // Store current state to detect action card effects
        int previousHandSize = gameEngine.getPlayerHand(0).size();
        Card previousCard = gameEngine.getCurrentCard();
        int previousPlayerIndex = gameEngine.getCurrentPlayerIndex();
        
        gameEngine.playCPUTurn();
        
        // 检查Flash效果是否还在继续
        if (gameEngine.isFlashEffectActive()) {
            // 如果Flash效果还在继续，确保游戏继续运行
            needsRedraw = true;
        }
        
        // Check if action card affected the human player
        Card newCard = gameEngine.getCurrentCard();
        int currentHandSize = gameEngine.getPlayerHand(0).size();
        int currentPlayerIndex = gameEngine.getCurrentPlayerIndex();
        
        // Detect if human player was forced to draw cards
        if (currentHandSize > previousHandSize) {
            int cardsDrawn = currentHandSize - previousHandSize;
            if (cardsDrawn == 2 && (previousCard.value == CardValue::DRAW_TWO)) {
                message = "CPU played DRAW TWO! You drew 2 cards and are skipped!";
            } else if (cardsDrawn == 4 && (previousCard.value == CardValue::WILD_DRAW_FOUR)) {
                message = "CPU played WILD DRAW FOUR! You drew 4 cards and are skipped!";
            } else if (gameEngine.isFlashEffectActive()) {
                message = "Flash effect! You drew " + std::to_string(cardsDrawn) + " cards!";
            }
        }
        // Detect if human player was skipped - FIXED LOGIC
        else if (previousCard.value == CardValue::SKIP && currentPlayerIndex != 0) {
            // Check if the skip should have affected the human player
            // When CPU plays skip, the next player (which might be human) gets skipped
            int nextPlayerAfterCPU = (previousPlayerIndex + 1) % gameEngine.getPlayerNames().size();
            if (nextPlayerAfterCPU == 0) { // If human player would be next, they were skipped
                message = "CPU played SKIP! Your turn is skipped!";
            }
        }
        // Detect if direction was reversed
        else if (previousCard.value == CardValue::REVERSE) {
            message = "CPU played REVERSE! Direction changed!";
        }
        // Detect if CPU played a wild card
        else if (previousCard.isWildCard() && previousPlayerIndex != currentPlayerIndex) {
            if (previousCard.value == CardValue::WILD_DRAW_FOUR) {
                message = "CPU played WILD DRAW FOUR! You drew 4 cards and are skipped!";
            } else if (previousCard.value == CardValue::FLASH) {
                message = "CPU played FLASH! Starting Flash effect!";
            } else {
                message = "CPU played " + previousCard.getColorString() + "!";
            }
        }
        
        // Advance turn after CPU plays (if not already advanced by action cards)
        if (gameEngine.getCurrentPlayerIndex() == previousPlayerIndex) {
            gameEngine.nextTurn();
        }
        needsRedraw = true;
    }
}

bool UnoUI::isGameRunning() const {
    return gameRunning;
}