#ifndef NEW_H
#define NEW_H

#include "shari.h"
#include <iostream>
#include <string>

class Newer : public Smesharik
{
public:
    Newer(const std::string &n);
    virtual void speak() = 0;
    void IsPlaying();
};

class Krosh : public Newer
{
public:
    Krosh();
    virtual void speak() override;
    void IsPlaying();
    ~Krosh();
};

class Yojik : public Newer
{
public:
    Yojik();
    virtual void speak() override;
    void IsPlaying();
    ~Yojik();
};

#endif
