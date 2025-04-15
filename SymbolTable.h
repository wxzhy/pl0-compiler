#ifndef SYMBOL_H
#define SYMBOL_H
#include <list>
#include <string>
#include <vector>
#include <iostream>
using namespace std;

enum symbol_type
{
    symbol_null, //��
    symbol_variable, //����
    symbol_constant, //����
    symbol_parameter, //����
    symbol_procedure, //����
};

struct symbol //����
{
    string name; //������
    symbol_type type; //��������
    int value; //������ֵ
    int offset;//���������̵ĵ�ַ
    int parameter;//�������������ڹ���
    int level; //���
};

class Frame //֡
{
    vector<symbol> symbols; //���ű�
    string name; //֡��
    int parameter; //��������
    int size; //��С��Ĭ�ϰ���SL��DL��RA
    int level; //���
public:
    Frame(string name): parameter(0),size(3),level(0),name(name)
    {
    }

    Frame(string name,int level): parameter(0),size(3),level(level),name(name)
    {}

    void addSymbol(symbol_type type, string name, int value)
    {
        if (type==symbol_variable||type==symbol_parameter)//�����������Ҫ����ռ�
        {
            size++;
            symbols.emplace_back(symbol{name, type, value, size, 0 ,level});
        }else if (type==symbol_procedure)//����
        {
            symbols.emplace_back(symbol{name, type, 0, value, 0,level});
        }
        else//����
        {
            symbols.emplace_back(symbol{name, type, value, 0,0,level});
        }
    }

    symbol lookup(string name) //���ҷ��ţ���������λ��
    {
        for (auto it = symbols.rbegin(); it != symbols.rend(); it++)
        {
            if (it->name == name)
            {
                return *it; //���ط��Ŷ���
            }
        }
        return {name, symbol_null, 0, 0};
    }

    bool add(symbol_type type, string name, int value) //��ӷ���
    {
        if (lookup(name).type != symbol_null) //�����ظ�����
        {
            cout << "����:����" << name << "�ظ�����" << endl;
            return false;
        }
        addSymbol(type, name, value);
        return true;
    }

    bool updateParameter(string name)//���²�������
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

    void print()//��ӡ��ǰ֡�ķ��ű�
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
    list<Frame> frames; //֡ջ

public:
    SymbolTable()
    {
    } //��ʼ��
    bool addSymbol(symbol_type type, string name, int value) //��ӷ���
    {
        auto &cur = frames.back();
        if (cur.add(type, name, value)) //�ڵ�ǰ֡���
        {
            if (type==symbol_parameter)//��Ӳ�������Ҫ������һ��Ĳ�������
            {
                auto prev = frames.rbegin();
                ++prev;
                if(prev->updateParameter(cur.getName()))//���²�������
                    return true;
                return false;
            }
            return true;
        }
        return false;
    }

    void addScope(string name) //���������
    {
        frames.emplace_back(name,frames.size()+1);
    }

    void delScope() //ɾ��������
    {
        frames.pop_back();
    }

	int getLevel() const //���ص�ǰ���
    {
        return frames.size();
    }

	bool setEntry(int entry)//�������
    {
        if (frames.size()<=1)//ֻ��һ��֡��Ϊprogram
        {
            return false;
        }
        auto pre=frames.rbegin();
        ++pre;
        return pre->setEntry(frames.back().getName(),entry);

    }

    symbol lookup(string name) //���ҷ���
    {
        for (auto it = frames.rbegin(); it != frames.rend(); ++it) //��ջ����ʼ����
        {
            auto pos = it->lookup(name);
            if (pos.type!=symbol_null) //�ҵ�
            {
                return pos;//���ط��Ŷ���
            }
        }
        return {name, symbol_null, 0, 0,0,0};
    }
    int getSize()//���ص�ǰ֡��С
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
