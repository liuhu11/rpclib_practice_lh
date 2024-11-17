# 注意事项

1. 本文件所有列举都只列出关键部分，并不全面
1. todo：找些相关链接帮助理解

## boost::asio

[asio 手册](https://www.boost.org/doc/libs/1_86_0/doc/html/boost_asio/reference.html)

注意：

1. `ip::tcp`是一个类

### async_connect()

[async_connect - 1.86.0](https://www.boost.org/doc/libs/1_86_0/doc/html/boost_asio/reference/async_connect.html)

作用如下：

1. The async_connect function is a composed asynchronous operation that establishes a socket connection by trying each endpoint in a sequence.
2. 当连接操作完成时，用户提供的处理程序会被调用

### async_write()

[async_write - 1.86.0](https://www.boost.org/doc/libs/1_86_0/doc/html/boost_asio/reference/async_write.html)

作用如下：

1. 异步写入，调用后返回，不阻塞
2. 保证会写入指定缓冲区中的所有数据，除非发生错误。
3. 内部可能会多次调用 async_write_some 来完成整个写入操作。
4. 写入完成或出错时，会调用指定的回调函数（handler），通知操作的结果。
5. 与 io_context 配合：async_write 需要与 boost::asio::io_context 配合使用，以管理异步操作。

### io_context

[io_context - 1.86.0](https://www.boost.org/doc/libs/1_86_0/doc/html/boost_asio/reference/io_context.html)

相关链接：

[boost::asio::io_context 在多线程环境中的行为和原理boost::asio::io_context - 掘金](https://juejin.cn/post/7386514632725430306)

[Boost.Asio看这一篇就够了 | zhangxiaoan is god](http://www.anger6.com/2022/05/05/boost/asio/)

作用如下：

1. 提供事件循环机制
2. 处理各种异步事件

成员函数：

1. [io_context::io_context - 1.86.0](https://www.boost.org/doc/libs/1_86_0/doc/html/boost_asio/reference/io_context/io_context.html)
1. [io_context::post - 1.86.0](https://www.boost.org/doc/libs/1_86_0/doc/html/boost_asio/reference/io_context/post.html)
1. [io_context::stop - 1.86.0](https://www.boost.org/doc/libs/1_86_0/doc/html/boost_asio/reference/io_context/stop.html)

### io_context::strand

[io_  `	` 	1context::strand - 1.86.0 (boost.org)](https://www.boost.org/doc/libs/1_86_0/doc/html/boost_asio/reference/io_context__strand.htmq2l)

作用如下：

1. 保证顺序执行
2. 与io_context配合

成员函数：

1. [io_context::strand::wrap - 1.86.0 (boost.org)](https://www.boost.org/doc/libs/1_86_0/doc/html/boost_asio/reference/io_context__strand/wrap.html)
1. [构造函数: 支持复制io_context::strand::strand - 1.86.0](https://www.boost.org/doc/libs/1_86_0/doc/html/boost_asio/reference/io_context__strand/strand.html)
1. [io_context::strand::post - 1.86.0](https://www.boost.org/doc/libs/1_86_0/doc/html/boost_asio/reference/io_context__strand/post.html)

### ip::tcp::socket

[ip::tcp::socket - 1.86.0 (boost.org)](https://www.boost.org/doc/libs/1_86_0/doc/html/boost_asio/reference/ip__tcp/socket.html)

作用如下：

1. 建立 TCP 连接
2. 发送数据，接收数据
3. 管理连接：关闭连接，检查连接状态
4. 设置 socket 选项
5. 获取连接信息：获取本地或远程的 ip 与端口号



成员函数：

1. [basic_stream_socket::close - 1.86.0 (boost.org)](https://www.boost.org/doc/libs/1_86_0/doc/html/boost_asio/reference/basic_stream_socket/close.html)
1. [构造函数: 不支持复制，支持移动basic_stream_socket::basic_stream_socket - 1.86.0](https://www.boost.org/doc/libs/1_86_0/doc/html/boost_asio/reference/basic_stream_socket/basic_stream_socket.html)
1. [basic_stream_socket::async_read_some - 1.86.0](https://www.boost.org/doc/libs/1_86_0/doc/html/boost_asio/reference/basic_stream_socket/async_read_some.html)
1. [Get the remote endpoint of the socket. basic_stream_socket::remote_endpoint - 1.86.0](https://www.boost.org/doc/libs/1_86_0/doc/html/boost_asio/reference/basic_stream_socket/remote_endpoint.html)
1. [Get the local endpoint of the socket. basic_stream_socket::local_endpoint - 1.86.0](https://www.boost.org/doc/libs/1_86_0/doc/html/boost_asio/reference/basic_stream_socket/local_endpoint.html)

### ip::tcp::resolver

[ip::tcp::resolver - 1.86.0](https://www.boost.org/doc/libs/1_86_0/doc/html/boost_asio/reference/ip__tcp/resolver.html)

作用如下：

1. 将域名转化为ip地址
2. 将服务名（如http）转化为端口号

成员函数：

1. [返回值是range ip::basic_resolver::resolve - 1.86.0](https://www.boost.org/doc/libs/1_86_0/doc/html/boost_asio/reference/ip__basic_resolver/resolve.html)

### ip::tcp::acceptor

[ip::tcp::acceptor - 1.86.0](https://www.boost.org/doc/libs/1_86_0/doc/html/boost_asio/reference/ip__tcp/acceptor.html)

作用如下：

1. 接受来自客户端的连接请求，并为这些连接提供一个可以进行数据传输的 socket。
2. 需要绑定到一个特定的端口，以便监听该端口上的传入连接。通过指定一个 `tcp::endpoint` 对象，开发者可以定义要监听的 IP 地址和端口号。

成员函数：

1. [构造函数 basic_socket_acceptor::basic_socket_acceptor - 1.86.0](https://www.boost.org/doc/libs/1_86_0/doc/html/boost_asio/reference/basic_socket_acceptor/basic_socket_acceptor.html)
2. [Open the acceptor using the specified protocol. basic_socket_acceptor::open - 1.86.0](https://www.boost.org/doc/libs/1_86_0/doc/html/boost_asio/reference/basic_socket_acceptor/open.html)
3. [basic_socket_acceptor::set_option - 1.86.0](https://www.boost.org/doc/libs/1_86_0/doc/html/boost_asio/reference/basic_socket_acceptor/set_option.html)
4. [Bind the acceptor to the given local endpoint. basic_socket_acceptor::bind - 1.86.0](https://www.boost.org/doc/libs/1_86_0/doc/html/boost_asio/reference/basic_socket_acceptor/bind.html)
5. [Place the acceptor into the state where it will listen for new connections. basic_socket_acceptor::listen - 1.86.0](https://www.boost.org/doc/libs/1_86_0/doc/html/boost_asio/reference/basic_socket_acceptor/listen.html)
6. [basic_socket_acceptor::async_accept - 1.86.0](https://www.boost.org/doc/libs/1_86_0/doc/html/boost_asio/reference/basic_socket_acceptor/async_accept.html)
7. [Cancel all asynchronous operations associated with the acceptor. basic_socket_acceptor::cancel - 1.86.0](https://www.boost.org/doc/libs/1_86_0/doc/html/boost_asio/reference/basic_socket_acceptor/cancel.html)

### ip::tcp::endpoint

[ip::tcp::endpoint - 1.86.0](https://www.boost.org/doc/libs/1_86_0/doc/html/boost_asio/reference/ip__tcp/endpoint.html)

作用如下：

1. 封装了 IP 地址和端口号

成员函数：

1.  [构造函数 ip::basic_endpoint::basic_endpoint - 1.86.0](https://www.boost.org/doc/libs/1_86_0/doc/html/boost_asio/reference/ip__basic_endpoint/basic_endpoint.html)
2. [Get the port associated with the endpoint. The port number is always in the host's byte order. ip::basic_endpoint::port - 1.86.0](https://www.boost.org/doc/libs/1_86_0/doc/html/boost_asio/reference/ip__basic_endpoint/port.html)
3. [Get the IP address associated with the endpoint. ip::basic_endpoint::address - 1.86.0](https://www.boost.org/doc/libs/1_86_0/doc/html/boost_asio/reference/ip__basic_endpoint/address.html)

## msgpack

[msgpack手册](https://c.msgpack.org/cpp/namespacemsgpack.html)

[github地址](https://github.com/msgpack/msgpack-c)

### object

[MessagePack for C++: msgpack::object Struct Reference](https://c.msgpack.org/cpp/structmsgpack_1_1object.html)

作用如下：

1. 表示解包（反序列化）后的数据
2. 通过 type 成员变量可以获取 msgpack::object 的具体类型,对应于 MessagePack 的类型
3. 可以从 msgpack::object 中获取具体的数据值,也可以将其转换为 C++ 的原生类型或自定义类型
4. 可以从 C++ 类型创建 msgpack::object,用于序列化

成员变量：

1. `msgpack::object::union_type via;`

成员函数：

1. ```c++
   // v: 想要得到的结果
   // 返回值：v的引用
   template<typename T>
   T& convert(T& v) const;
   ```
   
2. ```c++
   // Constructer
   // The object is constructed on the zone z. 
   template<typename T>
   object (const T &v, msgpack::zone &z);
   // 支持复制
   ```

嵌套类型：

```c++
union union_type {
        bool boolean;
        uint64_t u64;
        int64_t  i64;
#if defined(MSGPACK_USE_LEGACY_NAME_AS_FLOAT)
        MSGPACK_DEPRECATED("please use f64 instead")
        double   dec; // obsolete
#endif // MSGPACK_USE_LEGACY_NAME_AS_FLOAT
        double   f64;
        msgpack::object_array array;
        msgpack::object_map map;
        msgpack::object_str str;
        msgpack::object_bin bin;
        msgpack::object_ext ext;
    };
```



### object_handle

[MessagePack for C++: msgpack::object_handle Class Reference](https://c.msgpack.org/cpp/classmsgpack_1_1object__handle.html#details)

别名：

​	`typedef object_handle msgpack::unpacked`

作用如下：

1. 包装了object和zone 被销毁时，自动释放相关内存资源
2. 管理object和zone的生命周期

成员函数

1. ```c++
   // 更新内部的object为obj 是复制
   void set(const msgpack::object & obj);
   ```

2. ```c++
   // creates nil object and null zone.
   object_handle();
   
   // creates an object_handle holding object obj and zone z.
   object_handle(const msgpack::object& obj, msgpack::unique_ptr<msgpack::zone>&& z);
   // 不支持复制和移动
   ```

3. ```c++
   // 获取对应的object的常量引用
   const msgpack::object& get() const;
   ```

4. 

### sbuffer

https://c.msgpack.org/cpp/classmsgpack_1_1sbuffer.html

作用如下：

1. 存储序列化后的数据
2. 自动扩容

成员函数：

1. ```c++
   sbuffer(size_t initsz = MSGPACK_SBUFFER_INIT_SIZE);
   // 不支持复制 支持移动
   ```

2. ```c++
   // 逻辑上清空清空缓冲区的内容
   void clear();
   ```

3. 

### zone

[MessagePack for C++: msgpack::zone Class Reference](https://c.msgpack.org/cpp/classmsgpack_1_1zone.html)

作用如下：

1. 负责为解包后的 msgpack::object 分配和管理内存。它使用一种高效的内存池机制,可以快速分配小块内存

成员函数：

1. ```c++
   zone(size_t chunk_size = MSGPACK::ZONE::CHUNK_SIZE);
   // 不支持复制 支持移动
   ```

### unpacker

[MessagePack for C++: msgpack::unpacker Class Reference](https://c.msgpack.org/cpp/classmsgpack_1_1unpacker.html)

相关链接：

[msgpack-c 官方文档整理翻译之unpack_msgpack.unpackb(data, raw=true) 文档-CSDN博客](https://blog.csdn.net/weixin_40859716/article/details/123783181)

作用如下：

1. msgpack::unpacker用于从包含msgpack格式数据的缓冲区中解包出msgpack::object

成员函数：

1. ```c++
   unpacker(unpack_reference_func f = &unpacker::default_reference_func,
           void* user_data = MSGPACK::NULLPTR,
           size_t initial_buffer_size = MSGPACK_UNPACKER_INIT_BUFFER_SIZE,
           const unpack_limit& limit = unpack_limit());
   // 不支持复制，支持移动
   ```

2. ```c++
   // Get buffer pointer.
   char* buffer();
   ```

3. ```c++
   // Notify a buffer consumed information to msgpack::unpacker.
   void buffer_consumed(size_t size);
   ```

4. ```c++
   // Unpack one msgpack::object.
   // result用来存储unpack之后的数据
   bool next(msgpack::object_handle& result);
   
   // referenced: 如果解包的数据包含了对upacker中buffer的引用，则referenced被设置为true
   bool next(msgpack::object_handle& result,
            bool& referenced);
   ```
   
5. ```c++
   // After returning this function, buffer_capacity() returns at least 'size'.
   void reserve_buffer(size_t size = MSGPACK_UNPACKER_RESERVE_SIZE);
   ```

6. ```c++
   // The memory size that you can write.
   size_t buffer_capacity() const;
   ```

7. 

### unpack()

常用声明：

```c++
msgpack::object_handle unpack(const char* data, std::size_t len, 
                              unpack_reference_func f = MSGPACK_NULLPTR, 
                              void* user_data = MSGPAK_NULLPTR, 
                              const unpack_limit& limit = unpack_limit());
```



[MessagePack for C++: msgpack Namespace Reference](https://c.msgpack.org/cpp/namespacemsgpack.html#a21d60c5f750195ba8ed8cab7ceab3ca6)

作用如下：

1. 从包含MessagePack格式数据的缓冲区中解包数据，转换为msgpack::unpacked

### clone()

常用声明

```c++
object_handle clone(const msgpack::object& obj);
```



[MessagePack for C++: msgpack Namespace Reference](https://c.msgpack.org/cpp/namespacemsgpack.html)

作用如下：

1. 创建对象的深拷贝。其主要作用是确保在序列化和反序列化过程中，能够安全地复制数据结构，而不影响原始对象。

### pack()

常用声明

```c++
template<typename Stream, typename T>
void pack(Stream& s, const T& v);
// Stream: Any type that have a member function Stream write(const char*, size_t s)
// T: Any type that is adapted to MessagePack
```

[MessagePack for C++: msgpack Namespace Reference](https://c.msgpack.org/cpp/namespacemsgpack.html#afb0d5514b0618ebde91469fa148e5813)

作用如下：

1. 将消息v序列化到s中

## std

### apply()

[std::apply - cppreference.com](https://zh.cppreference.com/w/cpp/utility/apply)

作用如下：

1. 以元组 t 的各元素作为实参调用可调用 (Callable) 对象 f

### enable_shared_from_this

[std::enable_shared_from_this - cppreference.com](https://zh.cppreference.com/w/cpp/memory/enable_shared_from_this)

作用如下：

1. `enable_shared_from_this` 提供安全的替用方案，以替代` std::shared_ptr<T>(this)` 这样的表达式（这种不安全的表达式可能会导致 this 被多个互不知晓的所有者析构）

可能的实现：

1. 内部持有一个 mutable weak_ptr<T> weak_this 成员。创建shared_ptr时，由shared_ptr的构造函数对其进行赋值
2. shared_from_this() 方法通过 weak_this 创建并返回一个 shared_ptr。
3. 声明 shared_ptr 为友元类，允许其访问 weak_this。

### integral_constant

[std::integral_constant - cppreference.com](https://zh.cppreference.com/w/cpp/types/integral_constant)

作用如下：

1. 包装特定类型的静态常量

### conditional_t

[std::conditional - cppreference.com](https://zh.cppreference.com/w/cpp/types/conditional)

提供成员 typedef `type`，若 `B` 在编译时为 true 则定义为 `T`，或若 `B` 为 false 则定义为 `F`

### tuple_element

[std::tuple_element - cppreference.com](https://zh.cppreference.com/w/cpp/utility/tuple/tuple_element)

提供对元组元素类型的编译时索引访问

### is_void_v

[std::is_void - cppreference.com](https://zh.cppreference.com/w/cpp/types/is_void)

检查 `T` 是否为 void 类型。

### unique_ptr

成员函数：

1. [返回裸指针 std::unique_ptr::release - cppreference.com](https://zh.cppreference.com/w/cpp/memory/unique_ptr/release)

### promise

[std::promise - cppreference.com](https://zh.cppreference.com/w/cpp/thread/promise)

成员函数：

1. [std::promise::get_future - cppreference.com](https://zh.cppreference.com/w/cpp/thread/promise/get_future)

### future

[std::future - cppreference.com](https://zh.cppreference.com/w/cpp/thread/future)

成员函数：

1. [std::future::wait_for - cppreference.com](https://zh.cppreference.com/w/cpp/thread/future/wait_for)
2. [std::future::get - cppreference.com](https://zh.cppreference.com/w/cpp/thread/future/get)
3. [std::optional::value - cppreference.com](https://zh.cppreference.com/w/cpp/utility/optional/value)

### optional

[std::optional - cppreference.com](https://zh.cppreference.com/w/cpp/utility/optional)

成员函数：

1. [std::optional::operator bool, std::optional::has_value - cppreference.com](https://zh.cppreference.com/w/cpp/utility/optional/operator_bool)

其他相关：

1. [std::nullopt - cppreference.com](https://zh.cppreference.com/w/cpp/utility/optional/nullopt)
1. [销毁任何所含值 std::optional::reset - cppreference.com](https://zh.cppreference.com/w/cpp/utility/optional/reset)

### conditional_variable

[std::condition_variable - cppreference.com](https://zh.cppreference.com/w/cpp/thread/condition_variable)

成员函数：

1. [std::condition_variable::wait - cppreference.com](https://zh.cppreference.com/w/cpp/thread/condition_variable/wait)

2. [std::condition_variable::wait_for - cppreference.com](https://zh.cppreference.com/w/cpp/thread/condition_variable/wait_for)
3. 

### unique_lock

[std::unique_lock - cppreference.com](https://zh.cppreference.com/w/cpp/thread/unique_lock)

可以unlock()，可以配合condition_variable使用

### current_exception

[std::current_exception - cppreference.com](https://zh.cppreference.com/w/cpp/error/current_exception)

若在当前异常处理（典型地在 catch 子句中）中调用，则捕获当前异常对象，并创建一个保有该异常对象副本或到该异常对象引用（依赖于实现）的 [std::exception_ptr](https://zh.cppreference.com/w/cpp/error/exception_ptr)。被引用对象至少在仍有 `exception_ptr` 对象引用它时保持有效。

### intptr_t

足以保有指向 void 的指针的有符号整数类型

### function

[std::function - cppreference.com](https://zh.cppreference.com/w/cpp/utility/functional/function)

### ranges::transform

其中的投影可以是任意可调用对象

## tips

