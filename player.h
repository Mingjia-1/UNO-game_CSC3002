#ifndef PLAYER_H
#define PLAYER_H

#include "card.h"
#include <vector>
#include <string>

enum class CharacterType { LUCKY_STAR, COLLECTOR, THIEF, DEFENDER };

class Player {
private:
    int playerId;
    std::string name;
    std::vector<Card> hand;
    CharacterType character;
    bool skillAvailable;
    bool skillCooldown;
    int skillUsesRemaining;
    int maxSkillUses;
    bool defenderRevealed;
    
    // Round tracking for skill cooldowns
    int roundsPlayed;
    int lastSkillUseRound;

public:
    Player(int id, const std::string& playerName);
    
    int getId() const { return playerId; }
    std::string getName() const { return name; }
    std::vector<Card> getHand() const { return hand; }
    CharacterType getCharacter() const { return character; }
    std::string getCharacterString() const;
    bool canUseSkill() const;
    int getSkillUsesRemaining() const { return skillUsesRemaining; }
    int getMaxSkillUses() const { return maxSkillUses; }
    
    // Round tracking methods
    void incrementRound() { roundsPlayed++; }
    int getRoundsPlayed() const { return roundsPlayed; }
    int getLastSkillUseRound() const { return lastSkillUseRound; }
    void setLastSkillUseRound(int round) { lastSkillUseRound = round; }
    
    void addCard(const Card& card);
    bool removeCard(const Card& card);
    bool hasCard(const Card& card) const;
    int getHandSize() const { return hand.size(); }
    
    void setCharacter(CharacterType charType);
    void useSkill();
    void resetSkillCooldown();
    void revealDefender() { defenderRevealed = true; }
    bool isDefenderRevealed() const { return defenderRevealed; }
    
    std::vector<Card> getCardsByColor(CardColor color) const;
    bool hasColorCard(CardColor color) const;
    bool hasActionCards() const;
    bool hasNumberCards() const;
};

#endif