#include "gameengine.h"
#include <algorithm>
#include <random>
#include <stdexcept>
#include <iostream>
#include "helpers.h"
#include <map>

GameEngine::GameEngine() : currentPlayerIndex(0), gameDirection(true), 
                          gameStarted(false), chosenWildColor(CardColor::RED),
                          currentPlayColor(CardColor::RED),
                          flashEffectActive(false), flashColor(CardColor::RED),
                          flashChainCount(0), flashStartingPlayer(-1),
                          currentRound(0) {
    std::random_device rd;
    rng.seed(rd());
}

void GameEngine::initializeDeck() {
    drawPile.clear();
    
    for (int color = 0; color < 4; color++) {
        CardColor cardColor = static_cast<CardColor>(color);
        
        // Two of each number 1-9 (using explicit enum values to avoid off-by-one errors)
        drawPile.push_back(Card(cardColor, CardValue::ONE));
        drawPile.push_back(Card(cardColor, CardValue::ONE));
        drawPile.push_back(Card(cardColor, CardValue::TWO));
        drawPile.push_back(Card(cardColor, CardValue::TWO));
        drawPile.push_back(Card(cardColor, CardValue::THREE));
        drawPile.push_back(Card(cardColor, CardValue::THREE));
        drawPile.push_back(Card(cardColor, CardValue::FOUR));
        drawPile.push_back(Card(cardColor, CardValue::FOUR));
        drawPile.push_back(Card(cardColor, CardValue::FIVE));
        drawPile.push_back(Card(cardColor, CardValue::FIVE));
        drawPile.push_back(Card(cardColor, CardValue::SIX));
        drawPile.push_back(Card(cardColor, CardValue::SIX));
        drawPile.push_back(Card(cardColor, CardValue::SEVEN));
        drawPile.push_back(Card(cardColor, CardValue::SEVEN));
        drawPile.push_back(Card(cardColor, CardValue::EIGHT));
        drawPile.push_back(Card(cardColor, CardValue::EIGHT));
        drawPile.push_back(Card(cardColor, CardValue::NINE));
        drawPile.push_back(Card(cardColor, CardValue::NINE));
        
        // Two of each action card per color
        drawPile.push_back(Card(cardColor, CardValue::SKIP));
        drawPile.push_back(Card(cardColor, CardValue::SKIP));
        
        drawPile.push_back(Card(cardColor, CardValue::REVERSE));
        drawPile.push_back(Card(cardColor, CardValue::REVERSE));
        
        drawPile.push_back(Card(cardColor, CardValue::DRAW_TWO));
        drawPile.push_back(Card(cardColor, CardValue::DRAW_TWO));
        
        drawPile.push_back(Card(cardColor, CardValue::PACKAGE));
        drawPile.push_back(Card(cardColor, CardValue::PACKAGE));
    }
    
    // Four Wild cards
    for (int i = 0; i < 4; i++) {
        drawPile.push_back(Card(CardColor::WILD, CardValue::WILD));
    }
    
    // Two Wild Draw Four cards
    for (int i = 0; i < 2; i++) {
        drawPile.push_back(Card(CardColor::WILD, CardValue::WILD_DRAW_FOUR));
    }
    
    // Two Flash cards
    for (int i = 0; i < 2; i++) {
        drawPile.push_back(Card(CardColor::WILD, CardValue::FLASH));
    }
    
    // Debug: Print deck composition
    std::cout << "Deck initialized with " << drawPile.size() << " cards:" << std::endl;
    
    // 修改统计代码，移除zeroCards的统计
    int numberCards = 0, actionCards = 0, wildCards = 0;
    for (const Card& card : drawPile) {
        if (card.value >= CardValue::ONE && card.value <= CardValue::NINE) numberCards++;
        else if (card.value >= CardValue::SKIP && card.value <= CardValue::PACKAGE) actionCards++;
        else if (card.isWildCard()) wildCards++;
    }

    std::cout << "Number cards (1-9): " << numberCards << " (should be 72)" << std::endl;
    std::cout << "Action cards: " << actionCards << " (should be 32)" << std::endl;
    std::cout << "Wild cards: " << wildCards << " (should be 8)" << std::endl;
}

