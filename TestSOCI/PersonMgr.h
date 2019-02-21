#pragma once
#include <string>
#include <vector>
#include <chrono>
#include <soci/soci.h>
#include <iostream>
#include "fmt\bundled\format.h"
using std::string;

class Person
{
public:
    Person() = default;
    Person(std::string name) : _name(std::move(name)) {};
    Person(const std::string & name, const std::string & id, unsigned age, bool sex = true, double height = 0):
        _name(name), _id(id), _age(age), _sex(sex), _height(height)
    {

    }
    string name() const { return _name; }
    string id() const { return _id; }
    int age() const { return _age; }
    bool sex() const { return _sex; }
    double height() const { return _height; }
    string name(const std::string & name)
    {
        string temp = std::move(_name);
        _name = name;
        return temp;
    }
private:
    std::string _name;
    std::string _id;
    int _age;
    bool _sex;
    double _height;
};

class PersonMgr
{

private:
    const string SQL_CREATE = "CREATE TABLE IF NOT EXISTS {} (\
`Name` TEXT NOT NULL , `ID` TEXT NOT NULL UNIQUE, `Age` INTEGER,`Sex` TEXT, `Height` REAL, PRIMARY KEY(`ID`))";
    const string SQL_REPLACE = "REPLACE INTO {} ( Name, ID, Age, Sex, Height) values(:name,:id, :age, :sex, :height)";
    const string SQL_SELECT = "SELECT * FROM {} WHERE `Sex`=:sex limit {}";
public:
    PersonMgr(soci::session &se, std::string table = "Person"):
        _session(se), _table(std::move(table))
    {

    }
    void CreateTable()
    {
        try
        {
            auto sql = fmt::format(SQL_CREATE, _table);
            _session << (sql);
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }
    void DropTable()
    {
        try
        {
            _session << ("DROP TABLE IF EXISTS " + _table);
            // _session << ("DROP TABLE 'Person'");
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }
    // transaction ¡Ì prepare  ¡Ì  bulk ¡Á
    void Put(const std::vector<Person> &);
    // transaction ¡Ì prepare  ¡Ì  bulk ¡Ì
    void Put5(const std::vector<Person> &, size_t BULK_SIZE = 50);

    std::vector<Person> Get(bool female, unsigned limit = 3);

private:
    /*const*/ soci::session & _session;
    const std::string _table;
};