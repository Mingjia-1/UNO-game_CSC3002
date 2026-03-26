#ifndef HELPERS_H
#define HELPERS_H

#include "card.h"
#include <string>

// Inline function to avoid multiple definition errors
inline std::string getColorString(CardColor color) {
    switch(color) {
        case CardColor::RED: return "RED";
        case CardColor::BLUE: return "BLUE";
        case CardColor::GREEN: return "GREEN";
        case CardColor::YELLOW: return "YELLOW";
        default: return "WILD";
    }
}

#endif