void GameEngine::shuffleDeck() {
    std::shuffle(drawPile.begin(), drawPile.end(), rng);
}

void GameEngine::dealInitialCards() {
    for (auto& player : players) {
        for (int i = 0; i < 7; i++) {
            player->addCard(drawFromDeck());
        }
    }
}

void GameEngine::assignCharacters() {
    std::vector<CharacterType> characters = {
        CharacterType::LUCKY_STAR,
        CharacterType::COLLECTOR, 
        CharacterType::THIEF,
        CharacterType::DEFENDER
    };
    
    std::shuffle(characters.begin(), characters.end(), rng);
    
    for (size_t i = 0; i < players.size() && i < characters.size(); i++) {
        players[i]->setCharacter(characters[i]);
    }
}

Card GameEngine::drawFromDeck() {
    if (drawPile.empty()) {
        // Reshuffle discard pile if draw pile is empty
        if (discardPile.size() > 1) {
            Card topCard = discardPile.back();
            discardPile.pop_back();
            
            drawPile = discardPile;
            discardPile.clear();
            discardPile.push_back(topCard);
            
            shuffleDeck();
        } else {
            throw std::runtime_error("No cards left to draw!");
        }
    }
    
    Card card = drawPile.back();
    drawPile.pop_back();
    return card;
}

void GameEngine::addPlayer(const std::string& name) {
    if (gameStarted) return;
    
    int playerId = players.size();
    players.push_back(std::make_unique<Player>(playerId, name));
}

void GameEngine::startGame() {
    if (players.size() < 2) return;
    
    initializeDeck();
    shuffleDeck();
    assignCharacters();
    dealInitialCards();
    
    // Ensure first card is not a wild card
    Card firstCard;
    do {
        if (!drawPile.empty()) {
            firstCard = drawPile.back();
            drawPile.pop_back();
            discardPile.push_back(firstCard);
        }
    } while (firstCard.isWildCard() && !drawPile.empty());
    
    gameStarted = true;
    currentPlayerIndex = 0;
    currentPlayColor = firstCard.color;
    currentRound = 0;
}

Card GameEngine::getCurrentCard() const {
    if (discardPile.empty()) return Card();
    return discardPile.back();
}

std::vector<Card> GameEngine::getPlayerHand(int playerId) const {
    if (playerId >= 0 && playerId < players.size()) {
        return players[playerId]->getHand();
    }
    return {};
}

std::vector<std::string> GameEngine::getPlayerCharacters() const {
    std::vector<std::string> characters;
    for (const auto& player : players) {
        characters.push_back(player->getCharacterString());
    }
    return characters;
}

std::vector<int> GameEngine::getPlayerCardCounts() const {
    std::vector<int> counts;
    for (const auto& player : players) {
        counts.push_back(player->getHandSize());
    }
    return counts;
}

std::string GameEngine::getCurrentPlayerName() const {
    if (currentPlayerIndex < players.size()) {
        return players[currentPlayerIndex]->getName();
    }
    return "";
}

int GameEngine::getNextPlayerIndex() const {
    if (gameDirection) {
        return (currentPlayerIndex + 1) % players.size();
    } else {
        return (currentPlayerIndex - 1 + players.size()) % players.size();
    }
}

