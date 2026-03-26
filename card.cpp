#include "card.h"
#include <stdexcept>
#include <iostream>

Card::Card(CardColor c, CardValue v) : color(c), value(v) {}

std::string Card::getColorString() const {
    switch(color) {
        case CardColor::RED: return "RED";
        case CardColor::YELLOW: return "YELLOW";
        case CardColor::GREEN: return "GREEN";
        case CardColor::BLUE: return "BLUE";
        case CardColor::WILD: 
            if (value == CardValue::FLASH) return "FLASH";
            else if (value == CardValue::WILD_DRAW_FOUR) return "WILD_DRAW4";
            else return "WILD";
        default: return "UNKNOWN";
    }
}

std::string Card::getValueString() const {
    switch(value) {
        case CardValue::ZERO: return "0";
        case CardValue::ONE: return "1";
        case CardValue::TWO: return "2";
        case CardValue::THREE: return "3";
        case CardValue::FOUR: return "4";
        case CardValue::FIVE: return "5";
        case CardValue::SIX: return "6";
        case CardValue::SEVEN: return "7";
        case CardValue::EIGHT: return "8";
        case CardValue::NINE: return "9";
        case CardValue::SKIP: return "SKIP";
        case CardValue::REVERSE: return "REVERSE";
        case CardValue::DRAW_TWO: return "DRAW_TWO";
        case CardValue::PACKAGE: return "PACKAGE";
        case CardValue::WILD: return "WILD";
        case CardValue::WILD_DRAW_FOUR: return "WILD_DRAW_FOUR";
        case CardValue::FLASH: return "FLASH";
        default: 
            std::cout << "DEBUG: Unknown card value: " << static_cast<int>(value) << std::endl;
            return "?";
    }
}

std::string Card::getCardType() const {
    if (value >= CardValue::ZERO && value <= CardValue::NINE) {
        return "NUMBER";
    } else if (value >= CardValue::SKIP && value <= CardValue::PACKAGE) {
        return "ACTION";
    } else {
        return "WILD";
    }
}

int Card::getCardNumber() const {
    if (value >= CardValue::ZERO && value <= CardValue::NINE) {
        return static_cast<int>(value); // 直接返回枚举的整数值
    }
    // 动作卡片和wild卡片保持原有逻辑
    switch(value) {
        case CardValue::REVERSE: return 11;
        case CardValue::SKIP: return 12;
        case CardValue::DRAW_TWO: return 13;
        case CardValue::PACKAGE: return 14;
        default: return 0; // Wild卡片
    }
}

bool Card::canPlayOn(const Card& other) const {
    // Wild cards can always be played
    if (isWildCard()) {
        return true;
    }
    
    // Package cards can be played on other package cards
    if (isPackageCard() && other.isPackageCard()) {
        return true;
    }
    
    // Same color
    if (color == other.color) {
        return true;
    }
    
    // Same value (number or action)
    if (value == other.value) {
        return true;
    }
    
    return false;
}

bool Card::isActionCard() const {
    return value == CardValue::SKIP || value == CardValue::REVERSE || 
           value == CardValue::DRAW_TWO || value == CardValue::PACKAGE;
}

bool Card::isWildCard() const {
    return value == CardValue::WILD || value == CardValue::WILD_DRAW_FOUR || 
           value == CardValue::FLASH;
}

bool Card::isPackageCard() const {
    return value == CardValue::PACKAGE;
}

bool Card::isFlashCard() const {
    return value == CardValue::FLASH;
}

bool Card::operator==(const Card& other) const {
    return color == other.color && value == other.value;
}