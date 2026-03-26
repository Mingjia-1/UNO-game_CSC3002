#ifndef CARD_H
#define CARD_H

#include <string>

enum class CardColor { RED, YELLOW, GREEN, BLUE, WILD };
enum class CardValue { 
    ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE,
    SKIP, REVERSE, DRAW_TWO, PACKAGE,
    WILD, WILD_DRAW_FOUR, FLASH
};

class Card {
public:
    CardColor color;
    CardValue value;
    
    Card(CardColor c = CardColor::RED, CardValue v = CardValue::ONE);
    std::string getColorString() const;
    std::string getValueString() const;
    std::string getCardType() const;
    int getCardNumber() const;
    bool canPlayOn(const Card& other) const;
    bool isActionCard() const;
    bool isWildCard() const;
    bool isPackageCard() const;
    bool isFlashCard() const;
    bool operator==(const Card& other) const;
};

#endif