bool GameEngine::canPlayCard(int playerId, const Card& card) const {
    if (playerId != currentPlayerIndex) return false;
    
    Card currentCard = getCurrentCard();
    
    // Special restriction for Wild Draw Four
    if (card.value == CardValue::WILD_DRAW_FOUR) {
        Player* player = players[playerId].get();
        if (player->hasColorCard(currentPlayColor)) {
            return false; // Cannot play Wild Draw Four if you have matching color
        }
    }
    
    // 关键修改：在Flash效果期间的特殊规则
    if (flashEffectActive && playerId != flashStartingPlayer) {
        // 在Flash效果中，只能出指定颜色的牌，不能通过wild或数字相等出其他颜色的牌
        if (!card.isWildCard() && card.color == flashColor) {
            return true; // 允许出指定颜色的牌
        }
        return false; // 禁止其他所有出牌方式
    }
    
    // Wild cards can always be played (除了在Flash效果中)
    if (card.isWildCard()) {
        return true;
    }
    
    // Package cards can be played on other package cards
    if (card.isPackageCard() && currentCard.isPackageCard()) {
        return true;
    }
    
    // If current card is wild, check against the chosen color
    if (currentCard.color == CardColor::WILD) {
        return card.color == currentPlayColor || card.value == currentCard.value;
    }
    
    // FIXED: Get the actual color to compare against
    CardColor colorToCompare = currentPlayColor;
    
    // For non-wild cards, we should also check against the actual card color
    if (currentCard.color != CardColor::WILD) {
        colorToCompare = currentCard.color;
    }
    
    // Same color
    if (card.color == colorToCompare) {
        return true;
    }
    
    // Same value (number or action)
    if (card.value == currentCard.value) {
        return true;
    }
    
    return false;
}

bool GameEngine::playCard(int playerId, const Card& card, CardColor chosenColor) {
    if (!canPlayCard(playerId, card)) return false;
    
    Player* player = players[playerId].get();
    if (!player->removeCard(card)) return false;
    
    discardPile.push_back(card);
    currentTurnDiscards.push_back(card); // Track for Collector skill restriction
    
    // 处理Flash效果中的chain计数
    if (flashEffectActive && playerId != flashStartingPlayer) {
        // 只有在Flash效果中且不是起始玩家出的指定颜色牌才增加chain
        if (!card.isWildCard() && card.color == flashColor) {
            flashChainCount++;
            std::cout << "Flash chain increased to " << flashChainCount << " cards!" << std::endl;
            std::cout << "Next player unable to play will draw " << flashChainCount << " cards!" << std::endl;
        }
    }
    
    // Set the current play color based on card type
    if (card.isWildCard()) {
        currentPlayColor = chosenColor;
        chosenWildColor = chosenColor;
        handleWildCard(card, chosenColor);
    } else {
        currentPlayColor = card.color;
        if (card.isActionCard()) {
            handleActionCard(card);
        }
        
        // If it's a Package card, automatically trigger the skill using the card's color
        if (card.value == CardValue::PACKAGE) {
            usePackageSkill(playerId, card.color);
        }
    }
    
    return true;
}

bool GameEngine::canPlayMultipleCards(int playerId, const std::vector<Card>& cards) const {
    if (playerId != currentPlayerIndex || cards.empty()) {
        return false;
    }
    
    // First card must be playable on current card
    if (!canPlayCard(playerId, cards[0])) {
        return false;
    }
    
    // All subsequent cards must have the same value as the first card
    CardValue firstValue = cards[0].value;
    for (size_t i = 1; i < cards.size(); i++) {
        if (cards[i].value != firstValue) {
            return false;
        }
    }
    
    return true;
}

bool GameEngine::playMultipleCards(int playerId, const std::vector<Card>& cards, CardColor chosenColor) {
    if (!canPlayMultipleCards(playerId, cards)) {
        return false;
    }
    
    Player* player = players[playerId].get();
    
    // Remove all cards from player's hand
    for (const Card& card : cards) {
        if (!player->removeCard(card)) {
            return false; // Card not found in hand
        }
        discardPile.push_back(card);
        currentTurnDiscards.push_back(card); // Track for Collector skill
    }
    
    // Handle the played cards
    Card firstCard = cards[0];
    if (firstCard.isWildCard()) {
        handleWildCard(firstCard, chosenColor);
        currentPlayColor = chosenColor;
    } else if (firstCard.isActionCard()) {
        handleActionCard(firstCard);
        currentPlayColor = firstCard.color;
    } else {
        currentPlayColor = firstCard.color;
    }
    
    return true;
}

Card GameEngine::drawCard(int playerId) {
    if (playerId >= 0 && playerId < players.size()) {
        Card card = drawFromDeck();
        players[playerId]->addCard(card);
        return card;
    }
    return Card();
}

