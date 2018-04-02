#include "QtKeyTool.h"

string QtKeyTool::itos(int i)
{
    ostringstream os;
    os<<i;
    string result;
    istringstream is(os.str());
    is>>result;
    return result;
}

string QtKeyTool::ftos(float f)
{
    ostringstream os;
    os<<f;
    string result;
    istringstream is(os.str());
    is>>result;
    return result;
}

string QtKeyTool::dtos(double d)
{
    ostringstream os;
    os<<d;
    string result;
    istringstream is(os.str());
    is>>result;
    return result;
}

string QtKeyTool::ltos(long l)  
{  
    ostringstream os;  
    os<<l;  
    string result;  
    istringstream is(os.str());  
    is>>result;  
    return result;  

}  

long QtKeyTool::stol(string str)
{
    long result;
    istringstream is(str);
    is >> result;
    return result;
}

int QtKeyTool::stoi(string str)
{
    int result;
    istringstream is(str);
    is >> result;
    return result;
}

float QtKeyTool::stof(string str)
{
    float result;
    istringstream is(str);
    is >> result;
    return result;
}

double QtKeyTool::stod(string str)
{
    double result;
    istringstream is(str);
    is >> result;
    return result;
}

long QtKeyTool::sListtol(string &str)
{
	long result;
	string long_tmp;
	char tmp;
	int i = 0;
    istringstream is(str);
    is >> tmp;
    i++;
    if(0 == strncmp(&tmp, ";", 1))
    {
    	str.substr(i);
    	result = stol(long_tmp);
		return result;
    }
	long_tmp += tmp;
}

