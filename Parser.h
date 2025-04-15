#ifndef PARSER_H
#define PARSER_H
#include <cmath>
#include <iostream>
#include<string>
#include "ASTNode.h"
#include "ByteCode.h"
#include "SymbolTable.h"
#include "Token.h"
using namespace std;

class Parser //PL0语法分析和语法树生成
{
    ifstream& fin;
    Token token;
    ASTNode* root; //语法树根节点
    SymbolTable table; //符号表
    Assembly assembly; //汇编代码
    void GetToken()
    {
        //文件中读取token
        if (fin.eof())
        {
            token.type = token_eof;
            return;
        }
        int t;
        fin >> t >> token.value >> token.line >> token.column;
        token.type = static_cast<token_type>(t);
        // token.print();
    }

    void panic() //跳过错误部分，直到同步点
    {
        //可能的同步点:
        //; , end )
        while (token.type != token_eof)
        {
            GetToken();
            if (token.type == token_semicolon || token.type == token_comma || token.type == token_end || token.type ==
                token_rparen)
            {
                GetToken(); //跳过同步点本身
                return;
            }
        }
    }

    ASTNode* match(token_type type) //匹配,如果是则返回节点,否则报错
    {
        if (token.type == type)
        {
            ASTNode* node = nullptr;
            if (token.type == token_id)
            {
                node = new ASTNode(ast_identifier, token.value, token.line, token.column);
            }
            else if (token.type == token_integer)
            {
                node = new ASTNode(ast_number, token.value, token.line, token.column);
            }
            GetToken();
            return node;
        }
        else
        {
            cout << "错误：第" << token.line << "行，第" << token.column << "列，应为" << friendlyNames[type] << endl;
            panic();
            return nullptr;
        }
    }

    bool expect(token_type type) //期望,如果是则跳过
    {
        if (token.type == type)
        {
            GetToken();
            return true;
        }
        return false;
    }

    // <program>->program<id>;<block>
    void parse_program() //处理程序，入口
    {
        GetToken();
        match(token_program);
        root = new ASTNode(ast_program, token.line, token.column);
        root->addChild(match(token_id));
        table.addScope(root->children[0]->value); //添加作用域
        match(token_semicolon);
        root->addChild(parse_block());
    }

    // <block>->[<condecl>][<vardecl>][<procedure>]<body>
    ASTNode* parse_block() //语句块
    {
        auto* block = new ASTNode(ast_block, token.line, token.column);
        int entry = assembly.getSize(); //记录入口地址
        assembly.emit(JMP, 0, 0); //生成跳转指令，跳转入口地址
        if (token.type == token_const)
        {
            block->addChild(parse_condecl()); //常量定义
        }
        if (token.type == token_var)
        {
            block->addChild(parse_vardecl()); //变量定义
        }
        if (token.type == token_procedure)
        {
            auto* proc = new ASTNode(ast_proc, token.line, token.column);
            block->addChild(proc);
            do
            {
                parse_procedure(proc);
            }
            while (expect(token_semicolon));
        }
        assembly.backpatch(entry, assembly.getSize()); //回填入口地址
        table.setEntry(assembly.getSize()); //设置过程的入口地址
        assembly.emit(INT, 0, table.getSize()); //生成分配空间指令
        block->addChild(parse_body());
        table.delScope(); //删除作用域
        assembly.emit(OPR, 0, 0); //生成返回指令
        return block;
    }

    // <condecl>->const<const>{,<const>};
    ASTNode* parse_condecl() //处理常量声明
    {
        auto* condecl = new ASTNode(ast_condecl, token.line, token.column);
        match(token_const);
        do
        {
            condecl->addChild(parse_const());
        }
        while (expect(token_comma));
        match(token_semicolon);
        return condecl;
    }

    // <const>-><id>=<integer>
    ASTNode* parse_const() //处理常量声明
    {
        auto* con = new ASTNode(ast_const, token.line, token.column);
        con->addChild(match(token_id));
        match(token_assign);
        con->addChild(match(token_integer));
        if (con->children.size() == 2) //添加符号到符号表
        {
            table.addSymbol(symbol_constant, con->children[0]->value, stoi(con->children[1]->value)); //添加常量到符号表
        }
        return con;
    }

    // <vardecl>->var<id>{,<id>};
    ASTNode* parse_vardecl() //处理变量声明
    {
        auto* vardecl = new ASTNode(ast_vardecl, token.line, token.column);
        match(token_var);
        do
        {
            vardecl->addChild(match(token_id));
            table.addSymbol(symbol_variable, vardecl->children.back()->value, 0); //添加变量到符号表
        }
        while (expect(token_comma));
        match(token_semicolon);
        return vardecl;
    }