void GameEngine::nextTurn() {
    currentTurnDiscards.clear(); // Clear turn discards for Collector skill
    
    // 在切换玩家之前检查Flash效果
    // 只在Flash效果激活且当前玩家不是起始玩家时才处理
    if (flashEffectActive && currentPlayerIndex != flashStartingPlayer) {
        handleFlashEffect(currentPlayerIndex);
        
        // 如果Flash效果结束了，继续正常游戏
        if (!flashEffectActive) {
            // Flash效果已结束，正常切换玩家
            advanceToNextPlayer();
        }
        // 如果Flash效果还在继续，玩家需要出牌，不切换玩家
    } else {
        // 正常情况下的玩家切换
        advanceToNextPlayer();
    }
    
    // 如果Flash效果还在继续，在新玩家回合开始时给出提示
    if (flashEffectActive && currentPlayerIndex != flashStartingPlayer) {
        std::cout << "Flash effect active! " << players[currentPlayerIndex]->getName()
                  << " must play a " << getColorString(flashColor) 
                  << " card or draw " << flashChainCount << " cards!" << std::endl;
    }
}

// 添加辅助方法来处理玩家切换
void GameEngine::advanceToNextPlayer() {
    if (gameDirection) {
        currentPlayerIndex = (currentPlayerIndex + 1) % players.size();
    } else {
        currentPlayerIndex = (currentPlayerIndex - 1 + players.size()) % players.size();
    }
    
    // Check if we completed a full round (back to first player)
    if (currentPlayerIndex == 0) {
        currentRound++;
        for (auto& player : players) {
            player->incrementRound();
        }
    }
}
void GameEngine::handleActionCard(const Card& card) {
    switch(card.value) {
        case CardValue::SKIP:
            {
                int skippedPlayer = getNextPlayerIndex();
                std::cout << players[skippedPlayer]->getName() << " was skipped!" << std::endl;
                // Simply advance the turn twice to skip one player
                nextTurn(); // This moves to the player who should be skipped
                nextTurn(); // This moves to the player after the skipped one
            }
            break;
        case CardValue::REVERSE:
            {
                gameDirection = !gameDirection;
                std::cout << "Direction reversed! Now going " << (gameDirection ? "clockwise" : "counter-clockwise") << std::endl;
                // In 2-player game, reverse acts as skip
                if (players.size() == 2) {
                    nextTurn();
                }
            }
            break;
        case CardValue::DRAW_TWO:
            {
                int affectedPlayer = getNextPlayerIndex();
                std::cout << players[affectedPlayer]->getName() << " draws 2 cards and is skipped!" << std::endl;
                // Draw cards for the affected player
                drawCard(affectedPlayer);
                drawCard(affectedPlayer);
                // Skip the affected player by advancing an extra turn
                nextTurn();
            }
            break;
        case CardValue::PACKAGE:
            {
                std::cout << "Package card played! Player can discard all cards of one color." << std::endl;
                // Package card effect is handled separately through usePackageSkill
                // No turn skip for Package card
            }
            break;
        default:
            break;
    }
}

void GameEngine::handleWildCard(const Card& card, CardColor chosenColor) {
    // Store the next player before any turns change
    int affectedPlayer = getNextPlayerIndex();
    
    switch(card.value) {
        case CardValue::WILD_DRAW_FOUR:
            {
                std::cout << players[affectedPlayer]->getName() << " draws 4 cards and is skipped!" << std::endl;
                // Draw 4 cards for the affected player
                for (int i = 0; i < 4; i++) {
                    drawCard(affectedPlayer);
                }
                // Skip the affected player by advancing an extra turn
                nextTurn();
            }
            break;
        case CardValue::FLASH:
            {
                // 开始Flash效果
                flashEffectActive = true;
                flashColor = chosenColor;
                flashChainCount = 1; // Flash卡本身是第一张
                flashStartingPlayer = currentPlayerIndex;
                
                std::cout << "Flash card played! Starting Flash effect!" << std::endl;
                std::cout << "All players must play " << getColorString(chosenColor) 
                          << " cards in sequence!" << std::endl;
                std::cout << "Players unable to play will draw " << flashChainCount 
                          << " cards!" << std::endl;
                
                // Flash卡玩家完成出牌后立即进入下一个玩家
                // 不在这里调用nextTurn()，让正常的游戏流程处理
            }
            break;
        case CardValue::WILD:
            // Regular wild card just changes color, no special effect
            std::cout << "Wild card played! Color chosen: " << getColorString(chosenColor) << std::endl;
            break;
        default:
            break;
    }
    chosenWildColor = chosenColor;
    currentPlayColor = chosenColor;
}

