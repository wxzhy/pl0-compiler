    #ifndef LEXER_H
    #define LEXER_H
    #include <string>
    #include <fstream>
    #include <iostream>
    #include "Token.h"
    using namespace std;
    class Lexer//PL0词法分析
    {
        char ch = EOF;
        string strToken;
        ifstream& fin;
        int line = 1, column = 0;

        void GetChar() //读取一个字符
        {
            if (!fin.eof())
            {
                fin.get(ch);
                if (fin.eof()) //处理文件末尾
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

        void GetBC() //跳过空白字符
        {
            while (ch == ' ' || ch == '\n' || ch == '\t')
            {
                GetChar();
            }
        }

        void Concat() //连接字符
        {
            strToken += ch;
        }

        bool IsLetter() const //判断是否为字母
        {
            return isalpha(ch);
        }

        bool IsDigit() const //判断是否为数字
        {
            return isdigit(ch);
        }

        token_type Reserve() //查找保留字
        {
            if (reservedWords.find(strToken) != reservedWords.end())
            {
                return reservedWords[strToken];
            }
            //不是保留字返回0
            return token_null;
        }

        void Retract() //回退一个字符
        {
            fin.unget();
            if (ch == '\n') {
                line--;
            }
            else {
                column--;
            }
        }

        Token GetToken() //获取一个token
        {
            strToken.clear(); //清空字符串
            //读取直到不是空白字符
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
                //查找保留字
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
                if(IsLetter())//出现错误，异常的字母
                {
                    while (IsDigit() || IsLetter())//读完整个单词
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
            //单字符
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

        void error() //错误处理
        {
            //处理gbk字符
            if (ch > 0x80)
            {
                GetChar();
                Concat();
            }
            //打印错误信息
               cout << "错误：第" << line << "行，第" << column << "列，非法字符" << strToken <<endl;
        }

    public:
        explicit Lexer(ifstream& fin): fin(fin)
        {
            if(!fin)
            {
                cout << "文件打开失败！" << endl;
                return;
            }
        }

        void getAll(ofstream &fout) //打印所有
        {
            Token t;
            if (!fout)
            {
                cout << "文件打开失败！" << endl;
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