    // <procedure>->procedure<id>[<parameter>];<block>
    void parse_procedure(ASTNode* parent) //添加过程节点，参数为父节点
    {
        auto* procedure = new ASTNode(ast_procedure, token.line, token.column);
        match(token_procedure);
        procedure->addChild(match(token_id));
        if (!procedure->children.size()) 
        {
            panic();
            return;
        }
         if (table.addSymbol(symbol_procedure, procedure->children[0]->value, assembly.getSize())) //添加过程到符号表
        {
            table.addScope(procedure->children[0]->value); //添加作用域
        }
        else
        {
            panic();
            return;
        }
        match(token_lparen);
        auto* params = new ASTNode(ast_parameter, token.line, token.column);
        //处理函数参数
        if (token.type == token_id)
        {
            procedure->addChild(params);
            do
            {
                params->addChild(match(token_id));
                table.addSymbol(symbol_parameter, params->children.back()->value, 0); //添加参数到符号表
            }
            while (expect(token_comma));
        }
        match(token_rparen);
        match(token_semicolon);
        procedure->addChild(parse_block());
        parent->addChild(procedure);

    }

    // <body>->begin<statement>{;<statement>}end
    ASTNode* parse_body() //处理body语句
    {
        match(token_begin);
        auto* body = new ASTNode(ast_body, token.line, token.column);
        do
        {
            body->addChild(parse_statement());
        }
        while (expect(token_semicolon));
        match(token_end);
        return body;
    }

    ASTNode* parse_statement() //处理statement语句
    {
        switch (token.type)
        {
        case token_id:
            return parse_assign();
        case token_if:
            return parse_if();
        case token_while:
            return parse_while();
        case token_call:
            return parse_call();
        case token_begin:
            return parse_body();
        case token_read:
            return parse_read();
        case token_write:
            return parse_write();
        default:
            {
                cout << "错误：第" << token.line << "行，第" << token.column << "列，非法语句" << endl;
                panic();
                return nullptr;
            }
        }
    }

    // <statement>-><id>:=<exp>
    ASTNode* parse_assign() //处理赋值语句
    {
        auto* con = new ASTNode(ast_assign, token.line, token.column);
        con->addChild(match(token_id));
        match(token_assign);
        con->addChild(parse_exp());
        if (con->children.size() == 2) //添加符号到符号表
        {
            auto pos = table.lookup(con->children[0]->value); //查找变量
            if (pos.type == symbol_null)
            {
                cout << "错误：第" << token.line << "行，第" << token.column << "列，未定义的变量" << con->children[0]->value << endl;
                panic();
                return nullptr;
            }
            else if (pos.type ==symbol_procedure)
            {
                cout << "错误：第" << token.line << "行，第" << token.column << "列，非法赋值" << con->children[0]->value << endl;
                panic();
                return nullptr;
            }
            assembly.emit(STO, table.getLevel() - pos.level, pos.offset); //生成存储指令
        }
        return con;
    }

    // <statement>->if<lexp>then<statement>[else<statement>]
    ASTNode* parse_if() //处理if语句
    {
        auto* ifnode = new ASTNode(ast_if, token.line, token.column);
        match(token_if);
        ifnode->addChild(parse_lexp());
        int addr = assembly.getSize(); //记录地址
        assembly.emit(JPC, 0, 0); //生成条件跳转指令
        match(token_then);
        ifnode->addChild(parse_statement());
        int addr2 = assembly.getSize(); //记录地址
        assembly.emit(JMP, 0, 0); //生成跳转指令，从else跳出
        assembly.backpatch(addr, assembly.getSize()); //回填地址，跳转到then后面
        if (expect(token_else))
        {
            ifnode->addChild(parse_statement());
        }
        assembly.backpatch(addr2, assembly.getSize()); //回填地址，跳转到else后面，如果没有else则跳过
        return ifnode;
    }