void GameEngine::handleFlashEffect(int playerId) {
    if (!flashEffectActive) return;
    
    Player* currentPlayer = players[playerId].get();
    
    // 检查当前玩家是否有指定颜色的牌
    bool canPlayFlashColor = false;
    std::vector<Card> hand = currentPlayer->getHand();
    for (const Card& card : hand) {
        // 在Flash效果中，只能出指定颜色的牌（不能出wild卡）
        if (!card.isWildCard() && card.color == flashColor) {
            canPlayFlashColor = true;
            break;
        }
    }
    
    if (!canPlayFlashColor) {
        // 玩家无法出牌 - 必须抽取与chain count相等的牌数
        int cardsToDraw = flashChainCount;
        std::cout << currentPlayer->getName() << " cannot play a " << getColorString(flashColor) 
                  << " card! Drawing " << cardsToDraw << " cards!" << std::endl;
        
        for (int i = 0; i < cardsToDraw; i++) {
            drawCard(playerId);
        }
        
        // Flash效果结束
        endFlashEffect();
        
        // 重要：在Flash效果结束后，让当前玩家继续正常游戏
        // 不要在这里调用nextTurn()，让正常的游戏流程处理
    }
    // 注意：如果玩家可以出牌，不需要特殊处理，游戏会正常继续
}

void GameEngine::endFlashEffect() {
    flashEffectActive = false;
    flashChainCount = 0;
    flashStartingPlayer = -1;
    std::cout << "Flash effect ended!" << std::endl;
}

bool GameEngine::canUseSkill(int playerId) const {
    if (playerId >= 0 && playerId < players.size()) {
        return players[playerId]->canUseSkill();
    }
    return false;
}

void GameEngine::useSkill(int playerId) {
    if (canUseSkill(playerId)) {
        players[playerId]->useSkill();
    }
}

void GameEngine::resetSkillCooldowns() {
    for (auto& player : players) {
        player->resetSkillCooldown();
    }
}

std::vector<Card> GameEngine::peekTopCards(int count) {
    std::vector<Card> result;
    for (int i = 0; i < count && i < drawPile.size(); i++) {
        result.push_back(drawPile[drawPile.size() - 1 - i]);
    }
    return result;
}

bool GameEngine::isGameOver() const {
    for (const auto& player : players) {
        if (player->getHandSize() == 0) {
            return true;
        }
    }
    return false;
}

int GameEngine::getWinner() const {
    for (size_t i = 0; i < players.size(); i++) {
        if (players[i]->getHandSize() == 0) {
            return i;
        }
    }
    return -1;
}

