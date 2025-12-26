#ifndef NEW_H
#define NEW_H

#include "shari.h"
#include <iostream>
#include <string>

class Newer : public Smesharik
{
public:
    Newer(const std::string &n);
    virtual void speak() override;
    void IsPlaying();
};

class Krosh : public Newer
{
public:
    Krosh();
    virtual void speak() override;
    ~Krosh();
};

class Yojik : public Newer
{
public:
    Yojik();
    virtual void speak() override;
    ~Yojik();
};

#endif
