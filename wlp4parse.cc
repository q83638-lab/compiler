#include <iostream>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <utility>
#include <sstream>
#include <stack>
#include <queue>
#include <algorithm>

using namespace std;


struct node {
	string symbol;
	vector<node> subtree;
};

map<pair<string, string>, pair<string, string>> actiontable;
vector <vector <string>> ruletable;
queue<pair<string,string>> reads;
vector<node> treeStack;
vector<string> nonterminals;

//treefunctions

node makeTree(string &symbol, vector<node> tree) {
	node temp = { symbol, tree };
	return temp;
}

node makeTree(string &symbol) {
	return makeTree(symbol, vector<node>());
}

node makeTree(string &symbol, node tree) {
	vector<node> temp;
	temp.push_back(tree);
	return makeTree(symbol, temp);
}

//unordered_map<string,string> lexeme;
//Skip the grammar part of the input.
void skipLine(istream &in) {
  string s;
  getline(in, s);
}
string trim(const string &str) {
  size_t begin = str.find_first_not_of(" \t\n");
  if (begin == string::npos) return "";
  size_t end = str.find_last_not_of(" \t\n");
  return str.substr(begin, end - begin + 1);
}

void generateTable(istream &in) {
  int i, numTerm, numNonTerm, numRules,numActions;

  // read the number of terminals and move to the next line
  in >> numTerm;
  skipLine(in);

  // skip the lines containing the terminals
  for (i = 0; i < numTerm; i++) {
    skipLine(in);
  }

  // read the number of non-terminals and move to the next line
  in >> numNonTerm;
  skipLine(in);

  // save the non-terminals
  for (i = 0; i < numNonTerm; i++) {
    string s;
    getline(in, s);
    s = trim(s);
    nonterminals.emplace_back(s);
  }

  // skip the line containing the start symbol
  skipLine(in);

  // read the number of rules and move to the next line
  in >> numRules;

  skipLine(in);

  // skip the lines containing the production rules
  for (i = 0; i < numRules; i++) {
    string s;
    getline(in, s);
    s = trim(s);
     vector <string> temp;
     stringstream check1(s);
         string intermediate;
         while(getline(check1, intermediate, ' '))
             {
                 temp.push_back(intermediate);
             }
         ruletable.emplace_back(temp);
  }
  //skip the number of states
  skipLine(in);
  in >> numActions;
  skipLine(in);
  // read the number of actions
  for (i = 0; i < numActions; i++) {
   string s;
   getline(in, s);
   s = trim(s);
   vector <string> temp;
   stringstream check1(s);
   string intermediate;
   while(getline(check1, intermediate, ' '))
   {
   temp.push_back(intermediate);
   }
    pair <string,string> statesym = make_pair (temp[0],temp[1]);
    pair <string,string> action = make_pair (temp[2],temp[3]);
    actiontable.emplace(statesym, action);
   }

}

void readlexeme(istream &in) {
 // reading input
string s;
vector <string> temp;
while (getline(in, s)) {
s = trim(s);
stringstream check1(s);
string intermediate;

while(getline(check1, intermediate, ' '))
 {
temp.emplace_back(intermediate);
 }

}
reads.push(make_pair("BOF","BOF"));
for (int i = 0; i < temp.size(); i+=2) {
    pair <string,string> lexpair = make_pair (temp[i],temp[i+1]);
   // lexeme.emplace(lexpair);
    reads.push(lexpair);
}

reads.push(make_pair("EOF","EOF"));
}
// Prints the derivation with whitespace trimmed.

void parse() {

stack<string> statestack;
stack<string> symbolstack;
statestack.push("0");
string state;
string symbol;
node tree;
vector<node> children;
treeStack.push_back(makeTree(reads.front().first, makeTree(reads.front().second)));
int k = 0;
do {
symbol = reads.front().first;
state = statestack.top();
pair <string,string> actionpair =  make_pair(state, symbol);
try {
pair <string,string> actionrule = actiontable.at(actionpair);
if (actionrule.first == "shift") {
    symbolstack.push(reads.front().first);
    treeStack.push_back(makeTree(reads.front().first, makeTree(reads.front().second)));
    //cout << reads.front().first << " " << reads.front().second << endl;
    reads.pop();
    state = actionrule.second;
    statestack.push(actionrule.second);
    k++;

}
else while (actionrule.first == "reduce") {
int i = stoi(actionrule.second);
vector<string> rule = ruletable[i];
children.clear();
/*for (auto const &y : rule) {
    cout << y << " ";
}
cout << endl;*/

for (int t = 1; t < rule.size(); t++) {
    statestack.pop();
    symbolstack.pop();
    children.push_back(treeStack[treeStack.size()-1]);
    treeStack.pop_back();
}
state = statestack.top();
symbolstack.push(rule[0]);
symbol = rule[0];
//cout << "reduce: " << state << "," << symbol << endl;
actionpair = make_pair(state, symbol);
actionrule = actiontable.at(actionpair);
state = actionrule.second;
statestack.push(state);
treeStack.push_back(makeTree(rule[0], children));
}
}
catch (const std::out_of_range& oor) {
    std::cerr << "ERROR at " << k+2 << '\n';
    break;
  }

} while(symbolstack.empty()||symbolstack.top()!= "EOF");
}

void traverse(vector<node> &tree) {
	for(int i = tree.size() - 1; i >= 0; --i) {
		if(tree[i].subtree.empty()
		&& count(nonterminals.begin(), nonterminals.end(), tree[i].symbol) == 0) {
		}
		else if(tree[i].symbol == "BOF") {
			cout << "EOF EOF" << endl;
			break;
		}
		else if(tree[i].symbol == "EOF") {
			cout << "BOF BOF" << endl;
		}
		cout << tree[i].symbol;
		for(int j=tree[i].subtree.size()-1; j>=0; --j) {
			cout << " " << tree[i].subtree[j].symbol;
		}
		cout << endl;
		traverse(tree[i].subtree);
	}
}

// Reads a .cfg file and prints the left-canonical
// derivation without leading or trailing spaces.
int main() {
  ifstream myfile ("WLP4.lr1");
  generateTable(myfile);
  readlexeme(std::cin);
  parse();
  traverse(treeStack);
}
