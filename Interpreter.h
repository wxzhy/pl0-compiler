//
// Created by Tom on 24-12-17.
//

#ifndef INTERPRETER_H
#define INTERPRETER_H

#include<stack>
#include<vector>
#include "ByteCode.h"
using namespace std;
//活动记录
//栈帧（从上到下）
//变量
//RA:返回地址
//DL:动态链
//SL:静态链

class Interpreter
{
    ByteCode I; //指令寄存器
    int T, B, P; //指令指针、栈顶指针、基址寄存器、程序基址寄存器
    vector<int> S; //数据栈
    vector<ByteCode> codes; //指令序列
    ifstream &fin;

public:
    explicit Interpreter(ifstream& fin): fin(fin)
    {}

    void load() //加载代码
    {
        int f, l, a;
        if (!fin)
        {
            cout<<"文件打开失败！"<<endl;
            return;
        }
        while (!fin.eof())
        {
            ByteCode code;
            int f;
            fin>>f>>code.L>>code.A;
            code.F=static_cast<opcode>(f);
            if (!fin.eof())
                codes.push_back(code);
        }
    }

    void execute() //执行代码
    {
        T = 0; //栈顶指针
        B = 0; //基址指针
        P = 0; //程序计数器
        do{
            // getchar();
            I = codes[P];
            switch (I.F) //根据操作码执行操作
            {
            case INT: //分配空间
                {
                    T += I.A;
                    if (S.size()<T)
                        S.resize(T);
                    P++;
                    break;
                }
            case LIT: //将常量放到栈顶
                {
                    T++;
                    if (S.size()<T)
                        S.resize(T);
                    S[T-1]=I.A;
                    P++;
                    break;
                }
            case LOD: //将变量放到栈顶
                {
                    int static_link = B; //当前栈帧
                    for (int i = 0; i < I.L; i++) //处理层差
                    {
                        static_link = S[static_link]; //向上回溯
                    }
                    T++; //栈顶指针+1
                    if (S.size() < T)
                        S.resize(T);
                    S[T-1] = S[static_link + I.A - 1]; //加载变量，从1开始
                    P++; //程序计数器+1
                    break;
                }
            case STO: //将栈顶的值存到变量
                {
                    int static_link = B; //当前栈帧
                    if (I.L==-1)//参数传递，需要移动
                    {
                        if (S.size()<T+3)
                            S.resize(T+3);
                        S[T+2]=S[T-1];//参数向上移动，预留空间
                        T--;//栈顶下移
                        P++;
                        break;
                    }
                    for (int i = 0; i < I.L; i++) //处理层差
                    {
                        static_link = S[static_link]; //向上回溯
                    }
                    S[static_link + I.A-1] = S[T - 1]; //存储变量
                    T--; //栈顶指针-1
                    P++; //程序计数器+1
                    break;
                }
            case JMP: //无条件跳转
                {
                    P = I.A;//改写程序计数器
                    break;
                }
            case JPC: //条件跳转
                {
                    T--;
                    if (S[T] == 0)
                    {
                        P = I.A; //改写程序计数器
                    }else
                    {
                        P++;
                    }
                    break;
                }
            case RED: //读数据并存入栈顶
                {
                    int x;
                    cin >> x;
                    string s;
                    getline(cin, s);
                    T++;
                    if (S.size()<T)
                        S.resize(T);
                    S[T - 1] = x;
                    P++;
                    break;
                }
            case WRT: //打印数据
                {
                    cout << S[T - 1];
                    T--; //栈顶指针-1
                    P++; //程序计数器+1
                    break;
                }
            case CAL://过程调用
                {
                    int static_link=B;//当前栈帧
                    for (int i=0;i<I.L;i++)//处理层差
                    {
                        static_link=S[static_link];//向上回溯
                    }
                    if (S.size() < T + 3)
                        S.resize(T + 3);
                    S[T]=static_link;//静态链，存储外层活动记录
                    S[T+1]=B;//动态链，存储当前活动记录
                    S[T+2]=P+1;//返回地址
                    B=T;//基址指针指向栈顶
                    P=I.A;//跳转到过程入口
                    // T+=3;//栈顶指针+3
                    break;
                }
            case OPR: //运算
                {
                    switch (I.A)//根据操作数执行操作
                    {
                    case 0: //返回
                        {
						    T = B; //栈顶指针指向基址
                            P = S[T + 2]; //返回地址
                            B = S[T + 1]; //基址
                            break;
                        }
                    case 1: //取反
                        {
                            S[T - 1] = -S[T - 1];
                            P++;
                            break;
                        }
                    case 2: //加法
                        {
                            T--;
                            S[T - 1] += S[T];
                            P++;
                            break;
                        }
                    case 3: //减法
                        {
                            T--;
                            S[T - 1] -= S[T];
                            P++;
                            break;
                        }
                    case 4: //乘法
                        {
                            T--;
                            S[T - 1] *= S[T];
                            P++;
                            break;
                        }
                    case 5: //除法
                        {
                            T--;
                            S[T - 1] /= S[T];
                            P++;
                            break;
                        }
                    case 6: //奇偶判断
                        {
                            S[T - 1]%= 2;
                            P++;
                            break;
                        }
                    case 7: //等于
                        {
                            T--;
                            S[T - 1] = S[T - 1] == S[T];
                            P++;
                            break;
                        }
                    case 8: //不等于
                        {
                            T--;
                            S[T - 1] = S[T - 1] != S[T];
                            P++;
                            break;
                        }
                    case 9: //小于
                        {
                            T--;
                            S[T - 1] = S[T - 1] < S[T];
                            P++;
                            break;
                        }
                    case 10: //大于等于
                        {
                            T--;
                            S[T - 1] = S[T - 1] >= S[T];
                            P++;
                            break;
                        }
                    case 11: //大于
                        {
                            T--;
                            S[T - 1] = S[T - 1] > S[T];
                            P++;
                            break;
                        }
                    case 12: //小于等于
                        {
                            T--;
                            S[T - 1] = S[T - 1] <= S[T];
                            P++;
                            break;
                        }
                    case 13: //屏幕输出换行
                        {
                            cout << endl;
                            P++;
                            break;
                        }
                    }
                }
            }
        //debug();
        }while (P);
    }

    void debug()
    {
        cout<<"当前指令："<<names[I.F]<<' '<<I.L<<' '<<I.A<<endl;
        cout<<"当前寄存器："<<"T:"<<T<<" B:"<<B<<" P:"<<P<<endl;
        cout<<"当前栈：";
        for (int i=0;i<T;i++)
        {
            cout<<S[i]<<' ';
        }
        cout<<endl<<endl;
    }

    void run()
    {
        load();
        if (codes.empty())
        {
            cout << "代码为空！" << endl;
            return;
        }
        execute();
    }
};


#endif //INTERPRETER_H
