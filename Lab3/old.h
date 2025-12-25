#ifndef OLD_H
#define OLD_H

#include "shari.h"
#include <iostream>
#include <string>


class Older : public Smesharik {
public:
    OldSmesharik(const std::string& n) : Smesharik(n) {}

    virtual void giveAdvice() {}
};

class Karich : public Older {
public:
    Karich() : Newer("Karich") {}

    void speak() override {
        std::cout << "I love playing music\n";
        // место для вывода картинки
    }

    void fly() {
        // место для картинки
    }


    void giveAdvice() override {std::cout << " \n";}
    ~Karich() {}
};


class Pin : public Older {
    public:
        Pin() : Newer("Pin") {}
    
        void speak() override {
            std::cout << "Yaya! Is compiling! Glupi code\n";
            // место для вывода картинки
        }
    
        void giveAdvice() override {std::cout << " \n";}
        ~Pin() {}
};

class Sovuniya : public Older {
    public:
        Sovuniya() : Newer("Sovuniya") {}
    
        void speak() override {
            std::cout << "\n";
            // место для вывода картинки
        }
    
        void giveAdvice() override {std::cout << " \n";}
        ~Sovuniya() {}
};

class Losyash : public Older {
    public:
        Losyash() : Newer("Losyash") {}
    
        void speak() override {
            std::cout << "For any epsilon greater than zero there exists N equal to N of epsilon, which belongs to the natural numbers...\n";
            // место для вывода картинки
        }
    
        void giveAdvice() override {std::cout << " \n";}
        ~Losyash() {}
};

class Kopatich : public Older {
    public:
        Kopatich() : Newer("Kopatich") {}
    
        void speak() override {
            std::cout << "I grow carrots for Krosh\n";
            // место для вывода картинки
        }
    
        void giveAdvice() override {std::cout << " \n";}
        ~Kopatich() {}
};

#endif