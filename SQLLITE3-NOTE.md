针对开发过程中，碰到的 sqlite 和常规 sql 数据库不一致，或者不符合“常识”的地方。

# 亲和类型

> sqlite3 动态类型，建议存储 `real/double`，读取时却是 `text/string`，还得做向数值类型的转换 [issue #41][1]

> With static typing, the datatype of a value is determined by its container - the particular column in which the value is stored.
> In SQLite, the datatype of a value is associated with the value itself, not with its container.
> Any column in an SQLite version 3 database, except an INTEGER PRIMARY KEY column, may be used to store a value of any storage class.[手册][2]

涉及的三个术语：
- Storage Classes and Datatypes
- Type Affinity

SQLite 支持列的亲和类型概念。任何列仍然可以存储任何类型的数据，当数据插入时，该字段的数据将会优先采用亲缘类型作为该值的存储方式。SQLite 目前的版本支持以下五种亲缘类型：[下表摘自][3]

|亲和类型	|描述|
|-----|--|
|TEXT	|数值型数据在被插入之前，需要先被转换为文本格式，之后再插入到目标字段中。|
|NUMERIC|<ul><li>当文本数据被插入到亲缘性为 NUMERIC 的字段中时，如果转换操作不会导致*数据信息丢失以及完全可逆*，那么 SQLite 就会将该文本数据转换为 INTEGER 或 REAL 类型的数据，如果转换失败，SQLite 仍会以 TEXT 方式存储该数据。</li><li>对于 NULL 或 BLOB 类型的新数据，SQLite 将不做任何转换，直接以 NULL 或 BLOB 的方式存储该数据。</li><li>需要额外说明的是，对于浮点格式的常量文本，如"30000.0"，如果该值可以转换为 INTEGER 同时又不会丢失数值信息，那么 SQLite 就会将其转换为 INTEGER 的存储方式。</li></ul>|
|INTEGER	|对于亲缘类型为INTEGER的字段，其规则等同于NUMERIC，唯一差别是在执行CAST表达式时。|
|REAL	|其规则基本等同于NUMERIC，唯一的差别是不会将"30000.0"这样的文本数据转换为INTEGER存储方式。|
|NONE	|不做任何的转换，直接以该数据所属的数据类型进行存储。　|　

# 线程安全

sqlite3 数据库可以并发读，但不可以并发写 —— 这个和常理一致。但是，此场景针对整个库文件，也就是说对**某张表**执行写操作时，禁止对**整个库**（其他表）的并发操作。

[并发写如何处理][4] 呢？除了分库，

- 多线程编程中：a)循环 b)互斥锁/读写锁；
- 多进程编程中: a)循环 b)文件锁

https://stackoverflow.com/questions/10325683/can-i-read-and-write-to-a-sqlite-database-concurrently-from-multiple-connections

[sqlite 并发访问](http://blog.51cto.com/linzimo777/1544202)

[1]:https://github.com/tnie/StockDataLayer/issues/41
[2]:https://www.sqlite.org/datatype3.html
[3]:http://www.runoob.com/sqlite/sqlite-data-types.html
[4]:https://blog.csdn.net/wxh525123/article/details/7974798