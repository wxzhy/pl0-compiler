//
// Created by Tom on 24-11-10.
//

#ifndef TOKEN_H
#define TOKEN_H
#include <string>
#include <unordered_map>
#include<iomanip>
using namespace std;
/*可能的token:
 * program const var procedure begin end if then while do call read write odd
 * = <> < <= > >= + - * / ( ) , ; :=
 * id integer
 */
enum token_type
{
    token_null,
    token_program,//<prog>->program <id>;<block>
    token_const,//<condecl>->const <const>{,<const>};
    token_var,//<vardecl>->var <id>{,<id>};
    token_procedure,//<proc>->procedure <id> ([<id>{,<id>}]);<block>{;<proc>}
    //<body>->begin <statement>{;<statement>} end
    token_begin,
    token_end,
    token_assign,//<statement>-><id>:=<exp>
    //<statement>->if <lexp> then <statement>[else <statement>]
    token_if,
    token_then,
    token_else,
    //<statement>->while <lexp> do <statement>
    token_while,
    token_do,
    token_call,//<statement>->call <id>[<exp>{,<exp>}]
    token_read,//<statement>->read (<id>{,<id>})
    token_write,//<statement>->write(<exp>{,<exp>})
    //<lop>-> =|<>|< |<= |  >|>=
    token_eq,
    token_neq,
    token_lss,
    token_leq,
    token_gtr,
    token_geq,
    //<lexp>->odd <exp>
    token_odd,
    //<aop>->+|-
    token_plus,
    token_minus,
    //<mop>->*|/
    token_times,
    token_slash,
    //符号
    token_lparen,//(
    token_rparen,//)
    token_comma,//,
    token_semicolon,//;
    //<factor>-><id>|<integer>|(<exp>)
    token_id,
    token_integer,
    token_eof
};

//保留字表
inline unordered_map<string, token_type> reservedWords = {
    {"program", token_program},
    {"const", token_const},
    {"var", token_var},
    {"procedure", token_procedure},
    {"begin", token_begin},
    {"end", token_end},
    {"if", token_if},
    {"then", token_then},
    {"else", token_else},
    {"while", token_while},
    {"do", token_do},
    {"call", token_call},
    {"read", token_read},
    {"write", token_write},
    {"odd", token_odd}
};
//名称，用于输出
unordered_map<token_type,string> friendlyNames={
    {token_null,"null"},
    {token_program,"program"},
    {token_const,"const"},
    {token_var,"var"},
    {token_procedure,"procedure"},
    {token_begin,"begin"},
    {token_end,"end"},
    {token_assign,":="},
    {token_if,"if"},
    {token_then,"then"},
    {token_else,"else"},
    {token_while,"while"},
    {token_do,"do"},
    {token_call,"call"},
    {token_read,"read"},
    {token_write,"write"},
    {token_eq,"="},
    {token_neq,"<>"},
    {token_lss,"<"},
    {token_leq,"<="},
    {token_gtr,">"},
    {token_geq,">="},
    {token_odd,"odd"},
    {token_plus,"+"},
    {token_minus,"-"},
    {token_times,"*"},
    {token_slash,"/"},
    {token_lparen,"("},
    {token_rparen,")"},
    {token_comma,","},
    {token_semicolon,";"},
    {token_id,"id"},
    {token_integer,"integer"},

};

class Token//token类
{
public:
    token_type type; //类型
    string value; //值
    int line{}, column{}; //行号，列号
    Token(token_type t, string v, int l, int c): type(t), value(move(v)), line(l), column(c)
    {
    }

    Token(token_type t, string v): type(t), value(move(v))
    {
    }

    Token(token_type t, int l, int c): type(t), line(l), column(c)
    {
    }

    explicit Token(token_type t): type(t), line(0), column(0)
    {
    }

    Token(): type(token_null), line(0), column(0)
    {
    }

    void print() const
    {
		cout << setw(10) << friendlyNames[type] << setw(5) << line << setw(5) << column << setw(15) << value << endl;
    }
};


#endif //TOKEN_H
