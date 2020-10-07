#pragma once
#include <Windows.h>
#include <string>
using namespace std;
class tool
{
public:


	void mmap(string process, string dllpath);
    void loadLibrary(string process, string dllpath);
	void set_console(int w, int h);
	void setupConsole(std::string consoletitle, int w, int h);
	void checkInternet();
	int GetProcessIdByName(const std::string& p_name);
	string httpRequest(string site, string param);
	void checkPrivileges();
	bool downloadFile(string url, string filepath);




	tool();
	~tool();
};

