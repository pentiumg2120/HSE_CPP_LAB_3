#include "old.h"
#include "main.h"
#include <string>

Older::Older(const std::string &n) : Smesharik(n) {}

void Older::giveAdvice() {}

Karich::Karich() : Older("Karich") {}

void Karich::speak()
{
    std::cout << "I love playing music" << std::endl;
    ;
    printImage("./images/kar2.babe");
}

void Karich::fly()
{
    std::cout << "Нелетающий" << std::endl;
    printImage("./images/kar3.babe");
}

void Karich::giveAdvice() { std::cout << "Хорошо, если в старости всё, чего не хочется вспоминать, умещается в одном чемодане" << std::endl; }
Karich::~Karich() {}

Pin::Pin() : Older("Pin") {}

void Pin::speak()
{
    std::cout << "Yaya! Is compiling! Glupi code" << std::endl;
    printImage("./images/pin1.babe");
}

void Pin::giveAdvice() { std::cout << "Я тебя породить, я тебе и говорить «до свидания»! Auf Wiedersehen!" << std::endl; }
Pin::~Pin() {}

Sovuniya::Sovuniya() : Older("Sovuniya") {}

void Sovuniya::speak()
{
    std::cout << "" << std::endl;
    printImage("./images/sov2.babe");
}

void Sovuniya::giveAdvice() { std::cout << "Чем больше всяких рецептов знаешь, тем правильнее живёшь. В них вся мудрость жизни." << std::endl; }
Sovuniya::~Sovuniya() {}

Losyash::Losyash() : Older("Losyash") {}

void Losyash::speak()
{
    std::cout << "For any epsilon greater than zero there exists N equal to N of epsilon, which belongs to the natural numbers...\n";
    printImage("./images/los3.babe");
}

void Losyash::giveAdvice() { std::cout << "Сердечная привязанность творения — это первое, о чём должен позаботиться истинный творец!" << std::endl; }
Losyash::~Losyash() {}

Kopatich::Kopatich() : Older("Kopatich") {}

void Kopatich::speak()
{
    std::cout << "I grow carrots for Krosh\n";
    printImage("./images/kop2.babe");
}

void Kopatich::giveAdvice()
{
    std::cout << "Вальсы, твисты, ламбады… Мода меняется. Но мое сердце бьется только в ритме диско!" << std::endl;
    printImage("./images/kop3.babe");
}
Kopatich::~Kopatich() {}
