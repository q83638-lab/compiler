#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include "scanner.h"

/*
 * C++ Starter code for CS241 A3
 * All code requires C++14, so if you're getting compile errors make sure to
 * use -std=c++14.
 *
 * This file contains the main function of your program. By default, it just
 * prints the scanned list of tokens back to standard output.
 */
void printNum (int i) {
char c = (i >> 24) & 0xff;
std::cout << c;
c = (i >> 16) & 0xff;
std::cout << c;
c = (i >> 8) & 0xff;
std::cout << c;
c = i & 0xff;
std::cout << c;
}

int printJrJalr (int i, int command) {
int c = (i << 21);
return (c | command);
}
int printlismf (int i, int command) {
int c = (i << 11);
return (c | command);
}

int printmultdiv (int s1, int t1,int command) {
int s = (s1<< 21);
int t = (t1<< 16);
return (s | t| command);
}
int printasslt (int i1, int i2, int i3, int command) {
int s = (i1 << 21);
int t = (i2 << 16);
int d = (i3 << 11);
return ( s | t| d | command);
}

int printbeqbne(int s1, int t1, int i1, int command) {
int c = (command << 26);
int s = (s1 << 21);
int t = (t1 << 16);
int i =  (i1 & 0xffff);
return ( c|s | t|  i);
}

