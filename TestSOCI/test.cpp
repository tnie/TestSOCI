#include <thread>
#include "PersonMgr.h"
//#include <soci/sqlite3/soci-sqlite3.h>
#define _WINSOCKAPI_
#include <soci/mysql/soci-mysql.h>
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
        switch (i)
        {
        case 0:
            height = 100.0;
            break;
        case 1:
            height = std::nan("");
            break;
        case 2:
            height = std::nan("FeiShu!");
            break;
        default:
            break;
        }
        others.emplace_back(name, uuid(), dis1(random), /*dis2(random)*/false, height);
        if ((i + 1) % 100000 == 0)
        {
            spdlog::info("已经生成 {} 条数据", i + 1);
        }
    }
    return others;
}

int main()
{
    const int _MAX_COUNT = 1000/* * 50*/;
    auto others = data(_MAX_COUNT);
    const unsigned _POOL_SIZE = 50;
    auto ppool = std::make_shared<soci::connection_pool>(_POOL_SIZE);
    static_assert( std::is_move_constructible<soci::connection_pool>::value, "");
    try
    {
        for (size_t i = 0; i < _POOL_SIZE; i++)
        {
            auto & local = ppool->at(i);
            //local.open(soci::sqlite3, u8R"(.\data\22#.db)");
            local.open(soci::mysql, "db=test user=root password=''");
        }

    }
    catch (const std::exception& e)
    {
        spdlog::error("{} {}:{}", e.what(), __FUNCTION__, __LINE__);
    }
    vector<thread> vt;
    //PersonMgr mgr(soci::session(pool)); // ERR
    PersonMgr mgr{ ppool };
    //PersonMgr mgr2{ ppool, "Student" };
    {
        mgr.DropTable();
        mgr.CreateTable();
        /*mgr2.DropTable();
        mgr2.CreateTable();
        mgr2.Put5(others, 100);*/
        vt.emplace_back([&mgr, &others] {
            TickTick tt;
            mgr.Put(others);
        });

        // 若要保证数据一致性，需要自行加锁！
        //for (size_t i = 0; i < 50; i++)
        //{
        //    vt.emplace_back([i, &mgr, &mgr2] {
        //        //spdlog::info("===#{} {}", i, mgr.Get(true, 1000).size());
        //        spdlog::info("===#{} {}", i, mgr2.Get(true, 1000).size());
        //    });
        //    this_thread::sleep_for(100ms);

        //}
    }

    getchar();
    for (size_t i = 0; i < vt.size(); i++)
    {
        if (vt.at(i).joinable())
        {
            vt.at(i).join();
        }
    }
    mgr.Get(true, 1000);
    return 0;
}