该工程是Kant基础框架。

目录名称 |功能
----------------------|----------------
kantcpp               |Kant 框架C++语言的源代实现
vs-project            |vs工程配置
KantNode              |节点用例

kantcpp
特点：跨平台
util
注释
```
KT_Exception：异常基类
KT_File：文件处理类
KT_Thread：线程类(底层直接封装了c++11 thread, 从而跨平台兼容)
KT_ThreadQueue：线程队列类
KT_TimeProvider：时间提供类
KT_Coroutine：协程类
```
servant
```mermaid
flowchart TD;
     A-->B;
     A-->C;
     B-->D;
     C-->D;
```

Compile and install
```
ros编译: 
catkin_make --source ./src
KantFramework编译: 
mkdir build
cmake ..
make -j4
make install
```