void GameEngine::playCPUTurn() {
    if (currentPlayerIndex == 0) return; // Skip if it's human player's turn
    
    Player* cpuPlayer = players[currentPlayerIndex].get();
    
    // CPU might use skill (simple 30% chance if skill is available)
    if (cpuPlayer->canUseSkill() && (std::uniform_int_distribution<>(1, 100)(rng) <= 30)) {
        // Choose which skill to use based on character
        CharacterType cpuCharacter = cpuPlayer->getCharacter();
        bool skillUsed = false;
        
        switch(cpuCharacter) {
            case CharacterType::THIEF:
                {
                    // Thief: Try to steal from a random player
                    std::vector<int> possibleTargets;
                    for (int i = 0; i < players.size(); i++) {
                        if (i != currentPlayerIndex) {
                            possibleTargets.push_back(i);
                        }
                    }
                    
                    if (!possibleTargets.empty()) {
                        int targetIndex = std::uniform_int_distribution<>(0, possibleTargets.size() - 1)(rng);
                        int targetPlayer = possibleTargets[targetIndex];
                        bool stealActionCards = std::uniform_int_distribution<>(0, 1)(rng) == 0;
                        
                        skillUsed = useThiefSkill(currentPlayerIndex, targetPlayer, stealActionCards);
                        if (skillUsed) {
                            std::cout << cpuPlayer->getName() << " used Thief skill on " 
                                      << players[targetPlayer]->getName() << "!" << std::endl;
                        }
                    }
                }
                break;
                
            case CharacterType::COLLECTOR:
                {
                    // Collector: Steal from discard pile if not empty
                    if (!discardPile.empty()) {
                        int cardIndex = std::uniform_int_distribution<>(0, discardPile.size() - 1)(rng);
                        Card stolenCard = useCollectorSkill(currentPlayerIndex, cardIndex);
                        if (stolenCard.getColorString() != "UNKNOWN") {
                            skillUsed = true;
                            std::cout << cpuPlayer->getName() << " used Collector skill!" << std::endl;
                        }
                    }
                }
                break;
                
            case CharacterType::LUCKY_STAR:
                {
                    // Lucky Star: Draw 3 pick 1
                    std::vector<Card> topCards = useLuckyStarSkill(currentPlayerIndex);
                    if (!topCards.empty()) {
                        skillUsed = true;
                        // CPU randomly picks one card
                        int choice = std::uniform_int_distribution<>(0, topCards.size() - 1)(rng);
                        completeLuckyStarSkill(currentPlayerIndex, choice, topCards);
                        std::cout << cpuPlayer->getName() << " used Lucky Star skill!" << std::endl;
                    }
                }
                break;
                
            case CharacterType::DEFENDER:
                // Defender skill is passive and automatic - no active use
                break;
        }
        
        if (!skillUsed && cpuCharacter != CharacterType::DEFENDER) {
            std::cout << cpuPlayer->getName() << " failed to use skill!" << std::endl;
        }
    }
    
    // Rest of the CPU turn logic (card playing)
    std::vector<Card> cpuHand = cpuPlayer->getHand();
    Card currentCard = getCurrentCard();
    
    // 在Flash效果中，CPU只能尝试出指定颜色的牌
    if (flashEffectActive && currentPlayerIndex != flashStartingPlayer) {
        // 寻找指定颜色的牌
        for (const Card& card : cpuHand) {
            if (!card.isWildCard() && card.color == flashColor && canPlayCard(currentPlayerIndex, card)) {
                playCard(currentPlayerIndex, card);
                std::cout << cpuPlayer->getName() << " played: " << card.getColorString() 
                          << " " << card.getValueString() << std::endl;
                return;
            }
        }
        
        // 如果没有指定颜色的牌，必须抽牌
        Card drawnCard = drawCard(currentPlayerIndex);
        std::cout << cpuPlayer->getName() << " cannot play " << getColorString(flashColor) 
                  << " card! Drawing " << flashChainCount << " cards!" << std::endl;
        
        // 处理Flash效果惩罚
        handleFlashEffect(currentPlayerIndex);
        return;
    }
    
    // 正常游戏逻辑
    // Try to find a playable card
    for (const Card& card : cpuHand) {
        if (canPlayCard(currentPlayerIndex, card)) {
            // CPU plays the first playable card it finds
            if (card.isWildCard()) {
                // CPU randomly chooses a color
                CardColor randomColor = static_cast<CardColor>(std::uniform_int_distribution<>(0, 3)(rng));
                
                if (card.value == CardValue::FLASH) {
                    // For Flash card, choose color they have most of
                    std::map<CardColor, int> colorCount;
                    for (CardColor c : {CardColor::RED, CardColor::BLUE, CardColor::GREEN, CardColor::YELLOW}) {
                        colorCount[c] = 0;
                    }
                    
                    // Count non-wild cards of each color
                    for (const Card& handCard : cpuHand) {
                        if (!handCard.isWildCard()) {
                            colorCount[handCard.color]++;
                        }
                    }
                    
                    // Choose color with most cards
                    CardColor bestColor = CardColor::RED;
                    int maxCount = -1;
                    for (auto& pair : colorCount) {
                        if (pair.second > maxCount) {
                            maxCount = pair.second;
                            bestColor = pair.first;
                        }
                    }
                    randomColor = bestColor;
                }
                
                playCard(currentPlayerIndex, card, randomColor);
                std::cout << cpuPlayer->getName() << " played: " << card.getColorString() 
                          << " " << card.getValueString() << " (Chose: " << getColorString(randomColor) << ")" << std::endl;
                
                return; // Return after playing wild card
                
            } else {
                // Normal card play (including Package card)
                playCard(currentPlayerIndex, card);
                std::cout << cpuPlayer->getName() << " played: " << card.getColorString() 
                          << " " << card.getValueString() << std::endl;
                
                return;
            }
        }
    }
    
    // If no playable card, draw a card
    Card drawnCard = drawCard(currentPlayerIndex);
    std::cout << cpuPlayer->getName() << " drew a card" << std::endl;
    
    // Check if the drawn card can be played immediately
    if (canPlayCard(currentPlayerIndex, drawnCard)) {
        if (drawnCard.isWildCard()) {
            CardColor randomColor = static_cast<CardColor>(std::uniform_int_distribution<>(0, 3)(rng));
            playCard(currentPlayerIndex, drawnCard, randomColor);
            std::cout << cpuPlayer->getName() << " played the drawn card: " 
                      << drawnCard.getColorString() << " " << drawnCard.getValueString() 
                      << " (Chose: " << getColorString(randomColor) << ")" << std::endl;
        } else {
            playCard(currentPlayerIndex, drawnCard);
            std::cout << cpuPlayer->getName() << " played the drawn card: " 
                      << drawnCard.getColorString() << " " << drawnCard.getValueString() << std::endl;
        }
    }
}

