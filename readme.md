# Crystal Engine 晶核引擎
## 协议
&emsp;使用GPL3.0协议，详见LICENSE文件。
## 简述
&emsp;Crystal是自CCL以来的全新升级版游戏引擎，在CCL的基础上做出了改进，使其更加简单易用，而且效率更高  
&emsp;和CCL一样，不会提供成体系的,简体中文以外的注释和文档。但是如果你英文基础较好，应该能够仅根据变量函数名、结构体、宏定义的命名就推断出功能和用意（它们是有意义且相关的英文词汇组合而成的）
## 定位
&emsp;Crystal是新一代游戏引擎（豆腐店自用）（^^  
所以说别拉出去比了，作者并没有去秋名山飙车的兴趣，只想安安静静做一个（或者很多个）自己玩的游戏而已。
## 命名规范
所有的**类型名**使用全大写，不得包含下划线，例如:
~~~C
typedef void* CRLVOID;
typedef CRLVOID CRSTRUCTURE;
typedef CRSTRUCTURE* PCRSTRUCTURE;
~~~
&emsp;通常来说和Crystal关系密切的类型名都会有CR前缀成分来表明关系，以及防止重名。
***
所有的**函数名**和**函数指针定义**采用每个单词首字母大写的方案，例如：
~~~C
CRAPI CRSTRUCTURE CRDynamic();
typedef void(*DSCallback)(CRLVOID data)
~~~
&emsp;假如此函数（或函数指针）有明显的功能性，就会优先将关联的功能缩写作为前缀：DSCallback（**D**-ata-**S**-tructure）。
&emsp;一般来说，和Crystal相关的函数和函数指针都带有CR前缀
***
所有的**宏定义**采用大写，意义单元之间采用下划线“_”隔开，例如：
~~~C
#define CRV_MAJOR 0
#define CRV_MINOR 0
~~~
&emsp;大部分和Crystal相关的宏定义会有CR前缀
***
所有的**内部函数**命名采用首位下划线格式，意义单元采用下划线“_”隔开，不得使用大写字母，例如：
~~~C
static void _clear_callback_(CRLVOID data)
{
    //...
}
static void _on_close_(void)
{
	//...
}
~~~
&emsp;内部函数实际上不会暴露在用户环境中，但是为了在编写源代码时易于区分，使用自己的命名方式规范
## 演示
&emsp;将Crystal包含进你的项目，你可以决定使用静态库或者动态库，也可以添加自己的第三方模块。官方Release（如果有的话）不会提供静态库版本，想要使用需要自行编译，在使用介绍完毕之后将会讲述编译方法和环境。  
&emsp;作者的建议是使用cmake构建项目，也将介绍如何在Linux-Cmake中使用Crystal。首先：
~~~shell
#进入Crystal的项目根目录，然后执行：
bash Autobuild.sh
#以上命令有问题建议进入vim将doc文件格式改为unix
#:set ff=unix

#之后在。/build/bin/文件夹中会有生成的二进制文件
cd ./build/bin/
ls -al

#运行自带Demo
./Demo

#然后你会看到这样的程序字符界面：
Demo$:
#键入一个演示并回车：
Demo$:Demo3
~~~
&emsp;尝试一下里面官方自带的演示（Demo1、Demo2、Demo3...），然后你就知道这个引擎目前大概能干些什么了，二进制文件全部放在bin目录里面，你可以将其拷贝出来放到你的项目里面，头文件全部都在include文件夹里面，记得包含进去。  
&emsp;**Demo列表：**  
&emsp;Demo1-网络相关功能演示(完整的演示至少需要两个运行中实体)  
&emsp;Demo2-数据处理功能演示其一  
&emsp;Demo3-图像及UI系列功能演示  
&emsp;Demo4-音频功能演示
***
## 配置