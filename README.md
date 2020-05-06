# compiler
translate higher-level programming languages (subset of c++) to lower-level languages (e.g. machine code). 

Declarations, control structures and statements that may be used are restricted to:
int or int*(declaration of a single int or int* variable with an unsigned integer constant or NULL initializer; all declarations in WLP4 must precede all statements and control structures; every declaration must include an integer constant or NULL initializer)
if (must have an else clause)
while
return (must be the last statement in function)
println
= (i.e. assignment)
delete [] (i.e. deallocation)

To compile: cat example.cpp | ./scan | ./parse | ./gen > out.asm