int main (){
std::string line;
try{
std::map < std::string, int >label_table;
int lnum = 0;
std:: vector <std::vector<Token>> tokens_read;
std:: vector <int> inds;
int ind = 0;
 while (getline (std::cin, line)) {
   ind = 0;
    std::vector < Token > tokenLine = scan (line);
        tokens_read.emplace_back(tokenLine);
        if (!tokenLine.empty ()){
            if (tokenLine[ind].getKind() == Token::LABEL) {
                for (auto &token : tokenLine) {
                     if (token.getKind () == Token::LABEL) {
                           if (label_table.count(token.getLexeme().substr (0, (token.getLexeme()).length ()-1)) != 0)
                            {
                             throw ScanningFailure("ERRORS dup label");
                            }
                     else {
                           label_table.emplace (token.getLexeme().substr (0, (token.getLexeme()).length ()-1), lnum);
                           ind++;
                          }
                      }
                }
            }
            if (tokenLine.size()== ind+2 || tokenLine.size()== ind+6 || (tokenLine.size()== ind+ 4) || tokenLine.size()== ind+7) {
             lnum+=4;
            }
                       }

	inds.emplace_back(ind);
    }
int i = 0;
int currentline = 0;
    for (auto &tokenline : tokens_read){
         if (!tokenline.empty()){
            if (tokenline.size()== inds[i]+2) {
                 if (tokenline[inds[i]].getKind() == Token::WORD) {
                          if (tokenline[inds[i]+1].getKind() == Token::INT || tokenline[inds[i]+1].getKind() == Token::HEXINT) {
                               int64_t num = tokenline[inds[i]+1].toLong();
                                if (num>=-2147483648&&num <=4294967295) {
                                    printNum(num);
                                 }
                          }
                          else if (tokenline[inds[i]+1].getKind() == Token::ID) {
                                                              if (label_table.count(tokenline[inds[i]+1].getLexeme()) != 0)
                                                                  {
                                                                 printNum(label_table.find(tokenline[inds[i]+1].getLexeme())->second);
                                                                  }
                                                               else
                                                              throw ScanningFailure("ERRORS not in table");
                                                         }
                          else{
                                 throw ScanningFailure("ERRORS not a valid word");
                          }
                 }
                 else if ((tokenline[inds[i]]).getKind() == Token::ID ) {

                    if ( tokenline[inds[i]+1].getKind() == Token::REG)
                        {
                        std::string reg = tokenline[inds[i]+1].getLexeme().substr(1, (tokenline[inds[i]+1].getLexeme()).length ());
			            std::stringstream ss(reg);
                        int regint = 0;
                        ss >> regint;
			            if ( regint >= 0 && regint <= 31) {
			             if ((tokenline[inds[i]]).getLexeme() =="jr"){
                      	 printNum(printJrJalr(regint,8));
                      	 } else if  ((tokenline[inds[i]]).getLexeme() =="jalr")
                      	 {
                      	 printNum(printJrJalr(regint,9));
                      	 } else if (tokenline[inds[i]].getLexeme() == "lis") {
                      	 printNum(printlismf(regint,20));
                      	 }  else if (tokenline[inds[i]].getLexeme() == "mfhi") {
                                                 	 printNum(printlismf(regint,16));
                                                 	 }
                            else if (tokenline[inds[i]].getLexeme() == "mflo") {
                                                                             	 printNum(printlismf(regint,18));
                                                                             	 }
                     	 else
                      	    throw ScanningFailure("ERRORS not a valid command");
                      	 }
				      else
					                                  throw ScanningFailure("ERRORS not in range");
                        }

		    else
		      throw ScanningFailure("ERRORS not a valid reg");

		 }else {
                        throw ScanningFailure("ERRORS not a 2 word command");
                        }
              currentline+=4;
            }
            else if (tokenline.size()== inds[i]+ 4) {
            if ((tokenline[inds[i]]).getKind() == Token::ID  &&
                           tokenline[inds[i]+1].getKind() == Token::Token::REG &&
                           tokenline[inds[i]+2].getKind() == Token::Token::COMMA &&
                           tokenline[inds[i]+3].getKind() == Token::Token::REG)
             {
               std::string reg1 = tokenline[inds[i]+1].getLexeme().substr(1, (tokenline[inds[i]+1].getLexeme()).length());
               std::string reg2 = tokenline[inds[i]+3].getLexeme().substr(1, (tokenline[inds[i]+3].getLexeme()).length());
               int r1 = std::stoi(reg1);
               int r2 = std::stoi(reg2);
               if ((r1 >= 0 && r1 <= 31) && (r2 >= 0 && r2 <= 31)) {
               if ((tokenline[inds[i]]).getLexeme() =="mult" ) {
               printNum(printmultdiv (r1,r2,24));
               } else if ((tokenline[inds[i]]).getLexeme() =="multu" )
                printNum(printmultdiv (r1,r2,25));
                else if ((tokenline[inds[i]]).getLexeme() =="div" )
                printNum(printmultdiv (r1,r2,26));
                else if ((tokenline[inds[i]]).getLexeme() =="divu" )
                printNum(printmultdiv (r1,r2,27));
                else throw ScanningFailure("ERRORS not a valid command");
               } else throw ScanningFailure("ERRORS not a valid range");
             } else throw ScanningFailure("ERRORS not a valid 4 part command");
             currentline+=4;
            }
            else if (tokenline.size()== inds[i]+ 6) {
            if ((tokenline[inds[i]]).getKind() == Token::ID  &&
            tokenline[inds[i]+1].getKind() == Token::Token::REG &&
            tokenline[inds[i]+2].getKind() == Token::Token::COMMA &&
            tokenline[inds[i]+3].getKind() == Token::Token::REG &&
            tokenline[inds[i]+4].getKind() == Token::Token::COMMA &&
            tokenline[inds[i]+5].getKind() == Token::Token::REG) {
            std::string reg1 = tokenline[inds[i]+1].getLexeme().substr(1, (tokenline[inds[i]+1].getLexeme()).length ());
            std::string reg2 = tokenline[inds[i]+3].getLexeme().substr(1, (tokenline[inds[i]+3].getLexeme()).length ());
            std::string reg3 = tokenline[inds[i]+5].getLexeme().substr(1, (tokenline[inds[i]+5].getLexeme()).length ());
            int r1 = std::stoi(reg1);
            int r2 = std::stoi(reg2);
            int r3 = std::stoi(reg3);
            if ((r1 >= 0 && r1 <= 31) && (r2 >= 0 && r2 <= 31) &&(r3 >= 0 && r3 <= 31)) {
             if ((tokenline[inds[i]]).getLexeme() =="sub"){
                printNum(printasslt(r2,r3,r1,34));
              } else if  ((tokenline[inds[i]]).getLexeme() =="add") {
                printNum(printasslt(r2,r3,r1,32));
              } else if ((tokenline[inds[i]]).getLexeme() =="slt") {
                printNum(printasslt(r2,r3,r1,42));
              } else if  ((tokenline[inds[i]]).getLexeme() =="sltu") {
                printNum(printasslt(r2,r3,r1,43));
              }
              else throw ScanningFailure("ERRORS not a valid type");

            } else throw ScanningFailure("ERRORS not a valid range");

            }
             else if ((tokenline[inds[i]]).getKind() == Token::ID  &&
                        tokenline[inds[i]+1].getKind() == Token::REG &&
                        tokenline[inds[i]+2].getKind() == Token::COMMA &&
                        tokenline[inds[i]+3].getKind() == Token::REG &&
                        tokenline[inds[i]+4].getKind() == Token::COMMA ) {
                         std::string reg1 = tokenline[inds[i]+1].getLexeme().substr(1, (tokenline[inds[i]+1].getLexeme()).length ());
                         std::string reg2 = tokenline[inds[i]+3].getLexeme().substr(1, (tokenline[inds[i]+3].getLexeme()).length ());
                         int r1 = std::stoi(reg1);
                         int r2 = std::stoi(reg2);
                         int64_t num=0;
                          if (tokenline[inds[i]+5].getKind() == Token::INT || tokenline[inds[i]+5].getKind() == Token:: HEXINT)
                        num = tokenline[inds[i]+5].toLong();
                         if (tokenline[inds[i]+5].getKind() == Token::ID)  {
			   if (label_table.count(tokenline[inds[i]+5].getLexeme()) == 0)
				   throw ScanningFailure("ERRORS not found in label table");
			int labelline = label_table.find(tokenline[inds[i]+5].getLexeme())->second;	

                         num = (labelline-currentline-4)/4;

                         }
                 if ((r1 >= 0 && r1 <= 31) && (r2 >= 0 && r2 <= 31) && (((tokenline[inds[i]+5].getKind() == Token::INT
                 ||  tokenline[inds[i]+5].getKind() == Token::ID )&& (num>=-32768 && num<=32767)) ||
			(tokenline[inds[i]+5].getKind() == Token:: HEXINT&& num<=65535))){
                  if ((tokenline[inds[i]]).getLexeme() =="beq"){
                         printNum(printbeqbne(r1, r2, num, 4));
                } else if  ((tokenline[inds[i]]).getLexeme() =="bne") {
                         printNum(printbeqbne(r1, r2, num, 5));
                 } else throw ScanningFailure("ERRORS not a valid command");
                  }else throw ScanningFailure("ERRORS not a valid range");

             }

            else throw ScanningFailure("ERRORS not a valid 3 fields instruction");
             currentline+=4;
            }
            else if (tokenline.size()== inds[i]+ 7)  {
            if ((tokenline[inds[i]]).getKind() == Token::ID  &&
                        tokenline[inds[i]+1].getKind() == Token::Token::REG &&
                        tokenline[inds[i]+2].getKind() == Token::Token::COMMA &&
                        (tokenline[inds[i]+3].getKind() == Token::Token::INT ||
                        tokenline[inds[i]+3].getKind() == Token::Token::HEXINT)
                        &&
                        tokenline[inds[i]+4].getKind() == Token::LPAREN &&
                        tokenline[inds[i]+5].getKind() == Token::Token::REG
                        &&  tokenline[inds[i]+6].getKind() == Token::RPAREN) {
                        std::string reg1 = tokenline[inds[i]+1].getLexeme().substr(1, (tokenline[inds[i]+1].getLexeme()).length ());
                        std::string reg2 = tokenline[inds[i]+5].getLexeme().substr(1, (tokenline[inds[i]+5].getLexeme()).length ());
                        int r1 = std::stoi(reg1);
                        int r2 = std::stoi(reg2);
                        int64_t num= tokenline[inds[i]+3].toLong();
                        if ((r1 >= 0 && r1 <= 31) && (r2 >= 0 && r2 <= 31) && (tokenline[inds[i]+3].getKind() == Token::INT
                        && (num>=-32768 && num<=32767) ||
                        (tokenline[inds[i]+3].getKind() == Token:: HEXINT&& num<=65535)) ) {
                         if ((tokenline[inds[i]]).getLexeme() =="lw"){
                            printNum(printbeqbne(r1, r2, num, 35));
                          } else if  ((tokenline[inds[i]]).getLexeme() =="sw") {
                            printNum(printbeqbne(r1, r2, num, 43));
                          }
                          else throw ScanningFailure("ERRORS not a valid type");

                        } else throw ScanningFailure("ERRORS not a valid range");

                        }
            }


            else if (tokenline[0].getKind() == Token :: LABEL);
	    
            else
             {
                        throw ScanningFailure("ERRORS not a word or label");
             }
        }
	 i++;
    }

 } catch (ScanningFailure & f){
    std::cerr << f.what () << std::endl;
    return 1;
    }
return 0;
}
