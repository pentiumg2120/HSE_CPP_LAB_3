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
int main()
{
    std::cout << "Привет! Сегодня ты познакомишься с миром смешариков!\n";
    printImage("./images/1.babe");
    std::cout << "Нажимай Enter, чтобы продолжить...\n";
    char step{};
    std::cin >> step;
    printImage("./images/2.babe");
    std::cout << "Нажимай Enter, чтобы продолжить...\n";
}