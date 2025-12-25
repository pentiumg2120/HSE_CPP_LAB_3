#include "codec.h"
#include "terminal.h"
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <cctype>
#include "main.h"

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
    std::cout << "Введите цифру от 1 до 7" << std::endl;
    int n;
    std::cin >> n;

    switch (n)
    {
    case 1:
        smesharik_select();
        break;

    default:
        smesharik_select();
        break;
    }
}
int main()
{
    std::cout << "Привет! Сегодня ты познакомишься с миром смешариков!\n";
    printImage("./images/zas1.babe");
    smesharik_select();
}