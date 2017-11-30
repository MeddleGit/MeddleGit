#pragma once

#include <QString>
#include <initializer_list>

class Git
{
public:
    static QString Cmd(std::initializer_list<QString> args);

    /*template<typename... Ts>
    static QString Cmd(Ts&&... args)
    {
        return Cmd(std::initializer_list<QString>{args...});
    }*/
};
