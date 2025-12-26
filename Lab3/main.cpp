#include "codec.h"
#include "terminal.h"
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <cctype>
#include "main.h"
#include "new.h"
#include "old.h"

void printImage(std::string path)
{
    std::ifstream f(path, std::ios::binary);
    std::vector<uint8_t> d((std::istreambuf_iterator<char>(f)),
                           std::istreambuf_iterator<char>());
    image img = Decode(d, false);
    renderToTerminal(img);
}
void pk()
{
    std::cout << "Нажмите на любую кнопку, чтобы продолжить..." << std::endl;
    char a;
    std::cin >> a;
}
void smesharik_select()
{
    while (true)
    {
        std::cout << "Введите цифру от 1 до 7, 0 для выхода" << std::endl;
        int n;
        std::cin >> n;

        switch (n)
        {
        case 0:
            return;
        case 1:
        {
            std::unique_ptr<Newer> krosh_via_newer = std::make_unique<Krosh>();
            (*krosh_via_newer).speak();
            pk();
            (*krosh_via_newer).IsPlaying();
            continue;
        }
        case 2:
        {
            Yojik yojik_via_newer = Yojik();
            yojik_via_newer.speak();
            pk();
            yojik_via_newer.IsPlaying();
            continue;
        }
        case 3:
        {
            Karich smesharik = Karich();
            smesharik.speak();
            pk();
            smesharik.giveAdvice();
            pk();
            smesharik.fly();
            continue;
        }
        case 4:
        {
            Pin smesharik = Pin();
            smesharik.speak();
            pk();
            smesharik.giveAdvice();
            continue;
        }
        case 5:
        {
            Sovuniya smesharik = Sovuniya();
            smesharik.speak();
            pk();
            smesharik.giveAdvice();
            continue;
        }
        case 6:
        {
            Losyash smesharik = Losyash();
            smesharik.speak();
            pk();
            smesharik.giveAdvice();
            continue;
        }
        case 7:
        {
            Kopatich smesharik = Kopatich();
            smesharik.speak();
            pk();
            smesharik.giveAdvice();
            continue;
        }
        default:
            continue;
        }
    }
}
int main()
{
    std::cout << "Привет! Сегодня ты познакомишься с миром смешариков!\n";
    printImage("./images/zas1.babe");
    smesharik_select();
}
