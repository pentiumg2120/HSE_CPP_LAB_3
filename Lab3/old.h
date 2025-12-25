#ifndef OLD_H
#define OLD_H

#include "shari.h"
#include <iostream>
#include <string>

class Older : public Smesharik
{
public:
    Older(const std::string &n);
    virtual void giveAdvice();
};

class Karich : public Older
{
public:
    Karich();
    void speak() override;
    void fly();
    void giveAdvice() override;
    ~Karich();
};

class Pin : public Older
{
public:
    Pin();

    void speak() override;

    void giveAdvice() override;
    ~Pin();
};

class Sovuniya : public Older
{
public:
    Sovuniya();

    void speak() override;

    void giveAdvice() override;
    ~Sovuniya();
};

class Losyash : public Older
{
public:
    Losyash();

    void speak() override;

    void giveAdvice() override;
    ~Losyash();
};

class Kopatich : public Older
{
public:
    Kopatich();

    void speak() override;

    void giveAdvice() override;
    ~Kopatich();
};

#endif
