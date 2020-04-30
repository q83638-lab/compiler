#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <utility>
#include <sstream>
#include <algorithm>

using namespace std;

// parsetree structure
struct Tree {
    string rule;
    vector<string> tokens;
    vector<Tree> subtree;
};

// top symbol table
map<string, pair<vector<string>, map<string, pair<string, int> > > > tST;
// tracking current procedure
string proc;
// tracking procedure for signatures
string sproc;
// tracking signature count
int scount;
// store variable offsets
int offset;
// counter for loop label
int cloop = 0;
// counter for conditional statement labels
int ccond = 0;
// counter for delete label
int cdel = 0;

// check if a token is a non-terminal
bool isnonT(string token)
{
    vector<string> terminals = {
        "BOF", "BECOMES", "COMMA", "ELSE", "EOF", "EQ", "GE",
        "GT", "ID", "IF", "INT", "LBRACE", "LE", "LPAREN",
        "LT", "MINUS", "NE", "NUM", "PCT", "PLUS", "PRINTLN",
        "RBRACE", "RETURN", "RPAREN", "SEMI", "SLASH", "STAR", "WAIN",
        "WHILE", "AMP", "LBRACK", "RBRACK", "NEW", "DELETE", "NULL"
    };
    return count(terminals.begin(), terminals.end(), token) == 0;
}

// create parsetree from input
Tree createTree(istream& in)
{
    Tree newtree;
    string s;
    vector<string> temp;
    getline(in, s);
    newtree.rule = s;
    stringstream check1(s);
    string intermediate;

    while (getline(check1, intermediate, ' ')) {
        temp.emplace_back(intermediate);
    }
    newtree.tokens = temp;
    if (isnonT(temp[0])) {
        for (int i = 1; i < newtree.tokens.size(); i++) {
            newtree.subtree.emplace_back(createTree(in));
        }
    }
    return newtree;
}

void push(int reg)
{
    cout << "sw $" << reg << ", -4($30)" << endl;
    cout << "sub $30, $30, $4" << endl;
}

void pop(int reg)
{
    cout << "add $30 , $30 , $4" << endl;
    cout << "lw $" << reg << ", -4($30)" << endl;
}

void call(string func)
{
    push(31);
    cout << "lis $5" << endl;
    cout << ".word " + func << endl;
    cout << "jalr $5" << endl;
    pop(31);
}

int offsets(string var)
{
    return tST[proc].second[var].second;
}

void code(string var) { cout << "lw $3," << offsets(var) << "($29)" << endl; }

void swreg(string var)
{
    cout << "sw $3," << offsets(var) << "($29)" << endl;
    cout << "sub $30, $30, $4" << endl;
}

//prologue and epilogue for wain
void prologue()
{
    cout << ".import init" << endl;
    cout << ".import new" << endl;
    cout << ".import delete" << endl;
    cout << ".import print" << endl;
    cout << "lis $4" << endl;
    cout << ".word 4" << endl;
    cout << "lis $11" << endl;
    cout << ".word 1" << endl;
    cout << " ;$29 pointing at the bottom of the stack " << endl;
    cout << "sub $29, $30, $4" << endl;
}

void epilogue()
{
    cout << ";restore and return" << endl;
    cout << "add $30 , $29 , $4" << endl;
    cout << "jr $31" << endl;
}

//update offset to account for positive offsets for non-wain function arguments
void updateoffset()
{
    for (auto& t : tST) {
        if (t.first != "wain") {
            int tot = t.second.first.size();
            for (auto& x : t.second.second) {
                x.second.second += tot * 4;
            }
        }
    }
}

