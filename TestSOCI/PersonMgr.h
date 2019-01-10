#pragma once
#include <string>
#include <vector>
#include <chrono>
#include <soci/soci.h>
using std::string;



class Person
{
public:
    Person(const std::string & name, const std::string & id, unsigned age, bool sex = true):
        _name(name), _id(id), _age(age), _sex(sex)
    {

    }
    string name() const
    {
        return _name;
    }
    string id() const
    {
        return _id;
    }
    int age() const
    {
        return _age;
    }
    bool sex() const
    {
        return _sex;
    }
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
};
class PersonMgr
{

private:
    const string SQL_CREATE = "CREATE TABLE IF NOT EXISTS `Person` (\
`Name` TEXT NOT NULL , `ID` TEXT NOT NULL UNIQUE, `Age` INTEGER,`Sex` TEXT, PRIMARY KEY(`ID`))";
    const string SQL_REPLACE = "REPLACE INTO Person( Name, ID, Age, Sex) values(:name,:id, :age, :sex)";
public:
    PersonMgr(soci::session &se):
        _session(se)
    {

    }
    void CreateTable()
    {
        try
        {
            _session << (SQL_CREATE);

        }
        catch (const std::exception&)
        {

        }
    }
    void DropTable()
    {
        try
        {
            _session << ("DROP TABLE IF EXISTS 'Person'");
            // _session << ("DROP TABLE 'Person'");
        }
        catch (const std::exception&)
        {

        }
    }
    void Put(const std::vector<Person> &);
    void Put2(const std::vector<Person> &);
    void Put3(const std::vector<Person> &);
    void Put4(const std::vector<Person> &, size_t BULK_SIZE = 50);
    void Put5(const std::vector<Person> &, size_t BULK_SIZE = 50);
    void Put6(const std::vector<Person> &, size_t BULK_SIZE = 50);

private:
    /*const*/ soci::session & _session;

};
