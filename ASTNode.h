//
// Created by Tom on 24-11-20.
//

#ifndef AST_H
#define AST_H
#include <string>
#include <utility>
#include <vector>
#include <iostream>
using namespace std;
enum ast_type//语法树节点类型
{
    ast_program,//<prog>->program <id>;<block>
    ast_block,//<block>->[<condecl>][<vardecl>][<proc>]<body>
    ast_body,//<body>->begin <statement>{;<statement>} end
    //<condecl>->const <const>{,<const>};
    ast_condecl,
    ast_const,
    //<vardecl>->var <id>{,<id>};
    ast_vardecl,
    ast_var,
    //<proc>->procedure <id> ([parameter]);<block>{;<proc>}
    ast_proc,
    ast_procedure,
    ast_parameter,//<parameter>-><id>{,<id>}
    //statement
    ast_assign,//<statement>-><id>:=<exp>
    ast_if,//<statement>->if <lexp> then <statement>[else <statement>]
    ast_while,//<statement>->while <lexp> do <statement>
    ast_call,//<statement>- >call <id>[argument]
    ast_argument,//<argument>-><exp>{,<exp>}
    ast_read,//<statement>->read (<id>{,<id>})
    ast_write,//<statement>->write(<exp>{,<exp>})
    //logical expression
    ast_eq,//<lexp>-><exp>=<exp>
    ast_neq,//<lexp>-><exp> <> <exp>
    ast_lss,//<lexp>-><exp> < <exp>
    ast_geq,//<lexp>-><exp> >= <exp>
    ast_gtr,//<lexp>-><exp> > <exp>
    ast_leq,//<lexp>-><exp> <= <exp>
    ast_odd,//<lexp>->odd <exp>
    //expression
    ast_unary_plus,//<exp>->+<term>
    ast_unary_minus,//<exp>->-<term>
    ast_plus,//<exp>->[+|-]<term>{+<term>}
    ast_minus,//<exp>->[+|-]<term>{-<term>}
    //term
    ast_times,//<term>-><factor>{*<factor>}
    ast_slash,//<term>-><factor>{/<factor>}
    //factor
    ast_identifier,//标识符
    ast_number//数字
};
unordered_map<ast_type,string> node_names={//提示名称，用于显示
    {ast_program,"program"},
    {ast_block,"block"},
    {ast_body,"body"},
    {ast_condecl,"condecl"},
    {ast_const,"const"},
    {ast_vardecl,"vardecl"},
    {ast_var,"var"},
    {ast_proc,"proc"},
    {ast_procedure,"procedure"},
    {ast_parameter,"parameter"},
    {ast_assign,"assign"},
    {ast_if,"if"},
    {ast_while,"while"},
    {ast_call,"call"},
    {ast_argument,"argument"},
    {ast_read,"read"},
    {ast_write,"write"},
    {ast_eq,"="},
    {ast_neq,"<>"},
    {ast_lss,"<"},
    {ast_leq,"<="},
    {ast_gtr,">"},
    {ast_geq,">="},
    {ast_odd,"odd"},
    {ast_plus,"+"},
    {ast_minus,"-"},
    {ast_times,"*"},
    {ast_slash,"/"},
    {ast_identifier,"id"},
    {ast_number,"integer"},
    {ast_unary_minus,"unary_minus"},
    {ast_unary_plus,"unary_plus"}
};


class ASTNode {//抽象语法树节点
public:
    ast_type type;//AST结点类型
    string value;//结点值
    vector<ASTNode *> children;//子结点
    int line{};//行号
    int column{};//列号
    explicit ASTNode(ast_type type, string value, int line, int column): type(type), value(move(value)), line(line), column(column) {}
    ASTNode(ast_type type, string value): type(type), value(move(value)) {}
    ASTNode(ast_type type, int line, int column): type(type), line(line), column(column) {}
    explicit ASTNode(ast_type type): type(type) {}

    void addChild(ASTNode *child)
    {
        if(child)//忽略错误产生的空结点
        {
            children.push_back(child);
        }
    }

    void print(int depth)//打印语法树
    {
        for (int i = 0; i < depth; i++)
        {
            cout << "｜";
        }
        cout <<node_names[type]<<":　"<< value << endl;
        for (auto& child: children)
        {
            child->print(depth + 1);
        }
    }
};



#endif //AST_H
