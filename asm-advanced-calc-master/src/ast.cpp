#include "ast.h"
#include <iostream>
#include <sstream>
#include <set>
#include "asm.h"
#include <map>

const char * floatTemps[] = {"$f0",
                            "$f1",
                            "$f2",
                            "$f3",
                            "$f4",
                            "$f5",
                            "$f6",
                            "$f7",
                            "$f8",
                            "$f9",
                            "$f10",
                            "$f11",
                            "$f12",
                            "$f13",
                            "$f14",
                            "$f15",
                            "$f16",
                            "$f17",
                            "$f18",
                            "$f19",
                            "$f20",
                            "$f21",
                            "$f22",
                            "$f23",
                            "$f24",
                            "$f25",
                            "$f26",
                            "$f27",
                            "$f28",
                            "$f29",
                            "$f30",
                            "$f31"
                        };

#define FLOAT_TEMP_COUNT 32
set<string> intTempMap;
set<string> floatTempMap;

extern Asm assemblyFile;

int globalStackPointer = 0;
int labelCounter = 0;

class variableData{

    public:
        variableData(int offset){

            this->offset = offset;

        }

    int offset;

};

map<string, variableData*> localVariables;

string getFloatTemp(){
    for (int i = 0; i < FLOAT_TEMP_COUNT; i++)
    {
        if(floatTempMap.find(floatTemps[i]) == floatTempMap.end()){
            floatTempMap.insert(floatTemps[i]);
            return string(floatTemps[i]);
        }
    }
    cout<<"No more float registers!"<<endl;
    return "";
}

//Done
string getNewLabel(string label){

    stringstream ss;
    ss<<label<<"_"<<labelCounter;
    labelCounter++;
    return ss.str();

}

void releaseFloatTemp(string temp){
    floatTempMap.erase(temp);
}

//Done
void FloatExpr::genCode(Code &code){

    string floatTemp = getFloatTemp();
    code.place = floatTemp;
    stringstream ss;

    ss<<"li.s "<<floatTemp<<", "<<this->number<<endl;
    code.code = ss.str();
    
}

//Done
void SubExpr::genCode(Code &code){

    Code leftCode;
    Code rightCode;
    this->expr1->genCode(leftCode);
    this->expr2->genCode(rightCode);
    code.code = leftCode.code + "\n" + rightCode.code + "\n";
    string newTemp = getFloatTemp();
    code.code = "sub.s " + newTemp + ", "+leftCode.place+", "+rightCode.place+"\n";
    releaseFloatTemp(leftCode.place);
    releaseFloatTemp(rightCode.place);

}

//Done
void DivExpr::genCode(Code &code){

    Code leftCode;
    Code rightCode;
    this->expr1->genCode(leftCode);
    this->expr2->genCode(rightCode);
    code.code = leftCode.code + "\n" + rightCode.code + "\n";
    string newTemp = getFloatTemp();
    code.code = "div.s "+ newTemp + ", " + leftCode.place + ", " + rightCode.place + "\n";
    releaseFloatTemp(leftCode.place);
    releaseFloatTemp(rightCode.place);

}

//Done
void IdExpr::genCode(Code &code){

    if(localVariables.find(this->id) == localVariables.end()){

        string floatTemp = getFloatTemp();
        code.place = floatTemp;
        code.code = "l.s " + floatTemp + ", "+this->id + "\n";

    }else{

        string floatTemp = getFloatTemp();
        code.place = floatTemp;
        code.code = "l.s " + floatTemp + ", "+to_string(localVariables[this->id]->offset) + "($sp)\n";

    }

}

//Done
string ExprStatement::genCode(){
    // return "Expr statement code generation\n";

    Code exprCode;
    this->expr->genCode(exprCode);
    releaseFloatTemp(exprCode.place);
    return exprCode.code;

}

//Revisar
string IfStatement::genCode(){
    // return "If statement code generation\n";

    string end_if = getNewLabel("end_if");
    Code exprCode;

    this->conditionalExpr->genCode(exprCode);
    stringstream code;

    code<<exprCode.code<<endl;
    
    code << "bc1f " << end_if <<endl;
    // code<<this->trueStatement->genCode()<<endl
    // <<end_if<<": "<<endl;

    releaseFloatTemp(exprCode.place);
    return code.str();


}


void MethodInvocationExpr::genCode(Code &code){
    
}

//Done
string AssignationStatement::genCode(){
    // return "Assignation statement code generation\n";

    if(localVariables.find(this->id) == localVariables.end()){

        localVariables[this->id] = new variableData(globalStackPointer);
        globalStackPointer+=4;

    }

    stringstream ss;
    Code value;
    this->value->genCode(value);
    ss<<value.code<<endl;
    ss<< "s.s "<< value.place <<", "<<localVariables[this->id]->offset<<"($sp)"<<endl;
    releaseFloatTemp(value.place);
    return ss.str();

}

//Done
void GteExpr::genCode(Code &code){

    stringstream ss;
    ss<<code.code<<endl;

    Code leftCode;
    Code rightCode;

    this->expr1->genCode(leftCode);
    this->expr2->genCode(rightCode);
    string temp = getFloatTemp();

    ss<<"c.lt.s "<<rightCode.place<<", "<<leftCode.place<<endl;
    releaseFloatTemp(leftCode.place);
    releaseFloatTemp(rightCode.place);
    code.place = temp;
    code.code = ss.str();

}

//Done
void LteExpr::genCode(Code &code){

    stringstream ss;
    ss<<code.code<<endl;
    Code leftCode;
    Code rightCode;

    this->expr1->genCode(leftCode);
    this->expr2->genCode(rightCode);

    string newTemp = getFloatTemp();
    ss<<"c.lt.s "<<leftCode.place<<", "<<rightCode.place<<endl;
    releaseFloatTemp(leftCode.place);
    releaseFloatTemp(rightCode.place);

    code.place = newTemp;
    code.code = ss.str();

}

//Done
void EqExpr::genCode(Code &code){

    stringstream ss;
    ss<<code.code<<endl;

    Code leftCode;
    Code rightCode;

    this->expr1->genCode(leftCode);
    this->expr2->genCode(rightCode);

    string newTemp = getFloatTemp();

    ss<<"c.eq.s "<<leftCode.place<<", "<<rightCode.place<<endl;
    releaseFloatTemp(leftCode.place);
    releaseFloatTemp(rightCode.place);

    code.place = newTemp;
    code.code = ss.str();

}

//Done
void ReadFloatExpr::genCode(Code &code){
    
    stringstream ss;
    ss <<code.code<<endl;
    ss<<"li $v0, 6"<<endl
    <<"syscall"<<endl;
    code.code = ss.str();

}

//Revisar
string PrintStatement::genCode(){
    // return "Print statement code generation\n";

    Code exprCode;
    stringstream ss;
    ss<<"mov.s $f12, "<<exprCode.place<<endl
    <<"li $v0, 2"<<endl
    <<"syscall"<<endl;

    return ss.str();

}

//Done
string ReturnStatement::genCode(){
    // return "Return statement code generation\n";

    Code exprCode;
    this->expr->genCode(exprCode);
    releaseFloatTemp(exprCode.place);
    stringstream ss;
    ss<<exprCode.code<<endl;
    ss<<"mfc1 $v0, "<<exprCode.place<<endl;
    return ss.str();

}

string MethodDefinitionStatement::genCode(){
    return "Method definition code generation\n";
}