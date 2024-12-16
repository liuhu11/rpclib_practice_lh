1. 使用formatter的日志
2. 能用unique_ptr的地方用unique_ptr 可能不止一种情况
3. 审视一边注释及代码
4. 尽量去除所有的裸指针
5. 看多线程条件怎么保证的
6. 搜索中英文？ ！ todo
7. 看正常使用时的调用流程
8. 注意所有的atomic和mutex的意义
9. 看看cookbook
10. 改正所有的log.h

紧急：
1. 注意osync什么时候emit
2. handler的移动构造
3. client.cpp state有什么用 -- 不能及时退出
4. logger_有没有必要用mutable 在response::data中无法正常记录日志