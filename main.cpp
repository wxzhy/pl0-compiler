#include <fstream>
#include <iostream>
#include <string>

#include "Interpreter.h"
#include "Lexer.h"
#include "Parser.h"
using namespace std;
string filename = "test.pl0";
ifstream fin;

void printFile(ifstream& fin)
{
    char ch;
    while (fin.get(ch))
    {
        cout << ch;
    }
    cout << endl;
    fin.clear();
    fin.seekg(0, ios::beg);
}

void test(string& filename)
{
    fin.open(filename);
    if (!fin)
    {
        cout << "文件打开失败！" << endl;
        return;
    }
    printFile(fin);
    Lexer lexer(fin);
    cout << "词法分析结果：" << endl;
    cout <<  setw(10) << "类型" << setw(5) << "行号" << setw(5) << "列号" << setw(15) << "值" <<  endl;
    ofstream fout("lex.txt");
    lexer.getAll(fout);
    fin.close();
    fout.close();
    fin.open("lex.txt");
    Parser parser(fin);
    parser.start();
    cout << "语法树：" << endl;
    parser.treePrint();
    fin.close();
    cout << "语法分析结束" << endl;
    cout<<"汇编代码："<<endl;
    parser.codePrint();
    fout.open("code.txt");
    parser.saveCode(fout);
    fout.close();
    fin.close();
    cout << "汇编代码生成结束" << endl;
    ifstream file;
    file.open("code.txt");
    // printFile(file);
    Interpreter interpreter(file);
    // file.close();
    interpreter.run();
    cout << "解释执行结束" << endl;
}

int main()
{
    system("chcp 936>nul");
    while (1)
    {
        cout << "输入文件名：";
        cin >> filename;
            test(filename);
    }
}
