#include "PersonMgr.h"
#include <thread>
#include <iostream>

using namespace std;

// transaction √ prepare  √  bulk ×
void PersonMgr::Put(const std::vector<Person>& others)
{
    try
    {
        soci::transaction tr(_session);
        auto size = others.size();
        string name, id, sex;
        int age;
        double height;
        soci::statement st = (_session.prepare << (SQL_REPLACE), soci::use(name), soci::use(id),
            soci::use(age), soci::use(sex), soci::use(height));
        for (size_t i = 0; i < size; i++) {
            const auto & person = others.at(i);
            name = person.name();
            id = person.id();
            age = person.age();
            sex = person.sex() ? "M" : "F";
            height = person.height();
            st.execute(true);
            if ((i+1) % 100000 == 0)
            {
                cout << "已插入 " << i+1 << "条数据" << endl;
            }
        }

        tr.commit();
    }
    catch (soci::soci_error const &e)
    {
        cerr << e.what() << endl;
    }
    catch (std::exception const & e)
    {
        cerr << e.what() << endl;
    }
}

// transaction √ prepare  √  bulk √
void PersonMgr::Put5(const std::vector<Person>& others, size_t BULK_SIZE /*= 50*/)
{
    try
    {
        soci::transaction tr(_session);
        auto size = others.size();
        vector<string> name(BULK_SIZE), id(BULK_SIZE), sex(BULK_SIZE);
        vector<int> age(BULK_SIZE);
        vector<double> height(BULK_SIZE);
        soci::statement st = (_session.prepare << (SQL_REPLACE), soci::use(name), soci::use(id), soci::use(age),
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
        cerr << e.what() << endl;
    }
    catch (std::exception const & e)
    {
        cerr << e.what() << endl;
    }
}

std::vector<Person> PersonMgr::Get(bool female)
{
    try
    {
        soci::rowset<soci::row> rs = (_session.prepare << (SQL_SELECT), soci::use(female ? 'F' : 'M'));
        for (auto it = rs.begin(); it != rs.end(); ++it)
        {
            const soci::row& row = *it;
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
                case soci::dt_integer:
                    cout << row.get<int>(i) << "[dt_integer]";
                    break;
                case soci::dt_long_long:
                    cout << row.get<long long>(i) << "[dt_long_long]";
                    break;
                case soci::dt_unsigned_long_long:
                    cout << row.get<unsigned long long>(i) << "[dt_unsigned_long_long]";
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

            // WEIRD <Height>1.86[dt_string]</Height>
            auto height = row.get<double>(string("Height"));    // bad cast
        }
    }
    catch (soci::soci_error const &e)
    {
        cerr << e.what() << endl;
    }
    catch (const std::exception& e)
    {
        cerr << e.what() << endl;
    }
    return std::vector<Person>();
}