// Skill implementations
std::vector<Card> GameEngine::useLuckyStarSkill(int playerId) {
    if (!canUseSkill(playerId)) return {};
    
    // Peek at top 3 cards
    std::vector<Card> topCards = peekTopCards(3);
    
    // Remove the top 3 cards from draw pile (since we're peeking at them)
    for (int i = 0; i < 3 && !drawPile.empty(); i++) {
        drawPile.pop_back();
    }
    
    useSkill(playerId);
    return topCards;
}

bool GameEngine::completeLuckyStarSkill(int playerId, int cardIndex, const std::vector<Card>& shownCards) {
    if (playerId < 0 || playerId >= players.size()) return false;
    if (cardIndex < 0 || cardIndex >= shownCards.size()) return false;
    
    // Add selected card to player's hand
    Card selectedCard = shownCards[cardIndex];
    players[playerId]->addCard(selectedCard);
    
    // Return the other two cards to the draw pile (on top)
    for (int i = shownCards.size() - 1; i >= 0; i--) {
        if (i != cardIndex) {
            drawPile.push_back(shownCards[i]);
        }
    }
    
    std::cout << players[playerId]->getName() << " used Lucky Star and chose: " 
              << selectedCard.getColorString() << " " << selectedCard.getValueString() << std::endl;
    
    return true;
}

Card GameEngine::useCollectorSkill(int playerId, int cardIndex) {
    if (!canUseSkill(playerId) || discardPile.empty()) return Card();
    
    // Check if cardIndex is valid
    if (cardIndex < 0 || cardIndex >= discardPile.size()) return Card();
    
    Card selectedCard = discardPile[cardIndex];
    
    // Check if card was discarded this turn (Collector restriction)
    for (const Card& recentDiscard : currentTurnDiscards) {
        if (recentDiscard == selectedCard) {
            std::cout << "Cannot select cards just discarded this turn!" << std::endl;
            return Card();
        }
    }
    
    // Add to player's hand
    players[playerId]->addCard(selectedCard);
    
    // Remove from discard pile (but keep the order for others)
    discardPile.erase(discardPile.begin() + cardIndex);
    
    // Reveal to all players
    std::cout << players[playerId]->getName() << " collected: " 
              << selectedCard.getColorString() << " " << selectedCard.getValueString() 
              << " from discard pile!" << std::endl;
    
    useSkill(playerId);
    return selectedCard;
}

