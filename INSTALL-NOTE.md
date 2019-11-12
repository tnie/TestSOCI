github 库中 [tag3.2.3 内容][1] 和 sourceforget 不一致。源码是否一致未验证，至少前者缺少 CMakeLists.txt，无法按照 [3.2.3 手册][3] 安装。

手册中使用的 vc2010，实际在 vc2015 中编译时碰到的坑：

# 不编译任何后端时

```powershell
cmake ../soci-3.2.3 -G "Visual Studio 14 2015" `
-DWITH_BOOST=OFF -DWITH_ORACLE=OFF -DSOCI_EMPTY=ON `
-DSOCI_TESTS=OFF -DWITH_DB2=OFF -DWITH_FIREBIRD=OFF `
-DWITH_MYSQL=OFF -DWITH_ODBC=OFF -DWITH_POSTGRESQL=OFF `
-DWITH_SQLITE3=OFF # -DCMAKE_INSTALL_PREFIX="../v323"
```
  
**使用相对路径时异常**，安装到了根路径下 `/include` `/lib` `/bin` 下，我也没搞懂此处的根目录是哪

**绝对路径**  `-DCMAKE_INSTALL_PREFIX=E:\InstallSOCI\v323` 正常

`cmake --build . --target install`
 
# 编译 sqlite3 后端

文档写得清清楚楚，但 TM 是错的。结合 CMakeLists.txt 根本用不了。[宏的命名改了][2] 

**相对路径不生效**

    -DSQLITE3_INCLUDE_DIR=../sqlite3 -DSQLITE3_LIBRARY=../sqlite3/vc2015/Debug/libsqlite3d.lib

**只能用绝对路径**

    -DSQLITE3_INCLUDE_DIR=E:\InstallSOCI\sqlite3 -DSQLITE3_LIBRARY=E:\InstallSOCI\sqlite3/vc2015/Debug/libsqlite3d.lib

出现 `snprinf` 重定义问题 

> fatal error C1189: #error:  Macro definition of snprintf conflicts with Standard Library function declaration

解决方案：https://blog.csdn.net/A1367297309/article/details/52997312

# 编译 mysql 后端

	-DMYSQL_INCLUDE_DIR=E:\InstallSOCI\mysql\include -DMYSQL_LIBRARIES=E:\InstallSOCI\mysql\lib\vs14\mysqlclient.lib

在编译 sqlite3 后端时，使用 libsqlite3**d**.lib/libsqlite3.lib 时编译命令 `cmake --build` 无区别。但编译 mysql，不区分就会报错，因为 mysql 的调试版本和发布版本存在冲突

> 在 Debug 模式无法链接 Release 版本的 mysql 库。

有的库两种模式兼容，但有的不行。

```powershell
## Debug 版本
cmake ../soci-3.2.3 -G "Visual Studio 14 2015" `
-DWITH_BOOST=OFF -DWITH_ORACLE=OFF -DSOCI_EMPTY=OFF `
-DSOCI_TESTS=OFF -DWITH_DB2=OFF -DWITH_FIREBIRD=OFF `
-DWITH_MYSQL=ON -DWITH_ODBC=OFF -DWITH_POSTGRESQL=OFF `
-DWITH_SQLITE3=ON -DCMAKE_INSTALL_PREFIX=E:\InstallSOCI\0627 `
-DSQLITE3_INCLUDE_DIR=E:\InstallSOCI\sqlite3 -DSQLITE3_LIBRARY=E:\InstallSOCI\sqlite3/libsqlite3d.lib `
-DMYSQL_INCLUDE_DIR=E:\InstallSOCI\mysql\include -DMYSQL_LIBRARIES=E:\InstallSOCI\mysql\lib\vs14\debug\mysqlclient.lib `
-DCMAKE_DEBUG_POSTFIX="d"

cmake --build . --target install
```

```powershell
## Release 版本
cmake ../soci-3.2.3 -G "Visual Studio 14 2015" `
-DWITH_BOOST=OFF -DWITH_ORACLE=OFF -DSOCI_EMPTY=OFF `
-DSOCI_TESTS=OFF -DWITH_DB2=OFF -DWITH_FIREBIRD=OFF `
-DWITH_MYSQL=ON -DWITH_ODBC=OFF -DWITH_POSTGRESQL=OFF `
-DWITH_SQLITE3=ON -DCMAKE_INSTALL_PREFIX=E:\InstallSOCI\0627 `
-DSQLITE3_INCLUDE_DIR=E:\InstallSOCI\sqlite3 -DSQLITE3_LIBRARY=E:\InstallSOCI\sqlite3/libsqlite3.lib `
-DMYSQL_INCLUDE_DIR=E:\InstallSOCI\mysql\include -DMYSQL_LIBRARIES=E:\InstallSOCI\mysql\lib\vs14\mysqlclient.lib

cmake --build . --target install --config Release
```


# 静态库

开启 `SOCI_STATIC`，针对的是外围库。但 soci 核心依旧是动态库，可以 `-DSOCI_SHARED=OFF`
 
# 测试

开启测试，测试环境默认使用 core 的动态库。

编译无法通过，报错  `soci::sqlite3_soci_error::result() ` 找不到定义

**最大的坑**，在 vc2015 中，对于 `sql.once` 本应该抛出异常的场景会直接导致程序崩溃（eg. 表不存在时），但是 `sql.prepare` 能够按照预期抛出异常。关于此问题的详细描述见:

- https://github.com/SOCI/soci/issues/441
- https://github.com/SOCI/soci/issues/465

此问题虽然已经修复，但未合并到 3.2.3 版本中。4.0 的文档已存在，作者为什么不正式发布 v4.0 版本呢

[1]:https://github.com/SOCI/soci/tree/3.2.3
[2]:https://my.oschina.net/memorybox/blog/79255
[3]:http://soci.sourceforge.net/