// traverse the tree, build symbol table, removed checks for declaration errors.
void traverse(const Tree& t)
{
    if (t.rule == "main INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN "
                  "expr SEMI RBRACE") {
        proc = "wain";
        offset = 0;
        if (tST.find(proc) == tST.end()) {
            tST.emplace(make_pair(
                proc, make_pair(vector<string>(), map<string, pair<string, int> >())));
            Tree dcl1 = t.subtree[3].subtree[0];
            Tree dcl2 = t.subtree[5].subtree[0];
            if (dcl1.rule == "type INT STAR") {
                tST.at(proc).first.emplace_back(dcl1.subtree[0].tokens[1] + dcl1.subtree[1].tokens[1]);
            }
            else if (dcl1.rule == "type INT") {
                tST.at(proc).first.emplace_back(dcl1.subtree[0].tokens[1]);
            }
            push(1);
            if (dcl2.rule == "type INT STAR") {
                tST.at(proc).first.emplace_back(dcl2.subtree[0].tokens[1] + dcl2.subtree[1].tokens[1]);
            }
            else if (dcl2.rule == "type INT") {
                tST.at(proc).first.emplace_back(dcl2.subtree[0].tokens[1]);
            }
            push(2);
        }
    }
    else if (t.rule == "procedure INT ID LPAREN params RPAREN LBRACE dcls statements "
                       "RETURN expr SEMI RBRACE") {
        offset = 0;
        proc = t.subtree[1].tokens[1];
        if (tST.find(proc) == tST.end()) {
            tST.emplace(make_pair(
                proc, make_pair(vector<string>(), map<string, pair<string, int> >())));
        }
    }
    else if (t.rule == "paramlist dcl COMMA paramlist" || t.rule == "paramlist dcl") {
        Tree dcl1 = t.subtree[0].subtree[0];
        if (dcl1.rule == "type INT STAR") {
            tST.at(proc).first.emplace_back(dcl1.subtree[0].tokens[1] + dcl1.subtree[1].tokens[1]);
        }
        else if (dcl1.rule == "type INT") {
            tST.at(proc).first.emplace_back(dcl1.subtree[0].tokens[1]);
        }
    }
    else if (t.rule == "dcl type ID") {
        Tree type = t.subtree[1];
        Tree id = t.subtree[0].subtree[0];

        if (t.subtree.at(0).rule == "type INT") {
            tST[proc].second.emplace(make_pair(type.tokens[1], make_pair(id.tokens[1], offset)));
        }
        else if (t.subtree.at(0).rule == "type INT STAR") {
            Tree id2 = t.subtree[0].subtree[1];
            tST[proc].second.emplace(make_pair(type.tokens[1], make_pair(id.tokens[1] + id2.tokens[1], offset)));
        }
        offset -= 4; //calculating offsets for symbol table
    }
    
    for (const auto& i : t.subtree) {
        traverse(i);
    }
}

//typechecker with error checking removed, assume valid parse tree as input
string checkType(const Tree& t)
{
    if (!t.tokens.empty() && t.tokens.at(0) == "ID") {
        return tST[proc].second[t.tokens.at(1)].first;
    }
    else if (!t.tokens.empty() && t.tokens.at(0) == "NUM") {
        return "int";
    }
    else if (!t.tokens.empty() && t.tokens.at(0) == "NULL") {
        return "int*";
    }
    else if (t.rule == "dcls dcls dcl BECOMES NULL SEMI") {
        string type = checkType(t.subtree[1].subtree[1]);
        return checkType(t.subtree[1]);
    }
    else if (t.rule == "dcls dcls dcl BECOMES NUM SEMI") {
        return checkType(t.subtree[1]);
    }
    else if (t.rule == "dcl type ID") {
        return checkType(t.subtree[1]);
    }
    else if (t.rule == "factor NUM" || t.rule == "factor NULL" || t.rule == "factor ID" || t.rule == "expr term" || t.rule == "term factor" || t.rule == "lvalue ID") {
        string type = checkType(t.subtree[0]);
        return type;
    }
    else if (t.rule == "factor ID LPAREN RPAREN")
        return "int";
    else if (t.rule == "lvalue LPAREN lvalue RPAREN" || t.rule == "factor LPAREN expr RPAREN") {
        string type = checkType(t.subtree[1]);
        return type;
    }
    else if (t.rule == "lvalue STAR factor" || t.rule == "factor STAR factor") {
        string type = checkType(t.subtree[1]);
        if (type == "int*") {
            return "int";
        }
    }
    else if (t.rule == "expr expr PLUS term") {
        if (checkType(t.subtree[0]) == "int" && checkType(t.subtree[2]) == "int") {
            return "int";
        }
        else {
            return "int*";
        }
    }
    else if (t.rule == "expr expr MINUS term") {
        string type1 = checkType(t.subtree[0]);
        string type2 = checkType(t.subtree[2]);
        if (type1 == "int" && type2 == "int" || type1 == "int*" && type2 == "int*") {
            return "int";
        }
        else if (type1 == "int*" && type2 == "int") {
            return "int*";
        }
    }
    else if (t.rule == "term term STAR factor" || t.rule == "term term SLASH factor" || t.rule == "term term PCT factor") {
        string type1 = checkType(t.subtree[0]);
        string type2 = checkType(t.subtree[2]);
        if (type1 == "int" && type2 == "int") {
            return "int";
        }
    }
    else if (t.rule == "statement lvalue BECOMES expr SEMI") {
        string type = checkType(t.subtree[0]);
        if (type == checkType(t.subtree[2])) {
            return type;
        }
    }
    else if (t.rule == "arglist expr") {
        return checkType(t.subtree[0]);
    }
    else if (t.rule == "arglist expr COMMA arglist") {
        return checkType(t.subtree[2]);
    }
    else if (t.rule == "factor ID LPAREN arglist RPAREN") {
        return checkType(t.subtree[2]);
    }
    else if (t.rule == "test expr EQ expr" || t.rule == "test expr NE expr") {
        string type1 = checkType(t.subtree[0]);
        string type2 = checkType(t.subtree[2]);
        return type1;
    }
    else if (t.rule == "test expr LT expr" || t.rule == "test expr GT expr" || t.rule == "test expr LE expr" || t.rule == "test expr GE expr") {
        return "";
    }
    else if (t.rule == "factor NEW INT LBRACK expr RBRACK") {
        return "int*";
    }
    else if (t.rule == "factor AMP lvalue") {
        return "int*";
    }
    else if (t.rule == "statement PRINTLN LPAREN expr RPAREN SEMI") {
        string type = checkType(t.subtree[2]);
        return type;
    }
    else if (t.rule == "statement DELETE LBRACK RBRACK expr SEMI") {
        return checkType(t.subtree[3]);
    }
    else {
        return "";
    }
}