    // <lexp>-><exp><lop><exp>
    ASTNode* parse_lexp() //处理逻辑表达式
    {
        ASTNode* lexp;
        if (expect(token_odd))
        {
            lexp = new ASTNode(ast_odd, token.line, token.column);
            lexp->addChild(parse_exp());
            assembly.emit(OPR, 0, 6); //生成奇偶判断指令
        }
        else
        {
            lexp = new ASTNode(ast_eq, token.line, token.column);
            lexp->addChild(parse_exp());
            switch (token.type)
            {
            case token_eq:
                lexp->type = ast_eq;
                break;
            case token_neq:
                lexp->type = ast_neq;
                break;
            case token_lss:
                lexp->type = ast_lss;
                break;
            case token_leq:
                lexp->type = ast_leq;
                break;
            case token_gtr:
                lexp->type = ast_gtr;
                break;
            case token_geq:
                lexp->type = ast_geq;
                break;
            default:
                {
                    cout << "错误：第" << token.line << "行，第" << token.column << "列，非法逻辑表达式" << endl;
                    panic();
                }
            }
            GetToken();
            lexp->addChild(parse_exp());
            switch (lexp->type)
            {
            case ast_eq:
                assembly.emit(OPR, 0, 7); //生成相等判断指令
                break;
            case ast_neq:
                assembly.emit(OPR, 0, 8); //生成不等判断指令
                break;
            case ast_lss:
                assembly.emit(OPR, 0, 9); //生成小于判断指令
                break;
            case ast_leq:
                assembly.emit(OPR, 0, 12); //生成小于等于判断指令
                break;
            case ast_gtr:
                assembly.emit(OPR, 0, 11); //生成大于判断指令
                break;
            case ast_geq:
                assembly.emit(OPR, 0, 10); //生成大于等于判断指令
                break;
            default: break;
            }
        }
        return lexp;
    }

    // <exp>-><term>{<aop><term>}
    ASTNode* parse_exp() //表达式
    {
        ASTNode* exp;
        if (token.type == token_plus || token.type == token_minus)
        {
            exp = new ASTNode(token.type == token_plus ? ast_unary_plus : ast_unary_minus, token.line, token.column);
            GetToken();
        }
        else
        {
            exp = new ASTNode(ast_unary_plus, token.line, token.column);
        }
        ASTNode* cur = parse_term();
        while (token.type == token_plus || token.type == token_minus) //递归处理表达式
        {
            //创建父节点
            auto* parent = new ASTNode(token.type == token_plus ? ast_plus : ast_minus, token.line, token.column);
            parent->addChild(cur);
            GetToken();
            parent->addChild(parse_term());
            assembly.emit(OPR, 0, parent->type == ast_plus ? 2 : 3); //生成加法或减法指令
            cur = parent;
        }
        exp->addChild(cur);
        if (exp->type == ast_unary_minus) //如果是负数，生成负数指令
        {
            assembly.emit(OPR, 0, 1);
        }
        return exp;
    }

    // <term>-><factor>{<mop><factor>}
    ASTNode* parse_term() //项
    {
        ASTNode* cur = parse_factor();
        while (token.type == token_times || token.type == token_slash)
        {
            auto* parent = new ASTNode(token.type == token_times ? ast_times : ast_slash, token.line, token.column);
            parent->addChild(cur);
            GetToken();
            parent->addChild(parse_factor());
            assembly.emit(OPR, 0, parent->type == ast_times ? 4 : 5); //生成乘法或除法指令
            cur = parent;
        }
        return cur;
    }

    // <factor>-><id>|<integer>|(<exp>)
    ASTNode* parse_factor() //因子
    {
        if (token.type == token_id)
        {
            auto res = match(token_id);
            auto pos = table.lookup(res->value); //查找变量
            switch (pos.type)
            {
            case symbol_null:
                {
                    cout << "错误：第" << token.line << "行，第" << token.column << "列，未定义的变量" << res->value << endl;
                    panic();
                    return nullptr;
                }
            case symbol_procedure:
                {
                    cout << "错误：第" << token.line << "行，第" << token.column << "列，非法因子" << res->value << endl;
                    panic();
                    return nullptr;
                }
            case symbol_constant:
                {
                    assembly.emit(LIT, table.getLevel() - pos.level, pos.value); //生成常量指令
                    return res;
                }
            default://变量或参数
                {
                    assembly.emit(LOD, table.getLevel() - pos.level, pos.offset); //生成加载指令
                    return res;
                }
            }
        }
        if (token.type == token_integer)
        {
            auto res = match(token_integer);
            int num = stoi(res->value);
            assembly.emit(LIT, 0, num); //生成常量指令
            return res;
        }
        if (token.type == token_lparen)
        {
            GetToken();
            ASTNode* factor = parse_exp();
            match(token_rparen);
            return factor;
        }
        cout << "错误：第" << token.line << "行，第" << token.column << "列，非法因子" << endl;
        panic();
        return nullptr;
    }

