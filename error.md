1. 模板的名字查找问题
![not found](./resource_md/error/1.png)
模板中的无限定查找分为两类：
    不是待决类型的 -- 在第一次检查时需要通过（检查模板本身的定义）
        若是非待决类型 -- 是查找不到待决的基类中的名字的
    是待决类型的 -- 推迟到第二次检查（模板实参已确定）
2. ![not found](./resource_md/error/2.png)
template用以指明后面是依赖模板类型参数的模板 消除歧义 -- 与typename功能类似
3. 在进入初始化函数体之前 要确保所有的成员都已经构造好了  初始化函数体只是调整变量的状态
4. 声明时声明了默认参数的值 定义时就不能写了
5. 赋值运算符最后需要返回自己的引用
6. 在类外实现模板的全特化需要声明为inline -- 全特化貌似不隐式inline
7. "aka" 是 "also known as" 的缩写 -- 后面的内容可能更好懂



1. 一有连接就抛出bad_weak_ptr异常  -- 是因为私有继承了std::enable_shared_from_this
std::enable_shared_from_this实现