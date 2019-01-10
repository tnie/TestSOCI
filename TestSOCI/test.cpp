#include <iostream>
#include "PersonMgr.h"
#include <soci/sqlite3/soci-sqlite3.h>
#include "Utility.h"
#include <random>

using namespace std;

int main()
{
    vector<Person> others;
    std::default_random_engine random;
    std::uniform_int_distribution<int> dis1(1, 100);
    std::uniform_int_distribution<int> dis2(0, 1);
    for (size_t i = 0; i < 1000*50; i++)
    {
        string name("N");
        name += std::to_string(i);
        others.emplace_back(name, uuid(), dis1(random), dis2(random));
    }
    soci::session local;
    try
    {
        local.open(soci::sqlite3, u8R"(.\data\22#.db)");
    }
    catch (const std::exception& e)
    {
        cerr << e.what() << endl;
    }
    PersonMgr mgr(local);
    {
        //mgr.DropTable();
        //mgr.CreateTable();
        TickTick tt;
        mgr.Put(others);
    }
    {
        mgr.DropTable();
        mgr.CreateTable();
        TickTick tt;
        mgr.Put5(others, 100);
    }
    {
        mgr.DropTable();
        mgr.CreateTable();
        TickTick tt;
        mgr.Put6(others, 100);
    }
    return 0;
    {
        /*mgr.DropTable();
        mgr.CreateTable();
        TickTick tt;
        mgr.Put2(others);*/
    }
    {
        mgr.DropTable();
        mgr.CreateTable();
        TickTick tt;
        mgr.Put3(others);
    }
    {
        mgr.DropTable();
        mgr.CreateTable();
        TickTick tt;
        mgr.Put4(others);
    }
    return 0;
}