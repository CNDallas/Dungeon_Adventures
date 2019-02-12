#ifndef DICE_H
#define DICE_H

#include "utilities.h"
#include <fstream>
#include <iostream>
#include <string>

class dice
{
private:
    int base, numDice, sides;

public:
    dice()
    {
        this->base = randomInt(0, 6);
        this->numDice = randomInt(0, 6);
        this->sides = randomInt(0, 20);
    }

    dice(int base, int numDice, int sides)
    {
        this->base = base;
        this->numDice = numDice;
        this->sides = sides;
    }

    explicit dice(std::string s);

    std::string toString() const;

    void set_base(int base)
    {
        this->base = base;
    }

    void set_number(int number)
    {
        numDice = number;
    }

    void set_sides(int sides)
    {
        this->sides = sides;
    }

    int get_base() const
    {
        return base;
    }

    int get_number() const
    {
        return numDice;
    }

    int get_sides() const
    {
        return sides;
    }

    void set(int base, int number, int sides);

    int roll() const;

    std::ostream& print(std::ostream& o);
};

std::ostream& operator<<(std::ostream& o, dice& d);

#endif // DICE_H
