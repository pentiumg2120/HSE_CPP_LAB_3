#include "new.h"
#include "main.h"

Newer::Newer(const std::string &n) : Smesharik(n) {}

void Newer::IsPlaying() { std::cout << name << "is playing\n"; }

Krosh::Krosh() : Newer("Krosh") {}

void Krosh::speak()
{
    std::cout << "I like to eat carrots\n";
    printImage("./images/kar3.babe");
}
void Krosh::IsPlaying() { std::cout << name << "is playing\n"; }
Krosh::~Krosh() {}

Yojik::Yojik() : Newer("Yojik") {}

void Yojik::speak()
{
    std::cout << "I collect wonderful candy wrappers\n";
    printImage("./images/ezh1.babe");
}
void Yojik::IsPlaying() { std::cout << name << "is playing\n"; }
Yojik::~Yojik() {}
