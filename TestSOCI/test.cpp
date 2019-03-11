#include <thread>
#include "PersonMgr.h"
#include <soci/sqlite3/soci-sqlite3.h>
#include "Utility.h"
#include <random>

using namespace std;

vector<Person> data(size_t count)
{
    vector<Person> others;
    others.reserve(count);     // 增加这一句，可以显著提高 vector 的插入效率
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
            spdlog::info("已经生成 {} 条数据", i + 1);
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
        spdlog::error("{} {}:{}", e.what(), __FUNCTION__, __LINE__);
    }
    PersonMgr mgr(local);
    {
        mgr.DropTable();
        mgr.CreateTable();
        std::thread([&mgr, &others] {
            TickTick tt;
            mgr.Put5(others, 100);
        }).detach();

        // 若要保证数据一致性，需要自行加锁！
        for (size_t i = 0; i < 50; i++)
        {
            std::thread([&mgr, i] {
                spdlog::info("===#{} {}", i, mgr.Get(true, 1000).size());
            }).detach();
            //this_thread::sleep_for(10ms);
        }
    }

    getchar();
    return 0;
}