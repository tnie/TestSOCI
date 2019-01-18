#include <iostream>
#include "PersonMgr.h"
#include <soci/sqlite3/soci-sqlite3.h>
#include "Utility.h"
#include <random>

using namespace std;

void init(int);
void init2(int);
void init3(int);

void test_vector_cache()
{
    //init(_MAX_COUNT); // 1539ms
    //init2(_MAX_COUNT); // 998ms
    //init3(_MAX_COUNT); // 661ms
}

int main()
{
    const int _MAX_COUNT = 1000 * 50;
    vector<Person> others;
    others.reserve(_MAX_COUNT);     // 增加这一句，可以显著提高 vector 的插入效率
    std::default_random_engine random;
    std::uniform_int_distribution<int> dis1(1, 100);
    std::uniform_int_distribution<int> dis2(0, 1);
    for (size_t i = 0; i < _MAX_COUNT; i++)
    {
        string name("N");
        name += std::to_string(i);
        others.emplace_back(name, uuid(), dis1(random), dis2(random));
        if ((i+1) % 100000 == 0)
        {
            cout << "已生成 " << i+1 << "条数据" << endl;
        }
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
        mgr.DropTable();
        mgr.CreateTable();
        TickTick tt;
        mgr.Put(others);
    }
    return 0;
    {
        mgr.DropTable();
        mgr.CreateTable();
        TickTick tt;
        mgr.Put5(others, 100);  // 优于前者，但差别不大
    }
#ifdef __POOR_PERFORMANCE
    {
        mgr.DropTable();
        mgr.CreateTable();
        TickTick tt;
        mgr.Put6(others, 100);
    }
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
#endif
    return 0;
}

void init(int count)
{
    TickTick tt;
    // 未预先分配空间
    vector<Person> others;
    for (size_t i = 0; i < count; i++)
    {
        string name("N");
        name += std::to_string(i);
        others.emplace_back(name, name, i);
    }
}

void init2(int count)
{
    TickTick tt;
    // 预先分配空间，初始化后再赋值
    vector<Person> others;
    others.resize(count);
    // =
    //vector<Person> others(count, Person());
    for (size_t i = 0; i < count; i++)
    {
        string name("N");
        name += std::to_string(i);
        auto temp = Person{ name, name, i };
        others[i] = std::move(temp);
    }
}

void init3(int count)
{
    TickTick tt;
    // 预先分配空间，不初始化
    vector<Person> others;
    others.reserve(count);
    for (size_t i = 0; i < count; i++)
    {
        string name("N");
        name += std::to_string(i);
        others.emplace_back(name, name, i);
    }
}