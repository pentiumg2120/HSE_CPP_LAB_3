#include "new.h"
#include "main.h"

Newer::Newer(const std::string &n) : Smesharik(n) {}

void Newer::IsPlaying() { std::cout << name << "is playing\n"; }
void Newer::speak() { std::cout << "MOLODEZHHH" << std::endl; }

Krosh::Krosh() : Newer("Krosh") {}

void Krosh::speak()
{
    std::cout << "I like to eat carrots\n";
    printImage("./images/kro3.babe");
}

Krosh::~Krosh() {}

Yojik::Yojik() : Newer("Yojik") {}

void Yojik::speak()
{
    std::cout << "I collect wonderful candy wrappers\n";
    printImage("./images/ezh1.babe");
}
Yojik::~Yojik() {}
