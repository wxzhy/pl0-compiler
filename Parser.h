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

class Parser //PL0�﷨�������﷨������
{
    ifstream& fin;
    Token token;
    ASTNode* root; //�﷨�����ڵ�
    SymbolTable table; //���ű�
    Assembly assembly; //������
    void GetToken()
    {
        //�ļ��ж�ȡtoken
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

    void panic() //�������󲿷֣�ֱ��ͬ����
    {
        //���ܵ�ͬ����:
        //; , end )
        while (token.type != token_eof)
        {
            GetToken();
            if (token.type == token_semicolon || token.type == token_comma || token.type == token_end || token.type ==
                token_rparen)
            {
                GetToken(); //����ͬ���㱾��
                return;
            }
        }
    }

    ASTNode* match(token_type type) //ƥ��,������򷵻ؽڵ�,���򱨴�
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
            cout << "���󣺵�" << token.line << "�У���" << token.column << "�У�ӦΪ" << friendlyNames[type] << endl;
            panic();
            return nullptr;
        }
    }

    bool expect(token_type type) //����,�����������
    {
        if (token.type == type)
        {
            GetToken();
            return true;
        }
        return false;
    }

    // <program>->program<id>;<block>
    void parse_program() //����������
    {
        GetToken();
        match(token_program);
        root = new ASTNode(ast_program, token.line, token.column);
        root->addChild(match(token_id));
        table.addScope(root->children[0]->value); //���������
        match(token_semicolon);
        root->addChild(parse_block());
    }

    // <block>->[<condecl>][<vardecl>][<procedure>]<body>
    ASTNode* parse_block() //����
    {
        auto* block = new ASTNode(ast_block, token.line, token.column);
        int entry = assembly.getSize(); //��¼��ڵ�ַ
        assembly.emit(JMP, 0, 0); //������תָ���ת��ڵ�ַ
        if (token.type == token_const)
        {
            block->addChild(parse_condecl()); //��������
        }
        if (token.type == token_var)
        {
            block->addChild(parse_vardecl()); //��������
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
        assembly.backpatch(entry, assembly.getSize()); //������ڵ�ַ
        table.setEntry(assembly.getSize()); //���ù��̵���ڵ�ַ
        assembly.emit(INT, 0, table.getSize()); //���ɷ���ռ�ָ��
        block->addChild(parse_body());
        table.delScope(); //ɾ��������
        assembly.emit(OPR, 0, 0); //���ɷ���ָ��
        return block;
    }

    // <condecl>->const<const>{,<const>};
    ASTNode* parse_condecl() //����������
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
    ASTNode* parse_const() //����������
    {
        auto* con = new ASTNode(ast_const, token.line, token.column);
        con->addChild(match(token_id));
        match(token_assign);
        con->addChild(match(token_integer));
        if (con->children.size() == 2) //��ӷ��ŵ����ű�
        {
            table.addSymbol(symbol_constant, con->children[0]->value, stoi(con->children[1]->value)); //��ӳ��������ű�
        }
        return con;
    }

    // <vardecl>->var<id>{,<id>};
    ASTNode* parse_vardecl() //�����������
    {
        auto* vardecl = new ASTNode(ast_vardecl, token.line, token.column);
        match(token_var);
        do
        {
            vardecl->addChild(match(token_id));
            table.addSymbol(symbol_variable, vardecl->children.back()->value, 0); //��ӱ��������ű�
        }
        while (expect(token_comma));
        match(token_semicolon);
        return vardecl;
    }

    // <procedure>->procedure<id>[<parameter>];<block>
    void parse_procedure(ASTNode* parent) //��ӹ��̽ڵ㣬����Ϊ���ڵ�
    {
        auto* procedure = new ASTNode(ast_procedure, token.line, token.column);
        match(token_procedure);
        procedure->addChild(match(token_id));
        if (!procedure->children.size()) 
        {
            panic();
            return;
        }
         if (table.addSymbol(symbol_procedure, procedure->children[0]->value, assembly.getSize())) //��ӹ��̵����ű�
        {
            table.addScope(procedure->children[0]->value); //���������
        }
        else
        {
            panic();
            return;
        }
        match(token_lparen);
        auto* params = new ASTNode(ast_parameter, token.line, token.column);
        //����������
        if (token.type == token_id)
        {
            procedure->addChild(params);
            do
            {
                params->addChild(match(token_id));
                table.addSymbol(symbol_parameter, params->children.back()->value, 0); //��Ӳ��������ű�
            }
            while (expect(token_comma));
        }
        match(token_rparen);
        match(token_semicolon);
        procedure->addChild(parse_block());
        parent->addChild(procedure);

    }

    // <body>->begin<statement>{;<statement>}end
    ASTNode* parse_body() //����body���
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

    ASTNode* parse_statement() //����statement���
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
                cout << "���󣺵�" << token.line << "�У���" << token.column << "�У��Ƿ����" << endl;
                panic();
                return nullptr;
            }
        }
    }

    // <statement>-><id>:=<exp>
    ASTNode* parse_assign() //����ֵ���
    {
        auto* con = new ASTNode(ast_assign, token.line, token.column);
        con->addChild(match(token_id));
        match(token_assign);
        con->addChild(parse_exp());
        if (con->children.size() == 2) //��ӷ��ŵ����ű�
        {
            auto pos = table.lookup(con->children[0]->value); //���ұ���
            if (pos.type == symbol_null)
            {
                cout << "���󣺵�" << token.line << "�У���" << token.column << "�У�δ����ı���" << con->children[0]->value << endl;
                panic();
                return nullptr;
            }
            else if (pos.type ==symbol_procedure)
            {
                cout << "���󣺵�" << token.line << "�У���" << token.column << "�У��Ƿ���ֵ" << con->children[0]->value << endl;
                panic();
                return nullptr;
            }
            assembly.emit(STO, table.getLevel() - pos.level, pos.offset); //���ɴ洢ָ��
        }
        return con;
    }

    // <statement>->if<lexp>then<statement>[else<statement>]
    ASTNode* parse_if() //����if���
    {
        auto* ifnode = new ASTNode(ast_if, token.line, token.column);
        match(token_if);
        ifnode->addChild(parse_lexp());
        int addr = assembly.getSize(); //��¼��ַ
        assembly.emit(JPC, 0, 0); //����������תָ��
        match(token_then);
        ifnode->addChild(parse_statement());
        int addr2 = assembly.getSize(); //��¼��ַ
        assembly.emit(JMP, 0, 0); //������תָ���else����
        assembly.backpatch(addr, assembly.getSize()); //�����ַ����ת��then����
        if (expect(token_else))
        {
            ifnode->addChild(parse_statement());
        }
        assembly.backpatch(addr2, assembly.getSize()); //�����ַ����ת��else���棬���û��else������
        return ifnode;
    }

    // <lexp>-><exp><lop><exp>
    ASTNode* parse_lexp() //�����߼����ʽ
    {
        ASTNode* lexp;
        if (expect(token_odd))
        {
            lexp = new ASTNode(ast_odd, token.line, token.column);
            lexp->addChild(parse_exp());
            assembly.emit(OPR, 0, 6); //������ż�ж�ָ��
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
                    cout << "���󣺵�" << token.line << "�У���" << token.column << "�У��Ƿ��߼����ʽ" << endl;
                    panic();
                }
            }
            GetToken();
            lexp->addChild(parse_exp());
            switch (lexp->type)
            {
            case ast_eq:
                assembly.emit(OPR, 0, 7); //��������ж�ָ��
                break;
            case ast_neq:
                assembly.emit(OPR, 0, 8); //���ɲ����ж�ָ��
                break;
            case ast_lss:
                assembly.emit(OPR, 0, 9); //����С���ж�ָ��
                break;
            case ast_leq:
                assembly.emit(OPR, 0, 12); //����С�ڵ����ж�ָ��
                break;
            case ast_gtr:
                assembly.emit(OPR, 0, 11); //���ɴ����ж�ָ��
                break;
            case ast_geq:
                assembly.emit(OPR, 0, 10); //���ɴ��ڵ����ж�ָ��
                break;
            default: break;
            }
        }
        return lexp;
    }

    // <exp>-><term>{<aop><term>}
    ASTNode* parse_exp() //���ʽ
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
        while (token.type == token_plus || token.type == token_minus) //�ݹ鴦����ʽ
        {
            //�������ڵ�
            auto* parent = new ASTNode(token.type == token_plus ? ast_plus : ast_minus, token.line, token.column);
            parent->addChild(cur);
            GetToken();
            parent->addChild(parse_term());
            assembly.emit(OPR, 0, parent->type == ast_plus ? 2 : 3); //���ɼӷ������ָ��
            cur = parent;
        }
        exp->addChild(cur);
        if (exp->type == ast_unary_minus) //����Ǹ��������ɸ���ָ��
        {
            assembly.emit(OPR, 0, 1);
        }
        return exp;
    }

    // <term>-><factor>{<mop><factor>}
    ASTNode* parse_term() //��
    {
        ASTNode* cur = parse_factor();
        while (token.type == token_times || token.type == token_slash)
        {
            auto* parent = new ASTNode(token.type == token_times ? ast_times : ast_slash, token.line, token.column);
            parent->addChild(cur);
            GetToken();
            parent->addChild(parse_factor());
            assembly.emit(OPR, 0, parent->type == ast_times ? 4 : 5); //���ɳ˷������ָ��
            cur = parent;
        }
        return cur;
    }

    // <factor>-><id>|<integer>|(<exp>)
    ASTNode* parse_factor() //����
    {
        if (token.type == token_id)
        {
            auto res = match(token_id);
            auto pos = table.lookup(res->value); //���ұ���
            switch (pos.type)
            {
            case symbol_null:
                {
                    cout << "���󣺵�" << token.line << "�У���" << token.column << "�У�δ����ı���" << res->value << endl;
                    panic();
                    return nullptr;
                }
            case symbol_procedure:
                {
                    cout << "���󣺵�" << token.line << "�У���" << token.column << "�У��Ƿ�����" << res->value << endl;
                    panic();
                    return nullptr;
                }
            case symbol_constant:
                {
                    assembly.emit(LIT, table.getLevel() - pos.level, pos.value); //���ɳ���ָ��
                    return res;
                }
            default://���������
                {
                    assembly.emit(LOD, table.getLevel() - pos.level, pos.offset); //���ɼ���ָ��
                    return res;
                }
            }
        }
        if (token.type == token_integer)
        {
            auto res = match(token_integer);
            int num = stoi(res->value);
            assembly.emit(LIT, 0, num); //���ɳ���ָ��
            return res;
        }
        if (token.type == token_lparen)
        {
            GetToken();
            ASTNode* factor = parse_exp();
            match(token_rparen);
            return factor;
        }
        cout << "���󣺵�" << token.line << "�У���" << token.column << "�У��Ƿ�����" << endl;
        panic();
        return nullptr;
    }

    //<statement>->while<lexp>do<statement>
    ASTNode* parse_while() //����while���
    {
        auto* wh = new ASTNode(ast_while, token.line, token.column);
        match(token_while);
        int entry = assembly.getSize(); //��¼��ʼ��ַ
        wh->addChild(parse_lexp());
        int addr = assembly.getSize(); //��¼��ַ
        assembly.emit(JPC, 0, 0); //����������תָ��
        match(token_do);
        wh->addChild(parse_statement());
        assembly.emit(JMP, 0, entry); //������תָ���ת�������ж�
        assembly.backpatch(addr, assembly.getSize()); //�����ַ����ת��do����
        return wh;
    }

    //<call>->call<id>(<argument>)
    ASTNode* parse_call() //������̵���
    {
        auto* call = new ASTNode(ast_call, token.line, token.column);
        match(token_call);
        call->addChild(match(token_id));
        auto pos = table.lookup(call->children[0]->value); //���ҹ���
        match(token_lparen);
            call->addChild(parse_argument());
        match(token_rparen);
        if (pos.type == symbol_null)
        {
            cout << "���󣺵�" << token.line << "�У���" << token.column << "�У�δ����Ĺ���" << call->children[0]->value << endl;
            panic();
            return nullptr;
        }
        else if (pos.type != symbol_procedure)
        {
            cout << "���󣺵�" << token.line << "�У���" << token.column << "�У��Ƿ�����" << call->children[0]->value << endl;
            panic();
            return nullptr;
        }
        if (call->children[1]->children.size() != pos.parameter)
        {
            cout << "���󣺵�" << token.line << "�У���" << token.column << "�У�����������ƥ��" << call->children[0]->value << endl;
            panic();
            return nullptr;
        }
        for (int i = 0; i < call->children[1]->children.size(); i++)
        {
            assembly.emit(STO, -1, 3 + i); //���ɼ��ز���ָ��
        }
        assembly.emit(CAL, table.getLevel() - pos.level, pos.offset); //���ɵ���ָ��
        return call;
    }

    //<argument>-><exp>{,<exp>}
    ASTNode* parse_argument() //������̲���
    {
        auto* arg = new ASTNode(ast_argument);
        if (token.type == token_rparen)//�޲������
            return arg;
        do
        {
            arg->addChild(parse_exp());
        }
        while (expect(token_comma));
        return arg;
    }

    //<statement>->read(<id>{,<id>})
    ASTNode* parse_read() //��������
    {
        auto* read = new ASTNode(ast_read, token.line, token.column);
        match(token_read);
        match(token_lparen);
        do
        {
            read->addChild(match(token_id));
            auto pos = table.lookup(read->children.back()->value); //���ұ���
            if (pos.type == symbol_null)
            {
                cout << "���󣺵�" << token.line << "�У���" << token.column << "�У�δ����ı���" << read->children.back()->value <<
                    endl;
                panic();
                return nullptr;
            }
            else if (pos.type == symbol_procedure)
            {
                cout << "���󣺵�" << token.line << "�У���" << token.column << "�У��Ƿ���ȡ" << read->children.back()->value <<
                    endl;
                panic();
                return nullptr;
            }
            assembly.emit(RED, 0, 0); //���ɶ�ָ��
            assembly.emit(STO, table.getLevel() - pos.level, pos.offset); //д�����
        }
        while (expect(token_comma));
        match(token_rparen);
        return read;
    }

    //<statement>->write(<exp>{,<exp>})
    ASTNode* parse_write() //����д���
    {
        auto* write = new ASTNode(ast_write, token.line, token.column);
        match(token_write);
        match(token_lparen);
        do
        {
            write->addChild(parse_exp());
            assembly.emit(WRT, 0, 0); //����дָ��
            assembly.emit(OPR, 0, 13); //���ɻ���ָ��
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
            cout << "�ļ���ʧ�ܣ�" << endl;
            return;
        }
    }

    ASTNode* start() //��ʼ����
    {
        parse_program();
        return root;
    }

    void treePrint() const //��ӡ�﷨��
    {
        root->print(0);
    }

    void codePrint() //��ӡ������
    {
        assembly.print();
    }
    void saveCode(ofstream &fout) //���������
    {
        if (!fout)
        {
            cout << "�ļ���ʧ�ܣ�" << endl;
            return;
        }
        for (auto &code : assembly.codes)
        {
            fout << code.F << ' ' << code.L << ' ' << code.A << endl;
        }
    }
};


#endif //PARSER_H
