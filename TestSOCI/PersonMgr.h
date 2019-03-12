#pragma once
#include <shared_mutex>
#include <string>
#include <vector>
#include <chrono>
#include <memory>
#include <soci/soci.h>
#include <iostream>
#include "spdlog/spdlog.h"
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
    PersonMgr(std::shared_ptr<soci::connection_pool> ppool, std::string table = "Person"):
        _ppool(ppool), _table(std::move(table))
    {
        ppool->at(4);   // 连接池至少分配五个会话
    }
    /*PersonMgr(soci::session &&se, std::string table = "Person") :
        _session(std::move(se)), _table(std::move(table))
    {
        static_assert(std::is_move_constructible<soci::session>::value, "hah");
    }*/
    void CreateTable()
    {
        try
        {
            wrt_lock_t wl(_shmt);
            auto sql = fmt::format(SQL_CREATE, _table);
            soci::session(*_ppool) << (sql);
        }
        catch (const std::exception& e)
        {
            spdlog::error("{} {}:{}", e.what(), __FUNCTION__, __LINE__);
        }
    }
    void DropTable()
    {
        try
        {
            wrt_lock_t wl(_shmt);
            soci::session(*_ppool) << ("DROP TABLE IF EXISTS " + _table);
            // _session << ("DROP TABLE 'Person'");
        }
        catch (const std::exception& e)
        {
            spdlog::error("{} {}:{}", e.what(), __FUNCTION__, __LINE__);
        }
    }
    // transaction √ prepare  √  bulk ×
    void Put(const std::vector<Person> &);
    // transaction √ prepare  √  bulk √
    void Put5(const std::vector<Person> &, size_t BULK_SIZE = 50);

    std::vector<Person> Get(bool female, unsigned limit = 3);

    typedef std::unique_lock<std::shared_mutex> wrt_lock_t;
    typedef std::shared_lock<std::shared_mutex> read_lock_t;

private:
    std::shared_ptr<soci::connection_pool> _ppool;
    std::shared_mutex _shmt;    // 针对表？还是针对库（连接池）？
    const std::string _table;
};