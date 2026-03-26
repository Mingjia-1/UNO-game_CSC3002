#include "player.h"
#include <algorithm>

Player::Player(int id, const std::string& playerName) 
    : playerId(id), name(playerName), character(CharacterType::LUCKY_STAR),
      skillAvailable(true), skillCooldown(false), skillUsesRemaining(0),
      maxSkillUses(0), defenderRevealed(false),
      roundsPlayed(0), lastSkillUseRound(-1) {}

void Player::addCard(const Card& card) {
    hand.push_back(card);
}

bool Player::removeCard(const Card& card) {
    auto it = std::find(hand.begin(), hand.end(), card);
    if (it != hand.end()) {
        hand.erase(it);
        return true;
    }
    return false;
}

bool Player::hasCard(const Card& card) const {
    return std::find(hand.begin(), hand.end(), card) != hand.end();
}

void Player::setCharacter(CharacterType charType) {
    character = charType;
    skillAvailable = true;
    skillCooldown = false;
    defenderRevealed = false;
    roundsPlayed = 0;
    lastSkillUseRound = -1;
    
    switch(charType) {
        case CharacterType::LUCKY_STAR:
            skillUsesRemaining = 3;
            maxSkillUses = 3;
            break;
        case CharacterType::COLLECTOR:
        case CharacterType::THIEF:
        case CharacterType::DEFENDER:
            skillUsesRemaining = 1;
            maxSkillUses = 1;
            break;
        default:
            skillUsesRemaining = 0;
            maxSkillUses = 0;
            break;
    }
}

std::string Player::getCharacterString() const {
    switch(character) {
        case CharacterType::LUCKY_STAR: return "Lucky Star";
        case CharacterType::COLLECTOR: return "Collector";
        case CharacterType::THIEF: return "Thief";
        case CharacterType::DEFENDER: return "Defender";
        default: return "Unknown";
    }
}

bool Player::canUseSkill() const {
    switch(character) {
        case CharacterType::LUCKY_STAR:
            // Lucky Star: can use if hasn't reached limit and not used in current round
            return skillUsesRemaining > 0 && (roundsPlayed > lastSkillUseRound);
        case CharacterType::DEFENDER:
            // Defender: passive skill, cannot be actively used
            return false;
        default:
            // Collector and Thief: available if not on cooldown and has uses remaining
            return skillAvailable && !skillCooldown && skillUsesRemaining > 0;
    }
}

void Player::useSkill() {
    if (skillUsesRemaining > 0) {
        skillUsesRemaining--;
        lastSkillUseRound = roundsPlayed;
        
        // Only other characters (not Lucky Star) have immediate cooldown
        if (character != CharacterType::LUCKY_STAR) {
            skillCooldown = true;
        }
        
        if (skillUsesRemaining <= 0) {
            skillAvailable = false;
        }
    }
}

void Player::resetSkillCooldown() {
    if (skillCooldown && skillUsesRemaining > 0) {
        skillCooldown = false;
        // Lucky Star doesn't reset cooldown here - it's round-based
    }
}

std::vector<Card> Player::getCardsByColor(CardColor color) const {
    std::vector<Card> result;
    for (const auto& card : hand) {
        if (card.color == color) {
            result.push_back(card);
        }
    }
    return result;
}

bool Player::hasColorCard(CardColor color) const {
    for (const auto& card : hand) {
        if (card.color == color) {
            return true;
        }
    }
    return false;
}

bool Player::hasActionCards() const {
    for (const auto& card : hand) {
        if (card.isActionCard()) {
            return true;
        }
    }
    return false;
}

bool Player::hasNumberCards() const {
    for (const auto& card : hand) {
        if (card.getCardType() == "NUMBER") {
            return true;
        }
    }
    return false;
}