#include "INode.h"
#include "File.h"
#include "FileSystem.h"
#include "OpenFileManager.h"
#include "FileManager.h"
#include "User.h"
#include <iostream>
using namespace std;

DeviceDriver g_DeviceDriver;
SuperBlock g_SuperBlock;
BufferManager g_BufferManager;
OpenFileTable g_OpenFileTable;
FileSystem g_FileSystem;
INodeTable g_INodeTable;
FileManager g_FileManager;
User g_User;

void cmdArgs(const string& cmd, vector<string>& args) {
    args.clear();
    string str;
	unsigned int p, q;
	for (p = 0, q = 0; q < cmd.length(); p = q + 1) {
		q = cmd.find_first_of(" ");
		str = cmd.substr(p, q - p);
		if(!str.empty()){
			args.push_back(str);
		}
    }
}

int main() {
	User* user = &g_User;

    string cmd="help";
	vector<string> args;
    string arg0, arg1, arg2;
    cout << "++++++++++++++++++++ Unix文件系统模拟 ++++++++++++++++++++" << endl;
    cout << "请输入命令: " << endl;

    while (1) {
        if (cmd != "") {
            cmdArgs(cmd, args);
            cmd = args[0];
        }
        if (cmd == "help") {
            cout << "help" << endl;
            cout << "fformat" << endl;
        }
        else if (cmd == "fformat") {

        }
        else if (cmd == "ls") {

        }
        else if (cmd == "mkdir") {

        }
        else if (cmd == "touch") {

        }
        else if (cmd == "write") {

        }
        else if (cmd == "read") {

        }
        else if (cmd == "cd") {

        }
        else if (cmd != "") {
            cout << "warning: 您输入的命令不存在！" << endl;
        }
        cout << "[unix-fs ]$ ";
        getline(cin, cmd);
    }
    return 0;
}