    //<statement>->while<lexp>do<statement>
    ASTNode* parse_while() //处理while语句
    {
        auto* wh = new ASTNode(ast_while, token.line, token.column);
        match(token_while);
        int entry = assembly.getSize(); //记录开始地址
        wh->addChild(parse_lexp());
        int addr = assembly.getSize(); //记录地址
        assembly.emit(JPC, 0, 0); //生成条件跳转指令
        match(token_do);
        wh->addChild(parse_statement());
        assembly.emit(JMP, 0, entry); //生成跳转指令，跳转到条件判断
        assembly.backpatch(addr, assembly.getSize()); //回填地址，跳转到do后面
        return wh;
    }

    //<call>->call<id>(<argument>)
    ASTNode* parse_call() //处理过程调用
    {
        auto* call = new ASTNode(ast_call, token.line, token.column);
        match(token_call);
        call->addChild(match(token_id));
        auto pos = table.lookup(call->children[0]->value); //查找过程
        match(token_lparen);
            call->addChild(parse_argument());
        match(token_rparen);
        if (pos.type == symbol_null)
        {
            cout << "错误：第" << token.line << "行，第" << token.column << "列，未定义的过程" << call->children[0]->value << endl;
            panic();
            return nullptr;
        }
        else if (pos.type != symbol_procedure)
        {
            cout << "错误：第" << token.line << "行，第" << token.column << "列，非法过程" << call->children[0]->value << endl;
            panic();
            return nullptr;
        }
        if (call->children[1]->children.size() != pos.parameter)
        {
            cout << "错误：第" << token.line << "行，第" << token.column << "列，参数个数不匹配" << call->children[0]->value << endl;
            panic();
            return nullptr;
        }
        for (int i = 0; i < call->children[1]->children.size(); i++)
        {
            assembly.emit(STO, -1, 3 + i); //生成加载参数指令
        }
        assembly.emit(CAL, table.getLevel() - pos.level, pos.offset); //生成调用指令
        return call;
    }

    //<argument>-><exp>{,<exp>}
    ASTNode* parse_argument() //处理过程参数
    {
        auto* arg = new ASTNode(ast_argument);
        if (token.type == token_rparen)//无参数情况
            return arg;
        do
        {
            arg->addChild(parse_exp());
        }
        while (expect(token_comma));
        return arg;
    }

    //<statement>->read(<id>{,<id>})
    ASTNode* parse_read() //处理读语句
    {
        auto* read = new ASTNode(ast_read, token.line, token.column);
        match(token_read);
        match(token_lparen);
        do
        {
            read->addChild(match(token_id));
            auto pos = table.lookup(read->children.back()->value); //查找变量
            if (pos.type == symbol_null)
            {
                cout << "错误：第" << token.line << "行，第" << token.column << "列，未定义的变量" << read->children.back()->value <<
                    endl;
                panic();
                return nullptr;
            }
            else if (pos.type == symbol_procedure)
            {
                cout << "错误：第" << token.line << "行，第" << token.column << "列，非法读取" << read->children.back()->value <<
                    endl;
                panic();
                return nullptr;
            }
            assembly.emit(RED, 0, 0); //生成读指令
            assembly.emit(STO, table.getLevel() - pos.level, pos.offset); //写入变量
        }
        while (expect(token_comma));
        match(token_rparen);
        return read;
    }

    //<statement>->write(<exp>{,<exp>})
    ASTNode* parse_write() //处理写语句
    {
        auto* write = new ASTNode(ast_write, token.line, token.column);
        match(token_write);
        match(token_lparen);
        do
        {
            write->addChild(parse_exp());
            assembly.emit(WRT, 0, 0); //生成写指令
            assembly.emit(OPR, 0, 13); //生成换行指令
        }
        while (expect(token_comma));
        match(token_rparen);
        return write;
    }

public:
    explicit Parser(ifstream& fin): fin(fin), root(nullptr)
    {
        if (!fin)
        {
            cout << "文件打开失败！" << endl;
            return;
        }
    }

    ASTNode* start() //开始解析
    {
        parse_program();
        return root;
    }

    void treePrint() const //打印语法树
    {
        root->print(0);
    }

    void codePrint() //打印汇编代码
    {
        assembly.print();
    }
    void saveCode(ofstream &fout) //保存汇编代码
    {
        if (!fout)
        {
            cout << "文件打开失败！" << endl;
            return;
        }
        for (auto &code : assembly.codes)
        {
            fout << code.F << ' ' << code.L << ' ' << code.A << endl;
        }
    }
};


#endif //PARSER_H
