#ifndef MONSTER_DESCRIPTIONS_H
#define MONSTER_DESCRIPTIONS_H

#include "dice.h"
#include "utilities.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

enum abilities_bit_t {
    abilities_SMART,
    abilities_TELE,
    abilities_TUNNEL,
    abilities_ERRATIC,
    abilities_PASS,
    abilities_PICKUP,
    abilities_DESTROY,
    abilities_UNIQ,
    abilities_BOSS
};

class dungeon_t;

class monster_description
{
private:
    std::string monsters_name;
    std::string desc;
    std::vector<int> color;
    char symbol;
    unsigned int abilities, rarity;
    dice speed, damage, hp;

public:
    monster_description(const std::string& monsters_name,
        const std::string& desc,
        const std::vector<int>& color,
        const char symbol,
        const unsigned int abilities,
        const unsigned int rarity,
        const dice& speed,
        const dice& damage,
        const dice& hp)
        : monsters_name(monsters_name)
        , desc(desc)
        , color(color)
        , symbol(symbol)
        , abilities(abilities)
        , rarity(rarity)
        , speed(speed)
        , damage(damage)
        , hp(hp)
    {
    }
    monster_description()       
        : monsters_name()
        , desc()        
        , color(0)
        , symbol()
        , abilities(0) 
        , rarity(0)
        , speed() 
        , damage() 
        , hp()
    {
    }
    

    const std::string& getMonstersName() const
    {
        return monsters_name;
    }

    char getSymbol() const
    {
        return symbol;
    }

    const std::vector<int> getColor() const
    {
        return color;
    }

    int getFirstColor() const
    {
        return color[0];
    }

    const std::string& getDescription() const
    {
        return desc;
    }

    const dice& getSpeed() const
    {
        return speed;
    }

    const dice& getDamage() const
    {
        return damage;
    }

    const dice& getHitPoints() const
    {
        return hp;
    }

    unsigned int getRarity() const
    {
        return rarity;
    }

    unsigned int getAbilities() const
    {
        return abilities;
    }

    std::string getAbilitiesString();

    int monsterDescPrint();
};

int getMonstersFromFile(dungeon_t* d);

#endif // DESCRIPTIONS_H
