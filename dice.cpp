#include "dice.h"

dice::dice(std::string s)
{
    std::string sBase, sNumDice, sSides;
    int start = 0;
    int length = s.find('+');
    sBase = s.substr(start, length);
    start = length + 1;
    length = s.find('d', start) - start;

    sNumDice = s.substr(start, length);
    start += length + 1;
    length = s.length() - start;

    sSides = s.substr(start, length);

    base = std::stoi(sBase);
    numDice = std::stoi(sNumDice);
    sides = std::stoi(sSides);
}

std::string dice::toString() const
{
    std::string output = std::to_string(base) + "+" + std::to_string(numDice) + "d" + std::to_string(sides);
    return output;
}

void dice::set(int base, int number, int sides)
{
    this->base = base;
    numDice = number;
    this->sides = sides;
}

int dice::roll() const
{
    int output = base;
    int i;
    for(i = 0; i < numDice; i++) {
        output += randomInt(1, sides);
    }
    return output;
}

std::ostream& dice::print(std::ostream& o)
{
    return o << base << '+' << numDice << 'd' << sides;
}

std::ostream& operator<<(std::ostream& o, dice& d)
{
    return d.print(o);
}
