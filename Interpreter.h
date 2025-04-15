//
// Created by Tom on 24-12-17.
//

#ifndef INTERPRETER_H
#define INTERPRETER_H

#include<stack>
#include<vector>
#include "ByteCode.h"
using namespace std;
//���¼
//ջ֡�����ϵ��£�
//����
//RA:���ص�ַ
//DL:��̬��
//SL:��̬��

class Interpreter
{
    ByteCode I; //ָ��Ĵ���
    int T, B, P; //ָ��ָ�롢ջ��ָ�롢��ַ�Ĵ����������ַ�Ĵ���
    vector<int> S; //����ջ
    vector<ByteCode> codes; //ָ������
    ifstream &fin;

public:
    explicit Interpreter(ifstream& fin): fin(fin)
    {}

    void load() //���ش���
    {
        int f, l, a;
        if (!fin)
        {
            cout<<"�ļ���ʧ�ܣ�"<<endl;
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

    void execute() //ִ�д���
    {
        T = 0; //ջ��ָ��
        B = 0; //��ַָ��
        P = 0; //���������
        do{
            // getchar();
            I = codes[P];
            switch (I.F) //���ݲ�����ִ�в���
            {
            case INT: //����ռ�
                {
                    T += I.A;
                    if (S.size()<T)
                        S.resize(T);
                    P++;
                    break;
                }
            case LIT: //�������ŵ�ջ��
                {
                    T++;
                    if (S.size()<T)
                        S.resize(T);
                    S[T-1]=I.A;
                    P++;
                    break;
                }
            case LOD: //�������ŵ�ջ��
                {
                    int static_link = B; //��ǰջ֡
                    for (int i = 0; i < I.L; i++) //������
                    {
                        static_link = S[static_link]; //���ϻ���
                    }
                    T++; //ջ��ָ��+1
                    if (S.size() < T)
                        S.resize(T);
                    S[T-1] = S[static_link + I.A - 1]; //���ر�������1��ʼ
                    P++; //���������+1
                    break;
                }
            case STO: //��ջ����ֵ�浽����
                {
                    int static_link = B; //��ǰջ֡
                    if (I.L==-1)//�������ݣ���Ҫ�ƶ�
                    {
                        if (S.size()<T+3)
                            S.resize(T+3);
                        S[T+2]=S[T-1];//���������ƶ���Ԥ���ռ�
                        T--;//ջ������
                        P++;
                        break;
                    }
                    for (int i = 0; i < I.L; i++) //������
                    {
                        static_link = S[static_link]; //���ϻ���
                    }
                    S[static_link + I.A-1] = S[T - 1]; //�洢����
                    T--; //ջ��ָ��-1
                    P++; //���������+1
                    break;
                }
            case JMP: //��������ת
                {
                    P = I.A;//��д���������
                    break;
                }
            case JPC: //������ת
                {
                    T--;
                    if (S[T] == 0)
                    {
                        P = I.A; //��д���������
                    }else
                    {
                        P++;
                    }
                    break;
                }
            case RED: //�����ݲ�����ջ��
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
            case WRT: //��ӡ����
                {
                    cout << S[T - 1];
                    T--; //ջ��ָ��-1
                    P++; //���������+1
                    break;
                }
            case CAL://���̵���
                {
                    int static_link=B;//��ǰջ֡
                    for (int i=0;i<I.L;i++)//������
                    {
                        static_link=S[static_link];//���ϻ���
                    }
                    if (S.size() < T + 3)
                        S.resize(T + 3);
                    S[T]=static_link;//��̬�����洢�����¼
                    S[T+1]=B;//��̬�����洢��ǰ���¼
                    S[T+2]=P+1;//���ص�ַ
                    B=T;//��ַָ��ָ��ջ��
                    P=I.A;//��ת���������
                    // T+=3;//ջ��ָ��+3
                    break;
                }
            case OPR: //����
                {
                    switch (I.A)//���ݲ�����ִ�в���
                    {
                    case 0: //����
                        {
						    T = B; //ջ��ָ��ָ���ַ
                            P = S[T + 2]; //���ص�ַ
                            B = S[T + 1]; //��ַ
                            break;
                        }
                    case 1: //ȡ��
                        {
                            S[T - 1] = -S[T - 1];
                            P++;
                            break;
                        }
                    case 2: //�ӷ�
                        {
                            T--;
                            S[T - 1] += S[T];
                            P++;
                            break;
                        }
                    case 3: //����
                        {
                            T--;
                            S[T - 1] -= S[T];
                            P++;
                            break;
                        }
                    case 4: //�˷�
                        {
                            T--;
                            S[T - 1] *= S[T];
                            P++;
                            break;
                        }
                    case 5: //����
                        {
                            T--;
                            S[T - 1] /= S[T];
                            P++;
                            break;
                        }
                    case 6: //��ż�ж�
                        {
                            S[T - 1]%= 2;
                            P++;
                            break;
                        }
                    case 7: //����
                        {
                            T--;
                            S[T - 1] = S[T - 1] == S[T];
                            P++;
                            break;
                        }
                    case 8: //������
                        {
                            T--;
                            S[T - 1] = S[T - 1] != S[T];
                            P++;
                            break;
                        }
                    case 9: //С��
                        {
                            T--;
                            S[T - 1] = S[T - 1] < S[T];
                            P++;
                            break;
                        }
                    case 10: //���ڵ���
                        {
                            T--;
                            S[T - 1] = S[T - 1] >= S[T];
                            P++;
                            break;
                        }
                    case 11: //����
                        {
                            T--;
                            S[T - 1] = S[T - 1] > S[T];
                            P++;
                            break;
                        }
                    case 12: //С�ڵ���
                        {
                            T--;
                            S[T - 1] = S[T - 1] <= S[T];
                            P++;
                            break;
                        }
                    case 13: //��Ļ�������
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
        cout<<"��ǰָ�"<<names[I.F]<<' '<<I.L<<' '<<I.A<<endl;
        cout<<"��ǰ�Ĵ�����"<<"T:"<<T<<" B:"<<B<<" P:"<<P<<endl;
        cout<<"��ǰջ��";
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
            cout << "����Ϊ�գ�" << endl;
            return;
        }
        execute();
    }
};


#endif //INTERPRETER_H
