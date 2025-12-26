#ifndef SHARI_H
#define SHARI_H

#include <iostream>
#include <string>

class Smesharik
{
protected:
    std::string name;

public:
    Smesharik(const std::string &n) : name(n) {}

    std::string getName() const { return name; }

    virtual void speak() = 0;
    virtual ~Smesharik() = default;
};
#endif