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
        cout << "�ļ���ʧ�ܣ�" << endl;
        return;
    }
    printFile(fin);
    Lexer lexer(fin);
    cout << "�ʷ����������" << endl;
    cout <<  setw(10) << "����" << setw(5) << "�к�" << setw(5) << "�к�" << setw(15) << "ֵ" <<  endl;
    ofstream fout("lex.txt");
    lexer.getAll(fout);
    fin.close();
    fout.close();
    fin.open("lex.txt");
    Parser parser(fin);
    parser.start();
    cout << "�﷨����" << endl;
    parser.treePrint();
    fin.close();
    cout << "�﷨��������" << endl;
    cout<<"�����룺"<<endl;
    parser.codePrint();
    fout.open("code.txt");
    parser.saveCode(fout);
    fout.close();
    fin.close();
    cout << "���������ɽ���" << endl;
    ifstream file;
    file.open("code.txt");
    // printFile(file);
    Interpreter interpreter(file);
    // file.close();
    interpreter.run();
    cout << "����ִ�н���" << endl;
}

int main()
{
    system("chcp 936>nul");
    while (1)
    {
        cout << "�����ļ�����";
        cin >> filename;
            test(filename);
    }
}
