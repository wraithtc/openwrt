#include <sstream>
#include <string> 
#include <cstring> 

using namespace std;

class QtKeyTool
{
public:
	string itos(int i);
	string ftos(float f);
	string dtos(double d);
	string ltos(long l);
	long stol(string str);
	int stoi(string str);
	float stof(string str);
	double stod(string str);
	long sListtol(string &str);
	
protected:
	void stolList(string str);
};