// print symbol table
void print()
{
    for (auto const& t : tST) {
        cerr << t.first << " ";
        for (auto const& i : t.second.first) {
            cerr << i + " ";
        }
        cerr << endl;
        for (auto const& x : t.second.second) {
            cerr << x.first << " " << x.second.first << " " << x.second.second
                 << endl;
        }
        cerr << endl;
    }
}

//code generation begin
void code_gen(const Tree& t)
{
    string type = "";
    if (t.rule == "procedures main") {
        code_gen(t.subtree.at(0));
        epilogue();
    }
    else if (t.rule == "main INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements "
                       "RETURN expr SEMI RBRACE") {
        proc = "wain";
        code_gen(t.subtree.at(8)); // dcls
        code_gen(t.subtree.at(9)); // statements
        code_gen(t.subtree.at(11)); // expr
    }
    else if (t.rule == "procedures procedure procedures") {
        code_gen(t.subtree.at(1));
        code_gen(t.subtree.at(0));
    }
    else if (t.rule == "procedure INT ID LPAREN params RPAREN LBRACE dcls statements "
                       "RETURN expr SEMI RBRACE") {
        proc = t.subtree[1].tokens[1];
        string flabel = "F" + proc;
        cout << flabel << ":" << endl;
        cout << "sub $29, $30, $4" << endl;
        code_gen(t.subtree[6]); // dcls
        // push(1);
        // push(2);
        // push(5);
        push(29);
        push(31);
        //  push(7);
        //  push(8);
        code_gen(t.subtree[7]); // statements
        code_gen(t.subtree[9]); // expr
        pop(31);
        pop(29);
        // pop(5);
        // pop(2);
        // pop(1);
        cout << "add $30, $29, $4" << endl;
        cout << "jr $31" << endl;
    }
    else if (t.rule == "factor ID LPAREN RPAREN") {
        string flabel = "F" + t.subtree[0].tokens[1];
        push(29);
        call(flabel);
        pop(29);
    }
    else if (t.rule == "factor ID LPAREN arglist RPAREN") {
        string flabel = "F" + t.subtree[0].tokens[1];
        push(29);
        push(31);
        code_gen(t.subtree[2]);
        cout << "lis $5" << endl;
        cout << ".word " << flabel << endl;
        cout << "jalr $5" << endl;
        int popnum = tST[t.subtree[0].tokens[1]].first.size() * 4;
        cerr << "popnum =" << popnum << endl;
        cout << "lis $15" << endl;
        cout << ".word " << popnum << endl;
        cout << "add $30, $30, $15" << endl;
        pop(31);
        pop(29);
    }
    else if (t.rule == "dcls dcls dcl BECOMES NUM SEMI") {
        code_gen(t.subtree[0]);
        string varname = t.subtree[1].subtree[1].tokens[1];
        cout << "lis $3" << endl;
        cout << ".word " << t.subtree.at(3).tokens[1] << endl;
        swreg(varname);
    }
    else if (t.rule == "statement lvalue BECOMES expr SEMI") {
        string type = checkType(t.subtree[0]);
        cerr << " type: " << type << endl;
        string varname = t.subtree[0].tokens[1];
        if (type == "int") {
        code_gen(t.subtree[0]);
        code_gen(t.subtree[2]);
        cout << "sw $3," << offsets(varname) << "($29)" << endl;
        }
        else if (type == "int*")  {
        code_gen(t.subtree[2]);
        push(3);
        cout << "add $5, $0, $3" << endl;
        code_gen(t.subtree[0]);
        pop(5);
        cout << "sw $5, 0($3)" << endl;
        }
    }
    else if (t.rule == "lvalue ID") {
        if (type == "AMP") {
            cout << "lis $3" << endl;
            string varname = t.subtree[0].tokens[1];
            cout << ".word " << offsets(varname) << endl;
            cout << "add $3, $3, $29" << endl;
        }
    }
    else if (t.rule == "lvalue LPAREN lvalue RPAREN") {
        code_gen(t.subtree[1]);
    }
    else if (t.rule == "arglist expr") {
        code_gen(t.subtree[0]);
        push(3);
    }
    else if (t.rule == "arglist expr COMMA arglist") {
        code_gen(t.subtree[0]);
        push(3);
        code_gen(t.subtree[2]);
    }
    else if (t.rule == "statements statements statement") {
        code_gen(t.subtree.at(0));
        code_gen(t.subtree.at(1));
    }
    else if (t.rule == "statement PRINTLN LPAREN expr RPAREN SEMI") {
        push(1);
        code_gen(t.subtree.at(2));
        cout << "add $1, $3, $0" << endl;
        call("print");
        pop(1);
    }
    else if (t.rule == "expr term" || t.rule == "term factor") {
        code_gen(t.subtree.at(0));
    }
    else if (t.rule == "factor LPAREN expr RPAREN") {
        code_gen(t.subtree.at(1));
    }
    else if (t.rule == "factor NUM") {
        string val = t.subtree[0].tokens[1];
        cout << "lis $3" << endl;
        cout << ".word " + val << endl;
    }
    else if (t.rule == "factor ID") {
        string val = t.subtree[0].tokens[1];
        code(val);
    }
    else if (t.rule.find("term term") != string::npos || t.rule.find("expr expr") != string::npos) {
        code_gen(t.subtree.at(0));
        push(3);
        code_gen(t.subtree.at(2));
        pop(5);
        string left = checkType(t.subtree[0]);
        string right = checkType(t.subtree[2]);
        if (t.rule == "expr expr PLUS term") {
            if (left == "int*" && right == "int") {
                cout << ";int* + int" << endl;
                // cerr << "got here " <<endl;
                cout << "mult $3, $4" << endl;
                cout << "mflo $3" << endl;
            }
            else if (left == "int" && right == "int*") {
                cout << ";int + int*" << endl;
                cout << "mult $5, $4" << endl;
                cout << "mflo $5" << endl;
            }
            cout << "add $3, $5, $3" << endl;
        }
        else if (t.rule == "expr expr MINUS term") {
            if (left == "int*" && right == "int") {
                cout << ";int* - int" << endl;
                cout << "mult $3, $4" << endl;
                cout << "mflo $3" << endl;
                cout << "sub $3, $5, $3" << endl;
            }
            else if (left == "int*" && right == "int*") {
                cout << ";int* - int*" << endl;
                cout << "sub $3, $5, $3" << endl;
                cout << "divu $3, $4" << endl;
                cout << "mflo $3" << endl;
            }
            else {
                cout << "sub $3, $5, $3" << endl;
            }
        }
        else if (t.rule == "term term STAR factor") {
            cout << "mult $5, $3" << endl;
            cout << "mflo $3" << endl;
        }
        else if (t.rule == "term term SLASH factor") {
            cout << "div $5, $3" << endl;
            cout << "mflo $3" << endl;
        }
        else if (t.rule == "term term PCT factor") {
            cout << "div $5, $3" << endl;
            cout << "mfhi $3" << endl;
        }
    }
    else if (t.rule.find("test expr") != string::npos) {
        code_gen(t.subtree[0]);
        push(3);
        code_gen(t.subtree[2]);
        pop(5);
        // cerr << "type "<< checkType(t.subtree[0]) <<endl;
        string slt = checkType(t.subtree[0]) == "int" ? "slt" : "sltu";
        // cerr << "slt "<< slt <<endl;
        if (t.rule == "test expr LT expr") {
            cout << slt + "$3, $5, $3" << endl;
        }
        else if (t.rule == "test expr EQ expr") {
            cout << slt + "$6, $5, $3" << endl;
            cout << slt + "$7, $3, $5" << endl;
            cout << "add $3, $6, $7" << endl;
            cout << "sub $3, $11, $3" << endl;
        }
        else if (t.rule == "test expr NE expr") {
            cout << slt + "$6, $5, $3" << endl;
            cout << slt + "$7, $3, $5" << endl;
            cout << "add $3, $6, $7" << endl;
        }
        else if (t.rule == "test expr LE expr") {
            cout << slt + "$3, $3, $5" << endl;
            cout << "sub $3, $11, $3" << endl;
        }
        else if (t.rule == "test expr GE expr") {
            cout << slt + "$3, $5, $3" << endl;
            cout << "sub $3, $11, $3" << endl;
        }
        else if (t.rule == "test expr GT expr") {
            cout << slt + "$3, $3, $5" << endl;
        }
    }
    else if (t.rule == "statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE "
                       "LBRACE statements RBRACE") {
        ccond++;
        string fal = "FALSEIF" + to_string(ccond), end = "ENDIF" + to_string(ccond);
        code_gen(t.subtree[2]);
        cout << "beq $3, $0," + fal << endl;
        code_gen(t.subtree[5]);
        cout << "beq $0, $0," + end << endl;
        cout << fal + ":" << endl;
        code_gen(t.subtree[9]);
        cout << end + ":" << endl;
    }

    else if (t.rule == "statement WHILE LPAREN test RPAREN LBRACE statements RBRACE") {
        cloop++;
        string top = "TOPLOOP" + to_string(cloop),
               end = "ENDLOOP" + to_string(cloop);
        cout << top + ":" << endl;
        code_gen(t.subtree[2]);
        cout << "bne $3, $11," + end << endl;
        code_gen(t.subtree[5]);
        cout << "beq $0, $0," + top << endl;
        cout << end + ":" << endl;
    }
    else if (t.rule == "factor NULL") { // force crash
        cout << "lis $3" << endl;
        cout << ".word 1" << endl;
    }
    else if (t.rule == "dcls dcls dcl BECOMES NULL SEMI") {
        code_gen(t.subtree[0]);
        string varname = t.subtree[1].subtree[1].tokens[1];
        cout << "lis $3" << endl;
        cout << ".word " << 1 << endl;
        swreg(varname);
        cout << "sub $30, $30, $4" << endl;
    }
    else if (t.rule == "lvalue STAR factor") {
        code_gen(t.subtree[1]);
    }
    else if (t.rule == "factor AMP lvalue") {
        type = "AMP";
        code_gen(t.subtree[1]);
    }
    else if (t.rule == "factor STAR factor") {
        code_gen(t.subtree[1]);
        cout << "lw $3, 0($3)" << endl;
    }
    else if (t.rule == "factor NEW INT LBRACK expr RBRACK") {
        code_gen(t.subtree[3]);
        cout << "add $1, $3, $0" << endl;
        call("new");
        cout << "bne $3, $0, 1" << endl;
        cout << "add $3, $11, $0" << endl;
    }
    else if (t.rule == "statement DELETE LBRACK RBRACK expr SEMI") {
        cdel++;
        code_gen(t.subtree[3]);
        string lnull = "NULL" + to_string(cdel);
        cout << "beq $11, $3, " + lnull << endl;
        cout << "add $1, $0, $3" << endl;
        call("delete");
        cout << lnull + ":" << endl;
    }
}

//calling init and restoring registers
void prologueinit()
{
    push(2);
    if (tST["wain"].first.front() == "int") {
        cout << "add $2, $0, $0" << endl;
    }
    call("init");
    pop(2);
}

// create a tree base on input and do the two passes checks
int main()
{
    Tree parseTree = createTree(std::cin);
    try {
        prologue();
        traverse(parseTree); // generate symbol table
        prologueinit();
        print();
        proc = "";
        updateoffset();
        print();
        code_gen(parseTree.subtree.at(1));
    }
    catch (int e) {
    }
    return 0;
}