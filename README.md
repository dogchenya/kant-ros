该工程是Kant基础框架。

目录名称 |功能
----------------------|----------------
kantcpp               |Kant 框架C++语言的源代实现
vs-project            |vs工厂配置
KantNode              |节点用例

kantcpp

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