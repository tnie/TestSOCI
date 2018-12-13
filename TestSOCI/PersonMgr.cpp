#include "PersonMgr.h"
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
        soci::statement st = (_session.prepare << (SQL_REPLACE), soci::use(name), soci::use(id), soci::use(age), soci::use(sex));
        for (size_t i = 0; i < size; i++) {
            const auto & person = others.at(i);
            name = person.name();
            id = person.id();
            age = person.age();
            sex = person.sex() ? "M" : "F";
            st.execute(true);
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
        soci::statement st = (_session.prepare << (SQL_REPLACE), soci::use(name), soci::use(id), soci::use(age), soci::use(sex));
        for (size_t i = 0, j = 0; i < size; i++, j++) {
            const auto & person = others.at(i);
            name.at(j) = person.name();
            id.at(j) = person.id();
            age.at(j) = person.age();
            sex.at(j) = person.sex() ? "M" : "F";
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

// transaction √ prepare  √  bulk √
// 试验分多次提交事务，似乎会降低效率
void PersonMgr::Put6(const std::vector<Person>& others, size_t BULK_SIZE /*= 50*/)
{
    try
    {
        auto tr = std::make_shared<soci::transaction>(_session);
        auto size = others.size();
        vector<string> name(BULK_SIZE), id(BULK_SIZE), sex(BULK_SIZE);
        vector<int> age(BULK_SIZE);
        soci::statement st = (_session.prepare << (SQL_REPLACE), soci::use(name), soci::use(id), soci::use(age), soci::use(sex));
        for (size_t i = 0, j = 0; i < size; i++, j++) {
            const auto & person = others.at(i);
            name.at(j) = person.name();
            id.at(j) = person.id();
            age.at(j) = person.age();
            sex.at(j) = person.sex() ? "M" : "F";
            if (j + 1 == BULK_SIZE || i + 1 == size)
            {
                st.execute(true);
                j = 0;
            }
            // 每10000条数据提交一次
            if (i%10000 == 0 || i+1 == size)
            {
                tr->commit();
                tr = make_shared<soci::transaction>(_session);

            }
        }

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

// transaction × prepare √  bulk ×
void PersonMgr::Put2(const std::vector<Person>& others)
{
    try
    {
        //soci::transaction tr(_session);
        auto size = others.size();
        string name, id, sex;
        int age;
        soci::statement st = (_session.prepare << (SQL_REPLACE), soci::use(name), soci::use(id), soci::use(age), soci::use(sex));
        for (size_t i = 0; i < size; i++) {
            const auto & person = others.at(i);
            name = person.name();
            id = person.id();
            age = person.age();
            sex = person.sex() ? "M" : "F";
            st.execute(true);
        }

        //tr.commit();
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

// transaction √ prepare × bulk ×
void PersonMgr::Put3(const std::vector<Person>& others)
{
    try
    {
        soci::transaction tr(_session);
        auto size = others.size();
        string name, id, sex;
        int age;
        for (size_t i = 0; i < size; i++) {
            const auto & person = others.at(i);
            name = person.name();
            id = person.id();
            age = person.age();
            sex = person.sex() ? "M" : "F";
            /*soci::statement st = (*/_session/*.prepare*/ << (SQL_REPLACE), soci::use(name), soci::use(id), soci::use(age), soci::use(sex)/*)*/;
            //st.execute(true);
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

// transaction √ prepare × bulk √
void PersonMgr::Put4(const std::vector<Person> & others, size_t BULK_SIZE /*= 50*/)
{
    try
    {
        soci::transaction tr(_session);
        auto size = others.size();
        vector<string> name(BULK_SIZE), id(BULK_SIZE), sex(BULK_SIZE);
        vector<int> age(BULK_SIZE);
        for (size_t i = 0, j=0; i < size; i++, j++) {
            const auto & person = others.at(i);
            name.at(j) = person.name();
            id.at(j) = person.id();
            age.at(j) = person.age();
            sex.at(j) = person.sex() ? "M" : "F";
            if (j+1 == BULK_SIZE || i+1 == size)
            {
                /*soci::statement st = (*/_session/*.prepare*/ << (SQL_REPLACE), soci::use(name), soci::use(id), soci::use(age), soci::use(sex)/*)*/;
                j = 0;
            }
            //st.execute(true);
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

