#include <iostream>
#include <thread>
#include "PersonMgr.h"
#include <soci/sqlite3/soci-sqlite3.h>
#include "Utility.h"
#include <random>

using namespace std;

vector<Person> data(int count)
{
    vector<Person> others;
    others.reserve(count);     // ������һ�䣬����������� vector �Ĳ���Ч��
    std::default_random_engine random;
    std::uniform_int_distribution<int> dis1(1, 100);
    std::uniform_int_distribution<int> dis2(0, 1);
    std::uniform_int_distribution<int> dis3(70, 200);
    for (size_t i = 0; i < count; i++)
    {
        string name("N");
        name += std::to_string(i);
        auto height = static_cast<double>(dis3(random)) / 100.0;
        others.emplace_back(name, uuid(), dis1(random), dis2(random), height);
        if ((i + 1) % 100000 == 0)
        {
            cout << "������ " << i + 1 << "������" << endl;
        }
    }
    return others;
}

int main()
{
    const int _MAX_COUNT = 1000 * 50;
    auto others = data(_MAX_COUNT);
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
        std::thread([&mgr, &others] {
            TickTick tt;
            mgr.Put5(others, 100);
        }).detach();
        // �����ͬʱ��ѯ��ִ��δ��������ѯ�Ľ���ڱ仯��
        // ��Ҫ��֤����һ���ԣ���Ҫ���м�����
        for (size_t i = 0; i < 5; i++)
        {
            cout << "=== " << mgr.Get(true, 1000).size() << endl;
            this_thread::sleep_for(10ms);
        }
    }

    getchar();
    return 0;
}