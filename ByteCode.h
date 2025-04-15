#ifndef BYTECODE_H
#define BYTECODE_H
#include <iostream>
#include <unordered_map>
#include <vector>
using namespace std;
enum opcode
{
    LIT, //将常量放到栈顶
    OPR, //执行运算
    LOD, //将变量放到栈顶
    STO, //将栈顶的值存到变量
    CAL, //调用过程
    INT, //分配空间
    JMP, //无条件跳转
    JPC, //条件跳转
    RED, //读数据并存入变量
    WRT, //写数据
};
unordered_map<opcode,string> names={
    {LIT,"LIT"},
    {OPR,"OPR"},
    {LOD,"LOD"},
    {STO,"STO"},
    {CAL,"CAL"},
    {INT,"INT"},
    {JMP,"JMP"},
    {JPC,"JPC"},
    {RED,"RED"},
    {WRT,"WRT"}
};

struct ByteCode {
    opcode F;//操作码
    int L;//调用层与声明层的层差
    int A;//操作数
};

class Assembly//汇编代码
{
public:
    vector<ByteCode> codes;
    int cx = 0;//代码指针
    Assembly(){};//构造函数
    void emit(opcode F, int L, int A)//生成代码
    {
        codes.emplace_back(ByteCode{F, L, A});
        // print();
    }
    void emit(opcode F, int A)//生成代码
    {
        codes.emplace_back(ByteCode{F, 0, A});
    }
    void backpatch(int addr, int A)//回填
    {
        codes[addr].A = A;
    }
    int getSize()
    {
        return codes.size();
    }
    void print()
    {
        int cx=0;
        for (auto &code : codes)
        {
            cout << cx << ' ' << names[code.F] << ' ' << code.L << ' ' << code.A << endl;
            cx++;
        }
    }
};



#endif //BYTECODE_H
