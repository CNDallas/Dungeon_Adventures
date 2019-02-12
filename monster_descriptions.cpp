#include "monster_descriptions.h"
#include "dungeon.h"

int getMonstersFromFile(dungeon_t* d)
{
    std::ifstream f;
    std::string path = getenv("HOME");
    path += "/.rlg327/monster_desc.txt";
    f.open(path);
    std::string version = "RLG327 MONSTER DESCRIPTION 1";
    std::string versionCheck;
    std::getline(f, versionCheck);
    if(versionCheck != version) {
        std::cout << "File isn't the correct version" << std::endl;
        return -1;
    }
    while(!f.eof()) {
        std::string begin = "BEGIN MONSTER";
        std::string beginCheck;
        while(begin != beginCheck) {
            std::getline(f, beginCheck);
            if(f.eof()) {
                break;
            }
        }
        bool name, desc, color, speed, abil, hp, dam, symb, rrty;
        name = desc = color = speed = abil = hp = dam = symb = rrty = false;
        std::string mons_name, mons_desc;
        dice mons_speed, mons_dam, mons_hp;
        char mons_symb;
        unsigned int mons_abil, mons_rrty;
        std::vector<int> mons_color;
        mons_name = "";
        mons_desc = "";
        mons_color = {};
        mons_speed = {};
        mons_dam = {};
        mons_hp = {};
        mons_symb = ' ';
        mons_abil = mons_rrty = 0;
        mons_abil += 0;

        bool end = false;
        while(!end) {
            std::string keyword;
            f >> keyword;
            if(keyword.compare("NAME") == 0) {
                if(!name) {
                    f.get();
                    std::getline(f, mons_name);
                    name = true;
                } else {
                    end = true;
                }

            } else if(keyword.compare("SYMB") == 0) {
                if(!symb) {
                    f.get();
                    mons_symb = f.get();
                    f.ignore();
                    symb = true;
                } else {
                    end = true;
                }

            } else if(keyword.compare("COLOR") == 0) {
                if(!color) {
                    while(f.get() != '\n') {
                        std::string temp;
                        f >> temp;
                        int i;
                        for(i = 0; colors_lookup[i].name; i++) {
                            if(temp == colors_lookup[i].name) {
                                mons_color.push_back(colors_lookup[i].value);
                                break;
                            }
                        }
                    }
                    color = true;
                } else {
                    end = true;
                }

            } else if(keyword.compare("DESC") == 0) {
                if(!desc) {
                    std::string temp;
                    while(std::getline(f, temp) && temp.compare(".") != 0) {
                        mons_desc.append(temp);
                        mons_desc.append("\n");
                    }
                    desc = true;
                } else {
                    end = true;
                }

            } else if(keyword.compare("SPEED") == 0) {
                if(!speed) {
                    f.get();
                    std::string temp;
                    std::getline(f, temp);
                    mons_speed = dice(temp);
                    speed = true;
                } else {
                    end = true;
                }
            } else if(keyword.compare("DAM") == 0) {
                if(!dam) {
                    f.get();
                    std::string temp;
                    std::getline(f, temp);
                    mons_dam = dice(temp);
                    dam = true;
                } else {
                    end = true;
                }

            } else if(keyword.compare("HP") == 0) {
                if(!hp) {
                    f.get();
                    std::string temp;
                    std::getline(f, temp);
                    mons_hp = dice(temp);
                    hp = true;
                } else {
                    end = true;
                }

            } else if(keyword.compare("RRTY") == 0) {
                if(!rrty) {
                    f.get();
                    std::string temp;
                    std::getline(f, temp);
                    mons_rrty = stoi(temp);
                    rrty = true;
                } else {
                    end = true;
                }

            } else if(keyword.compare("ABIL") == 0) {
                if(!abil) {
                    while(f.get() != '\n') {
                        std::string temp;
                        f >> temp;
                        if(temp == "SMART") {
                            mons_abil |= (1 << 0);
                        } else if(temp == "TELE") {
                            mons_abil |= (1 << 1);
                        } else if(temp == "TUNNEL") {
                            mons_abil |= (1 << 2);
                        } else if(temp == "ERRATIC") {
                            mons_abil |= (1 << 3);
                        } else if(temp == "PASS") {
                            mons_abil |= (1 << 4);
                        } else if(temp == "PICKUP") {
                            mons_abil |= (1 << 5);
                        } else if(temp == "DESTROY") {
                            mons_abil |= (1 << 6);
                        } else if(temp == "UNIQ") {
                            mons_abil |= (1 << 7);
                        } else if(temp == "BOSS") {
                            mons_abil |= (1 << 8);
                        }
                    }
                    abil = true;
                } else {
                    end = true;
                }

            } else if(keyword.compare("END") == 0) {
                if(name && desc && color && speed && abil && hp && dam && symb && rrty) {
                    monster_description m = monster_description(mons_name, mons_desc, mons_color, mons_symb, mons_abil,
                        mons_rrty, mons_speed, mons_dam, mons_hp);
                    d->monster_descriptions.push_back(std::move(m));
                    end = true;
                } else {
                    end = true;
                }

            } else {
                end = true;
            }
        }
    }
    f.close();
    return 0;
}

int monster_description::monsterDescPrint()
{

    std::cout << "BEGIN MONSTER" << std::endl;
    std::cout << "NAME " + monsters_name << std::endl;
    std::cout << "DESC" + desc + "." << std::endl;
    std::cout << "COLOR ";
    for(int s : color) {
        std::string output;
        switch(s) {
        case 0:
            output = "BLACK ";
            break;
        case 1:
            output = "BLUE ";
            break;
        case 2:
            output = "CYAN ";
            break;
        case 3:
            output = "GREEN ";
            break;
        case 4:
            output = "MAGENTA ";
            break;
        case 5:
            output = "RED ";
            break;
        case 6:
            output = "WHITE ";
            break;
        case 7:
            output = "YELLOW ";
            break;
        default:
            output = "UNKNOWN COLOR ";
        }
        std::cout << output;
    }
    std::cout << "\n";
    std::cout << "SPEED " + speed.toString() << std::endl;
    std::cout << "ABIL " + getAbilitiesString() << std::endl;
    std::cout << "HP " + hp.toString() << std::endl;
    std::cout << "DAM " + damage.toString() << std::endl;
    std::cout << "SYMB ";
    std::cout << symbol << std::endl;
    std::cout << "RRTY " + std::to_string(rarity) << std::endl;
    std::cout << "END" << std::endl;
    return 0;
}

std::string monster_description::getAbilitiesString()
{
    std::string abilitiesString;
    if(bitChecker(abilities, 0)) {
        abilitiesString.append("SMART ");
    }
    if(bitChecker(abilities, 1)) {
        abilitiesString.append("TELE ");
    }
    if(bitChecker(abilities, 2)) {
        abilitiesString.append("TUNNEL ");
    }
    if(bitChecker(abilities, 3)) {
        abilitiesString.append("ERRATIC ");
    }
    if(bitChecker(abilities, 4)) {
        abilitiesString.append("PASS ");
    }
    if(bitChecker(abilities, 5)) {
        abilitiesString.append("PICKUP ");
    }
    if(bitChecker(abilities, 6)) {
        abilitiesString.append("DESTROY ");
    }
    if(bitChecker(abilities, 7)) {
        abilitiesString.append("UNIQ ");
    }
    if(bitChecker(abilities, 8)) {
        abilitiesString.append("BOSS ");
    }
    return abilitiesString;
}
