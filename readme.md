# Crystal Engine 方晶引擎
## 简述
&emsp;Crystal是自CCL以来的全新升级版，在CCL的基础上做出了改进，使其更加简单易用，而且效率更高  
&emsp;和CCL一样，不会提供成体系的简体中文意外的注释和文档。但是如果你英文基础较好，应该能够仅根据变量函数名、结构体、宏定义的命名就推断出功能和用意（它们是有意义且相关的英文词汇组合而成的）
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
&emsp;通常俩说和Crystal关系密切的类型名都会有CR前缀成分来表明关系，以及防止重名。
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
## 使用
&emsp;将Crystal包含进你的项目，你可以决定使用静态库或者动态库，也可以添加自己的第三方模块。官方Release（如果有的话）不会提供静态库版本，想要使用需要自行编译，在使用介绍完毕之后将会讲述编译方法和环境。
&emsp;作者的建议是使用cmake构建项目，也将介绍如何在cmake中使用Crystal。首先：