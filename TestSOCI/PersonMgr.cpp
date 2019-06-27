#include "PersonMgr.h"
#include <thread>
#include <iostream>

using namespace std;

namespace
{
    // MySQL can index only the first N chars of a BLOB or TEXT column.
    // SOCI blob interface is not supported by the MySQL backend.
    const string SQL_CREATE = "CREATE TABLE IF NOT EXISTS {} (\
`Name` TEXT NOT NULL , `ID` VARCHAR(100) NOT NULL UNIQUE, `Age` INTEGER,`Sex` TEXT,\
 `Height` REAL NOT NULL,  PRIMARY KEY(`ID`))";
    const string SQL_REPLACE = "REPLACE INTO {} ( Name, ID, Age, Sex, Height)\
 values(:name,:id, :age, :sex, :height)";
    const string SQL_SELECT = "SELECT * FROM {} WHERE `Sex`=:sex limit {}";

}


/*PersonMgr(soci::session &&se, std::string table = "Person") :
_session(std::move(se)), _table(std::move(table))
{
static_assert(std::is_move_constructible<soci::session>::value, "hah");
}*/

void PersonMgr::CreateTable()
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

void PersonMgr::DropTable()
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
void PersonMgr::Put(const std::vector<Person>& others)
{
    auto size = others.size();
    string name, id, sex;
    int age;
    double height;
    try
    {
        auto sql = fmt::format(SQL_REPLACE, _table);
        wrt_lock_t wl(_shmt);
        auto & _session = soci::session(*_ppool);
        soci::transaction tr(_session);
        soci::statement st = (_session.prepare << (sql), soci::use(name), soci::use(id),
            soci::use(age), soci::use(sex), soci::use(height));
        for (size_t i = 0; i < size; i++) {
            const auto & person = others.at(i);
            name = person.name();
            id = person.id();
            age = person.age();
            sex = person.sex() ? "M" : "F";
            height = person.height();
            string msg(std::to_string(i));
            if (i%2)
            {
                msg += "i love lyw";
            }
            st.execute(true);
            if ((i+1) % 100000 == 0)
            {
                spdlog::info("已插入 {} 条数据", i + 1);
            }
        }

        tr.commit();
    }
    catch (soci::soci_error const &e)
    {
        spdlog::error("{} {}:{}", e.what(), __FUNCTION__, __LINE__);
    }
    catch (std::exception const & e)
    {
        spdlog::error("{} {}:{}", e.what(), __FUNCTION__, __LINE__);
    }
}

// transaction √ prepare  √  bulk √
void PersonMgr::Put5(const std::vector<Person>& others, size_t BULK_SIZE /*= 50*/)
{
    auto size = others.size();
    vector<string> name(BULK_SIZE), id(BULK_SIZE), sex(BULK_SIZE);
    vector<int> age(BULK_SIZE);
    vector<double> height(BULK_SIZE);

    try
    {
        auto sql = fmt::format(SQL_REPLACE, _table);
        wrt_lock_t wl(_shmt);
        auto & _session = soci::session(*_ppool);
        soci::transaction tr(_session);
        soci::statement st = (_session.prepare << (sql), soci::use(name), soci::use(id), soci::use(age),
            soci::use(sex), soci::use(height));
        for (size_t i = 0, j = 0; i < size; i++, j++) {
            const auto & person = others.at(i);
            name.at(j) = person.name();
            id.at(j) = person.id();
            age.at(j) = person.age();
            sex.at(j) = person.sex() ? "M" : "F";
            height.at(j) = person.height();
            if (j + 1 == BULK_SIZE || i + 1 == size)
            {
                st.execute(true);
                j = 0;
            }
        }

        tr.commit();
    }
    catch (soci::soci_error const &e)
    {
        spdlog::error("{} {}:{}", e.what(), __FUNCTION__, __LINE__);
    }
    catch (std::exception const & e)
    {
        spdlog::error("{} {}:{}", e.what(), __FUNCTION__, __LINE__);
    }
}

std::vector<Person> PersonMgr::Get(bool female, unsigned limit)
{
    std::vector<Person> ps;
    ps.reserve(limit);
    try
    {
        auto sql = fmt::format(SQL_SELECT, _table, limit);
        read_lock_t rl(_shmt);
        auto & _session = soci::session(*_ppool);
        soci::rowset<soci::row> rs = (_session.prepare << (sql), soci::use(female ? 'F' : 'M'));
        for (auto it = rs.begin(); it != rs.end(); ++it)
        {
            const soci::row& row = *it;
//#define  __PRINT_PROP_
//#ifdef __PRINT_PROP_
            for (size_t i = 0; i < row.size(); i++)
            {
                auto & props = row.get_properties(i);
                cout << '<' << props.get_name() << '>';
                switch (props.get_data_type())
                {
                case soci::dt_string:
                    cout << row.get<std::string>(i) << "[dt_string]";
                    break;
                case soci::dt_double:
                    cout << row.get<double>(i) << "[dt_double]";
                    break;
                case soci::dt_date:
                    std::tm when = row.get<std::tm>(i);
                    cout << when.tm_year << "[dt_date]";
                    break;
                }

                cout << "</" << props.get_name() << '>' << std::endl;
            }

            cout << endl;
            std::this_thread::sleep_for(1s);
//#endif // __PRINT_PROP_
            ps.push_back(Person(row.get<string>(string("Name"))));

            // WEIRD <Height>1.86[dt_string]</Height>
            //auto height = row.get<double>(string("Height"));    // bad cast
        }
    }
    catch (soci::soci_error const &e)
    {
        spdlog::error("{} {}:{}", e.what(), __FUNCTION__, __LINE__);
    }
    catch (const std::exception& e)
    {
        spdlog::error("{} {}:{}", e.what(), __FUNCTION__, __LINE__);
    }
    return ps;
}