bool GameEngine::useThiefSkill(int playerId, int targetPlayerId, bool stealActionCards) {
    if (!canUseSkill(playerId) || targetPlayerId == playerId || 
        targetPlayerId < 0 || targetPlayerId >= players.size()) {
        return false;
    }
    
    // Check if target is Defender and can defend
    if (handleDefenderSkill(playerId, targetPlayerId)) {
        std::cout << players[targetPlayerId]->getName() << " (Defender) blocked the Thief skill!" << std::endl;
        useSkill(playerId); // Original skill user still uses their skill
        return false; // Skill was blocked
    }
    
    Player* thief = players[playerId].get();
    Player* target = players[targetPlayerId].get();
    
    // Check if target has the specified card type
    bool targetHasCards = stealActionCards ? target->hasActionCards() : target->hasNumberCards();
    if (!targetHasCards) {
        useSkill(playerId); // Skill fails but still counts as used
        return false;
    }
    
    // Get cards of specified type from target
    std::vector<Card> targetCards;
    std::vector<Card> targetHand = target->getHand();
    for (const Card& card : targetHand) {
        if (stealActionCards && card.isActionCard()) {
            targetCards.push_back(card);
        } else if (!stealActionCards && card.getCardType() == "NUMBER") {
            targetCards.push_back(card);
        }
    }
    
    // Randomly select one card to steal
    if (!targetCards.empty()) {
        int randomIndex = std::uniform_int_distribution<>(0, targetCards.size() - 1)(rng);
        Card stolenCard = targetCards[randomIndex];
        
        // Remove from target and add to thief
        target->removeCard(stolenCard);
        thief->addCard(stolenCard);
        
        // Thief returns a random card to target
        std::vector<Card> thiefHand = thief->getHand();
        if (!thiefHand.empty()) {
            int returnIndex = std::uniform_int_distribution<>(0, thiefHand.size() - 1)(rng);
            Card returnedCard = thiefHand[returnIndex];
            thief->removeCard(returnedCard);
            target->addCard(returnedCard);
        }
        
        useSkill(playerId);
        return true;
    }
    
    useSkill(playerId);
    return false;
}

bool GameEngine::handleDefenderSkill(int attackerId, int defenderId) {
    if (defenderId < 0 || defenderId >= players.size()) return false;
    
    Player* defender = players[defenderId].get();
    
    // Check if defender has Defender character and skill is available
    if (defender->getCharacter() != CharacterType::DEFENDER || !defender->canUseSkill()) {
        return false;
    }
    
    // Defender successfully defends - use their skill
    defender->useSkill();
    defender->revealDefender();
    
    std::cout << ">>> " << defender->getName() << " revealed as Defender and blocked the skill! <<<" << std::endl;
    return true;
}

bool GameEngine::usePackageSkill(int playerId, CardColor packageCardColor) {
    if (playerId < 0 || playerId >= players.size()) return false;
    
    Player* player = players[playerId].get();
    std::vector<Card> hand = player->getHand();
    std::vector<Card> cardsToDiscard;
    
    // Use Package card's own color, no need to choose
    CardColor color = packageCardColor;
    
    // Find all NUMBER cards of the specified color (excluding action cards)
    for (const Card& card : hand) {
        if (card.color == color && card.getCardType() == "NUMBER") {
            cardsToDiscard.push_back(card);
        }
    }
    
    // If found matching cards, remove them from hand and add to discard pile
    if (!cardsToDiscard.empty()) {
        for (const Card& card : cardsToDiscard) {
            player->removeCard(card);
            discardPile.push_back(card);
        }
        
        std::cout << players[playerId]->getName() << " discarded " << cardsToDiscard.size() 
                  << " " << getColorString(color) << " number cards using Package skill!" << std::endl;
        return true;
    } else {
        std::cout << "No " << getColorString(color) << " number cards to discard!" << std::endl;
        return false;
    }
}

std::vector<Card> GameEngine::getDiscardPileForCollector() const {
    return discardPile;
}

std::vector<std::string> GameEngine::getPlayerNames() const {
    std::vector<std::string> names;
    for (const auto& player : players) {
        names.push_back(player->getName());
    }
    return names;
}