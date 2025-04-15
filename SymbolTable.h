#ifndef SYMBOL_H
#define SYMBOL_H
#include <list>
#include <string>
#include <vector>
#include <iostream>
using namespace std;

enum symbol_type
{
    symbol_null, //空
    symbol_variable, //变量
    symbol_constant, //常量
    symbol_parameter, //参数
    symbol_procedure, //过程
};

struct symbol //符号
{
    string name; //符号名
    symbol_type type; //符号类型
    int value; //常量的值
    int offset;//变量、过程的地址
    int parameter;//参数个数，用于过程
    int level; //层次
};

class Frame //帧
{
    vector<symbol> symbols; //符号表
    string name; //帧名
    int parameter; //参数个数
    int size; //大小，默认包含SL、DL、RA
    int level; //层次
public:
    Frame(string name): parameter(0),size(3),level(0),name(name)
    {
    }

    Frame(string name,int level): parameter(0),size(3),level(level),name(name)
    {}

    void addSymbol(symbol_type type, string name, int value)
    {
        if (type==symbol_variable||type==symbol_parameter)//变量或参数需要申请空间
        {
            size++;
            symbols.emplace_back(symbol{name, type, value, size, 0 ,level});
        }else if (type==symbol_procedure)//过程
        {
            symbols.emplace_back(symbol{name, type, 0, value, 0,level});
        }
        else//常量
        {
            symbols.emplace_back(symbol{name, type, value, 0,0,level});
        }
    }

    symbol lookup(string name) //查找符号，返回所在位置
    {
        for (auto it = symbols.rbegin(); it != symbols.rend(); it++)
        {
            if (it->name == name)
            {
                return *it; //返回符号对象
            }
        }
        return {name, symbol_null, 0, 0};
    }

    bool add(symbol_type type, string name, int value) //添加符号
    {
        if (lookup(name).type != symbol_null) //符号重复定义
        {
            cout << "错误:符号" << name << "重复定义" << endl;
            return false;
        }
        addSymbol(type, name, value);
        return true;
    }

    bool updateParameter(string name)//更新参数个数
    {
        if (symbols.empty())
        {
            return false;
        }
        for (auto it = symbols.rbegin(); it != symbols.rend(); it++)
        {
            if (it->name == name)
            {
                it->parameter++;
                return true;
            }
        }
        return false;
    }

    int getLevel() const
    {
        return level;
    }

    int getSize() const
    {
        return size;
    }

    int getParameter() const
    {
        return parameter;
    }

    string getName() const
    {
        return name;
    }

    bool setEntry(string name,int entry)
    {
        for (auto it = symbols.rbegin(); it != symbols.rend(); it++)
        {
            if (it->type == symbol_procedure)
            {
                if (it->name == name)
                {
                    it->offset = entry;
                    return true;
                }
            }
        }
        return false;
    }

    void print()//打印当前帧的符号表
    {
        cout << "level:" << level << endl;
        for (auto& s : symbols)
        {
            cout << "name:" << s.name << " type:" << s.type << " value:" << s.value << " offset:" << s.offset << " parameter:" << s.parameter << " level:" << s.level << endl;
        }
    }

};

class SymbolTable
{
    list<Frame> frames; //帧栈

public:
    SymbolTable()
    {
    } //初始化
    bool addSymbol(symbol_type type, string name, int value) //添加符号
    {
        auto &cur = frames.back();
        if (cur.add(type, name, value)) //在当前帧添加
        {
            if (type==symbol_parameter)//添加参数，需要更新上一层的参数个数
            {
                auto prev = frames.rbegin();
                ++prev;
                if(prev->updateParameter(cur.getName()))//更新参数个数
                    return true;
                return false;
            }
            return true;
        }
        return false;
    }

    void addScope(string name) //添加作用域
    {
        frames.emplace_back(name,frames.size()+1);
    }

    void delScope() //删除作用域
    {
        frames.pop_back();
    }

	int getLevel() const //返回当前层次
    {
        return frames.size();
    }

	bool setEntry(int entry)//设置入口
    {
        if (frames.size()<=1)//只有一个帧，为program
        {
            return false;
        }
        auto pre=frames.rbegin();
        ++pre;
        return pre->setEntry(frames.back().getName(),entry);

    }

    symbol lookup(string name) //查找符号
    {
        for (auto it = frames.rbegin(); it != frames.rend(); ++it) //从栈顶开始查找
        {
            auto pos = it->lookup(name);
            if (pos.type!=symbol_null) //找到
            {
                return pos;//返回符号对象
            }
        }
        return {name, symbol_null, 0, 0,0,0};
    }
    int getSize()//返回当前帧大小
    {
        return frames.back().getSize();
    }

    void print()
    {
        for (auto& f : frames)
        {
            f.print();
        }
    }
};


#endif //SYMBOL_H
