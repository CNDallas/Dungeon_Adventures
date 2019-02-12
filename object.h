#ifndef OBJECT_H
#define OBJECT_H

#include "dice.h"
#include "object_descriptions.h"
#include <string>

class object
{
private:
    const std::string* name;
    const std::string* desc;
    object_type_t type;
    int xLoc, yLoc;
    int color;
    int hitBonus;
    dice damageBonus;
    int dodgeBonus;
    int defenseBonus;
    int weight;
    int speed;
    int characteristics;
    bool artifact;
    int value;

public:
    object(object_description* o, int y, int x)
    {
        name = &o->get_name();
        desc = &o->get_description();
        type = o->get_type();
        color = o->get_color();
        hitBonus = o->get_hit().roll();
        damageBonus = o->get_damage();
        dodgeBonus = o->get_dodge().roll();
        defenseBonus = o->get_defense().roll();
        weight = o->get_weight().roll();
        speed = o->get_speed().roll();
        characteristics = o->get_characteristics().roll();
        value = o->get_value().roll();
        yLoc = y;
        xLoc = x;
        artifact = o->get_artifact();
    }

    const std::string get_name() const
    {
        return *name;
    }

    const std::string get_description() const
    {
        return *desc;
    }

    object_type_t get_type() const
    {
        return type;
    }

    int get_color() const
    {
        return color;
    }

    int get_hitBonus() const
    {
        return hitBonus;
    }

    dice get_damage() const
    {
        return damageBonus;
    }

    int get_dodge() const
    {
        return dodgeBonus;
    }

    int get_defense() const
    {
        return defenseBonus;
    }

    int get_weight() const
    {
        return weight;
    }

    int get_speed() const
    {
        return speed;
    }

    int get_characteristics() const
    {
        return characteristics;
    }

    int get_value() const
    {
        return value;
    }

    bool get_artifact() const
    {
        return artifact;
    }

    char get_object_symbol() const
    {
        return object_symbol[type];
    }
};

void generateObject(dungeon_t* d, std::vector<std::string>* artifacts);

void generateAllObjects(dungeon_t* d);

void printObjects(dungeon_t* d);

void printObjectBoard(dungeon_t* d);

void clearObjects(dungeon_t* d);

#endif // OBJECTS_H
