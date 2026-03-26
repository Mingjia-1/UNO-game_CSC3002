#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include "player.h"
#include "card.h"
#include <vector>
#include <memory>
#include <random>

class GameEngine {
private:
    std::vector<std::unique_ptr<Player>> players;
    std::vector<Card> drawPile;
    std::vector<Card> discardPile;
    int currentPlayerIndex;
    bool gameDirection;
    bool gameStarted;
    std::mt19937 rng;
    CardColor chosenWildColor;
    CardColor currentPlayColor;

    // Flash card tracking
    bool flashEffectActive;
    CardColor flashColor;
    int flashChainCount;
    int flashStartingPlayer;

    // Skill restrictions
    std::vector<Card> currentTurnDiscards;
    int currentRound;

    void initializeDeck();
    void shuffleDeck();
    void dealInitialCards();
    void assignCharacters();
    Card drawFromDeck();
    void handleActionCard(const Card& card);
    void handleWildCard(const Card& card, CardColor chosenColor);
    void handleFlashEffect(int playerId);
    
public:
    GameEngine();
    
    void addPlayer(const std::string& name);
    void startGame();
    bool isGameStarted() const { return gameStarted; }
    
    Card getCurrentCard() const;
    std::vector<Card> getPlayerHand(int playerId) const;
    std::vector<std::string> getPlayerCharacters() const;
    std::vector<int> getPlayerCardCounts() const;
    int getCurrentPlayerIndex() const { return currentPlayerIndex; }
    std::string getCurrentPlayerName() const;
    
    int getNextPlayerIndex() const;
    int getCurrentRound() const { return currentRound; }
    void incrementRound() { currentRound++; }
    
    bool canPlayCard(int playerId, const Card& card) const;
    bool playCard(int playerId, const Card& card, CardColor chosenColor = CardColor::RED);
    
    bool canPlayMultipleCards(int playerId, const std::vector<Card>& cards) const;
    bool playMultipleCards(int playerId, const std::vector<Card>& cards, CardColor chosenColor = CardColor::RED);
    
    Card drawCard(int playerId);
    void nextTurn();
    void advanceToNextPlayer();
    
    bool canUseSkill(int playerId) const;
    void useSkill(int playerId);
    void resetSkillCooldowns();
    
    std::vector<Card> peekTopCards(int count);
    std::vector<Card> getDiscardPile() const { return discardPile; }
    bool isGameOver() const;
    int getWinner() const;
    
    void setChosenWildColor(CardColor color) { chosenWildColor = color; }
    CardColor getChosenWildColor() const { return chosenWildColor; }
    CardColor getCurrentPlayColor() const { return currentPlayColor; }
    void setCurrentPlayColor(CardColor color) { currentPlayColor = color; }
    
    void playCPUTurn();

    // Skill methods
    std::vector<Card> useLuckyStarSkill(int playerId);
    Card useCollectorSkill(int playerId, int cardIndex);
    bool useThiefSkill(int playerId, int targetPlayerId, bool stealActionCards);
    bool handleDefenderSkill(int attackerId, int defenderId);
    bool usePackageSkill(int playerId, CardColor packageCardColor);
    
    std::vector<Card> getDiscardPileForCollector() const;
    std::vector<std::string> getPlayerNames() const;
    bool completeLuckyStarSkill(int playerId, int cardIndex, const std::vector<Card>& shownCards);
    
    // Flash effect methods
    bool isFlashEffectActive() const { return flashEffectActive; }
    CardColor getFlashColor() const { return flashColor; }
    int getFlashChainCount() const { return flashChainCount; }
    void endFlashEffect();
};

#endif