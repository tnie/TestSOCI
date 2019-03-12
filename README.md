 
[soci 手册](http://soci.sourceforge.net/doc/master/) 阅读笔记，以摘抄为主

# Simple SQL statements

In many cases, the SQL query is intended to be **executed only once**, which means that statement parsing and execution can go together. The `session` class provides a special `once` member, which triggers parsing and execution of such one-time statements:

```cpp
sql.once << "drop table persons";
// For shorter syntax, the following form is also allowed:
sql << "drop table persons";
```

# Prepared statement

当我们需要插入 1000 条记录时，自然而然会写出

```cpp
for (int i = 0; i != 1000; ++i)
    sql << "insert into numbers(value) values(:val)", use(i);
```

The problem is that in both examples, not only the statement execution is repeated 100 times, but also the statement parsing and preparation.

需要重点学习 [Statements][4] 这部分。

prepare statement 可以结合 `vector` 批量插入 `st.execute(true)`，以及批量查询 `st.execute(false); while (st.fetch()) {}`

**疑问**：不使用 prepare statement，直接上 bulk operation 对性能提升有多大帮助呢？ 

# Transactions

事务的概念很好理解。但碰到加事务耗时大大减少，不加事务耗时很长的情况，理解不了。

[Improve INSERT-per-second performance of SQLite?][5]

> By default, SQLite will evaluate every `INSERT` / `UPDATE` statement within a unique transaction

上述帖子依次提到的提升插入速度的手段：（对查询没有帮助）
1. Using a Transaction √
2. Using a Prepared Statement √
3. ... 后续的实操性太低
4. 先建索引后插入，相比先插入数据后建索引要慢

前两者对性能提升是非常明显的，只有做完前两者性能依旧不达标时再考虑后面的操作。

|备注  |事务	|预处理	|批量	|插入1千条耗时 |插入5万条耗时  |插入5万条耗时|
|-----|-----|-------|-------|-----------|-----------|--------|
|	  |√	|√	    |×	    |150	    |2336	    |2287
|	  |×	|√	    |×	    |90024	    |	    |
|	  |√	|×	    |×	    |439	    |10726	    |
|50/批 |	√	|×	    |√	    |215	    |4217	    |
|50/批 |	√	|√	    |√	    |	    |2052	    |2174
|分多次提交，每10k条提交一次	  |√	|√	    |√	    |	    |	    |3817<sup>备注2<sup>

其他试验结论：
1. 批量入库，每批的条目超过 5 之后，入库耗时不再有明显的减少
2. 分多次提交事务，反而会增加耗时

# Data Binding

- Binding output data (`into`)
- Binding input data (`use`)

	需要注意的是 `use` 函数里的参数的生命周期，切记不能将函数返回值作为 `use` 函数的参数

当有多个参数需要绑定时，有两种选择：
- Binding by position

	约定 the order of "holes" in the SQL statement and the order of `into` and `use` expression should match.

	此时 sql 语句中可以不命名，使用 "?" 即可 `sql << "insert into person(id, firstname, lastname) values(?, ?, ?)"`
- [Binding by name](http://soci.sourceforge.net/doc/master/binding/#binding-by-name)

	小惊喜 bind the same local data to many "holes" at the same time

除了绑定基本类型，也可以绑定 ~~STL 容器~~ `std::vector`（Bulk operations）以及支持 ORM（object-relational mapping）

1. 只支持 vector 容器，而且只支持基本类型（数值类型、`std::string`、`std::tm`）和做了 ORM 的对象。`soci::blob` 类型不能在容器中使用。细节见 [Static binding for bulk operations][1]
2. 也可以绑定 User-defined C++ types，但需要给定自定义类型与 soci 支持的基本类型之间的转换规则。通过特例化 `type_conversion<> `模板并实现 `from_base` `to_base` 两个静态成员实现。
3. ORM 只是用户自定义类型的特例。自定义类型只是把工作换个地方做，感觉和 Query transformation 同属鸡肋

[Object-Relational Mapping][2] 提到 mapping 存在于自定义类型和 table columns 之间。[C++数据库操作之SOCI][3] 里面描述

> 占位符的名字不一定要和数据库 column 名一致，但后续操作中 `values` 语法<sup>1</sup>里的占位符必需和这里指定的一致。

备注1：插入或更新等 sql 语句中的 `insert into person(name) values(:n)`），在 `select` 查询语句中是不存在占位符的，此时只能和列名映射！ 

# Multithreading

soci 不是线程安全的，虽然 soci 提供了连接池。

> The simplest solution for multithreaded code is to set up a **separate** session object for each thread that needs to inteact with the database. Depending on the design of the client application this might be also the most straightforward approach.

试验结论：

3. 同一 session，并发写入（若使用事务）抛出异常“Cannot begin transaction. cannot start a transaction within a transaction”
4. 同一 session，并发读写可能抛出以下异常或直接崩溃（`iosfwd` 文件或 `delete_scalar.cpp:17` 文件）。即便执行未报错，但查询的结果一直在变化也是无意义的。

	```
	sqlite3_statement_backend::prepare: near "葺葺葺葺葺葺葺葺葺葺葺葺葺葺葺葺葺葺葺葺葺葺葺葺": syntax error PersonMgr::Get:128	// 2
	```
	```
	sqlite3_statement_backend::loadRS: NOT NULL constraint(约束) failed: Person.ID PersonMgr::Get:128	// 3
	```
	```
	Failure to bind on bulk operations PersonMgr::Put5:78
	```
	```
	No sqlite statement created PersonMgr::Get:128	// 1
	```
	```
	[error] No sqlite statement created PersonMgr::Get:128
	[error] sqlite3_statement_backend::prepare: near "葺葺葺葺葺葺葺葺葺葺葺葺葺葺葺葺葺葺葺葺葺葺葺葺": syntax error PersonMgr::Get:128
	[error] sqlite3_statement_backend::prepare: another row available PersonMgr::Get:128
	[error] sqlite3_statement_backend::prepare: another row available PersonMgr::Get:128
	```

5. 同一 session，并发查询除了出现过以上 1、2 异常外，还出现过以下异常

	```
	sqlite3_statement_backend::prepare: unrecognized token: "1000SELECT" PersonMgr::Get:128
	```

6. 并发读写（各自使用不同的 session）除了抛出以上 1、3 异常外，还出现过以下异常
	```
	 [error] sqlite3_statement_backend::prepare: near "SREPLACE": syntax error PersonMgr::Get:128
	 [error] sqlite3_statement_backend::prepare: near "SREPLACE": syntax error PersonMgr::Put5:78
	```
	```
	[error] sqlite3_statement_backend::prepare: database is locked PersonMgr::Get:128
	```


# 暂不关注的特性

## FAQ

[FAQ](http://soci.sourceforge.net/doc/master/faq/) 中关于操作符重载的两个问题还是值得细究的。

## Query transformation

感觉这个特性没什么用啊，并没有比拼凑 sql 语句更简洁直观。未深入了解、测试，不知道是否有功能或者性能上的惊喜

## Data Indicators

In order to support SQL NULL values and other conditions which are NOT real errors, the concept of indicator is provided.

- Select with `NULL` values
- Insert with `NULL` values
- Bulk operations with `NULL` values

real errors 应该是抛异常了吧

## Dynamic binding

前面提到使用 `user` `into` 绑定的是静态绑定机制。

感觉用不到动态绑定。碰到过表中命名存的数值类型，可是动态解析出来死活都是 `std::string`  #TODO#

## Large Objects (LOBs)

分为两种：二进制的，非二进制的

- Binary (BLOBs)，不能结合 `vector` 批量操作
- Long strings and XML


[1]:http://soci.sourceforge.net/doc/master/types/#static-binding
[2]:http://soci.sourceforge.net/doc/master/types/#object-relational-mapping
[3]:https://blog.csdn.net/littlewhite1989/article/details/54691367
[4]:http://soci.sourceforge.net/doc/master/statements/
[5]:https://stackoverflow.com/questions/1711631/improve-insert-per-second-performance-of-sqlite
