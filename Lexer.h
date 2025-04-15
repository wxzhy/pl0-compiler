    #ifndef LEXER_H
    #define LEXER_H
    #include <string>
    #include <fstream>
    #include <iostream>
    #include "Token.h"
    using namespace std;
    class Lexer//PL0�ʷ�����
    {
        char ch = EOF;
        string strToken;
        ifstream& fin;
        int line = 1, column = 0;

        void GetChar() //��ȡһ���ַ�
        {
            if (!fin.eof())
            {
                fin.get(ch);
                if (fin.eof()) //�����ļ�ĩβ
                {
                    ch = EOF;
                    return;
                }
                column++;
                if (ch == '\n')
                {
                    line++;
                    column = 0;
                }
            }
            else
                ch = EOF;

            // cout << "current:" << ch << endl;
        }

        void GetBC() //�����հ��ַ�
        {
            while (ch == ' ' || ch == '\n' || ch == '\t')
            {
                GetChar();
            }
        }

        void Concat() //�����ַ�
        {
            strToken += ch;
        }

        bool IsLetter() const //�ж��Ƿ�Ϊ��ĸ
        {
            return isalpha(ch);
        }

        bool IsDigit() const //�ж��Ƿ�Ϊ����
        {
            return isdigit(ch);
        }

        token_type Reserve() //���ұ�����
        {
            if (reservedWords.find(strToken) != reservedWords.end())
            {
                return reservedWords[strToken];
            }
            //���Ǳ����ַ���0
            return token_null;
        }

        void Retract() //����һ���ַ�
        {
            fin.unget();
            if (ch == '\n') {
                line--;
            }
            else {
                column--;
            }
        }

        Token GetToken() //��ȡһ��token
        {
            strToken.clear(); //����ַ���
            //��ȡֱ�����ǿհ��ַ�
            GetChar();
            GetBC();
            //<id>-><letter>{<letter>|<digit>}
            if (IsLetter())
            {
                while (IsLetter() || IsDigit())
                {
                    Concat();
                    GetChar();
                }
                Retract();
                //���ұ�����
                token_type num = Reserve();
                if (num != token_null)
                {
                    return {num, strToken, line, column};
                }
                return {token_id, strToken, line, column};
            }
            //<integer>->digit{digit}
            if (IsDigit())
            {
                while (IsDigit())
                {
                    Concat();
                    GetChar();
                }
                if(IsLetter())//���ִ����쳣����ĸ
                {
                    while (IsDigit() || IsLetter())//������������
                    {
                        Concat();
                        GetChar();
                    }
                    Retract();
                    return Token(token_null);
                }
                Retract();
                return {token_integer, strToken, line, column};
            }
            Concat();
            switch (ch)
            {
            //���ַ�
            //<aop>->+|-
            case '+': return {token_plus, strToken, line, column};;
            case '-': return {token_minus, strToken, line, column};
            //<mop>->*|/
            case '*': return {token_times, strToken, line, column};
            case '/': return {token_slash, strToken, line, column};
            //<lop>-> =|<>|< |<= |  >|>=
            case '=': return {token_eq, strToken, line, column};
            case '<':
                GetChar();
                if (ch == '=')
                {
                    Concat();
                    return {token_leq, strToken, line, column};
                }
                if (ch == '>')
                {
                    Concat();
                    return {token_neq, strToken, line, column};
                }
                Retract();
                return {token_lss, strToken, line, column};
            case '>':
                GetChar();
                if (ch == '=')
                {
                    Concat();
                    return {token_geq, strToken, line, column};
                }
                Retract();
                return {token_gtr, strToken, line, column};
            case '(':
                return {token_lparen, strToken, line, column};
            case ')':
                return {token_rparen, strToken, line, column};
            case ',':
                return {token_comma, strToken, line, column};
            case ';':
                return {token_semicolon, strToken, line, column};
            case ':':
                GetChar();
                if (ch == '=')
                {
                    Concat();
                    return {token_assign, strToken, line, column};
                }
                Retract();
                return {token_null, strToken, line, column};
            case EOF:
                return {token_eof, strToken, line, column};
            default:
                {
                    error();
                    return Token(token_null);
                }
            }
        }

        void error() //������
        {
            //����gbk�ַ�
            if (ch > 0x80)
            {
                GetChar();
                Concat();
            }
            //��ӡ������Ϣ
               cout << "���󣺵�" << line << "�У���" << column << "�У��Ƿ��ַ�" << strToken <<endl;
        }

    public:
        explicit Lexer(ifstream& fin): fin(fin)
        {
            if(!fin)
            {
                cout << "�ļ���ʧ�ܣ�" << endl;
                return;
            }
        }

        void getAll(ofstream &fout) //��ӡ����
        {
            Token t;
            if (!fout)
            {
                cout << "�ļ���ʧ�ܣ�" << endl;
                return;
            }
            do
            {
                t = GetToken();
                if (t.type == token_null)
                {
                    error();
                    continue;
                }
                t.print();


                fout<<t.type<<' '<<t.value<<' '<<t.line<<' '<<t.column<<endl;
            }
            while (t.type != token_eof);
        }
    };


    #endif //LEXER_H
