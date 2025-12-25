#ifndef NEW_H
#define NEW_H

#include "shari.h"
#include <iostream>
#include <string>


class Newer : public Smesharik {
public:
    Newer(const std::string& n) : Smesharik(n) {}

    void IsPlaying() {std::cout <<  name << "is playing\n";}
};


class Krosh: public Newer {
public:
    Krosh() : Newer("Krosh") {}

    void speak() override {
        std::cout << "I like to eat carrots\n";
        // место для вывода картинки
    }
    void IsPlaying() {std::cout <<  name << "is playing\n";}
    ~Krosh() {}
};


class Yojik: public Newer {
    public:
        Yojik() : Newer("Yojik") {}

        void speak() override {
            std::cout << "I collect wonderful candy wrappers\n";
            // место для вывода картинки
        }
        void IsPlaying() {std::cout <<  name << "is playing\n";}
        ~Yojik() {}
};
    

#endif