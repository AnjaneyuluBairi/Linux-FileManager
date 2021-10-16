#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <bits/stdc++.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <termios.h>
#include <unistd.h>
#include <pwd.h>
using namespace std;
#include <grp.h>
#define CLEAR printf("\033[H\033[J");

string userHome;
struct winsize tws;
int start_row = 0, end_row;
int curr_row;
int display_row = 6;

int win_row;
struct termios org_termios, new_termios; // termios structres
char cwd[1024];							 // for current working dir
string root;
stack<string> back;
stack<string> forw;
vector<string> contents;

void gotoxy(int x, int y)
{
	printf("\033[%d;%dH", x, y);
}

void printModeMsg(string msg)
{
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	gotoxy(w.ws_row - 1, 0);
	cout << msg << endl;
	gotoxy(0, 0);
}

string normalizeFileSize(long long FileSize)
{
	if (FileSize >= (1 << 30))
	{
		return (to_string(FileSize / (1 << 30)) + "G");
	}
	else if (FileSize >= (1 << 20))
	{
		return (to_string(FileSize / (1 << 20)) + "M");
	}
	else if (FileSize >= (1 << 10))
	{
		return (to_string(FileSize / (1 << 10)) + "K");
	}
	return (to_string(FileSize) + "B");
}
void Push(stack<string> &stack, char *dir)
{
	if (stack.empty())
	{
		stack.push(dir);
		return;
	}
	if (stack.top() != dir)
	{
		stack.push(dir);
	}
	return;
}
void InitialSettingsOfNormalMode()
{
	tcgetattr(STDIN_FILENO, &org_termios);
	new_termios = org_termios;
	new_termios.c_lflag &= (~ICANON);
	new_termios.c_lflag &= (~ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
	getcwd(cwd, 1024);
	root = cwd; // storing root path(start) in root
}

void ReadCurDir(vector<string> &contents)
{
	char str_dir[1024];
	getcwd(str_dir, 1024);
	contents.push_back(".");
	if (str_dir != userHome) // root or userHome
	{
		contents.push_back("..");
	}
	DIR *CurDir = opendir(".");
	struct dirent *PtrDirDrnt = readdir(CurDir);
	while (PtrDirDrnt)
	{
		if (PtrDirDrnt->d_name[0] != '.')
		{
			contents.push_back((PtrDirDrnt->d_name));
		}
		PtrDirDrnt = readdir(CurDir);
	}
}

void DisplayCurDir()
{
	// normal_mode_commands();
	printModeMsg("Normal Mode");
	DIR *CurDir = opendir(".");
	struct stat Status;
	mode_t St_Mode;
	curr_row = 0;
	struct dirent *PtrDirDrnt = readdir(CurDir);
	char str_dir[1024];
	getcwd(str_dir, 1024);
	cout << ".\n";
	curr_row++;
	if (str_dir != userHome)
	{
		cout << "..\n";
		curr_row++;
	}
	while (PtrDirDrnt)
	{
		stat(PtrDirDrnt->d_name, &Status);
		St_Mode = Status.st_mode;
		string Permissions = "";
		if (PtrDirDrnt->d_name[0] != '.')
		{
			Permissions += ((St_Mode & S_IFDIR) ? 'd' : 'f'); // isADirectory
			Permissions += ((St_Mode & S_IRUSR) ? 'r' : '-');
			Permissions += ((St_Mode & S_IWUSR) ? 'w' : '-');
			Permissions += ((St_Mode & S_IXUSR) ? 'x' : '-');
			Permissions += ((St_Mode & S_IRGRP) ? 'r' : '-');
			Permissions += ((St_Mode & S_IWGRP) ? 'w' : '-');
			Permissions += ((St_Mode & S_IXGRP) ? 'x' : '-');
			Permissions += ((St_Mode & S_IROTH) ? 'r' : '-');
			Permissions += ((St_Mode & S_IWOTH) ? 'w' : '-');
			Permissions += ((St_Mode & S_IXOTH) ? 'x' : '-');

			string LastModTime = ctime(&Status.st_mtime);
			LastModTime.pop_back();
			string name = PtrDirDrnt->d_name;

			double FileSize = (Status.st_size) / 1.0;
			string sizeOfFile = normalizeFileSize(FileSize);

			struct passwd *UserOwner = getpwuid(Status.st_uid);
			struct group *GroupOwner = getgrgid(Status.st_gid);
			cout << fixed << setprecision(2);
			cout << left << setw(20) << Permissions;
			cout << setw(10) << sizeOfFile << setw(10) << UserOwner->pw_name << setw(10) << GroupOwner->gr_name << setw(10) << LastModTime << "\t" << name;
			cout << endl;
			curr_row++;
			if (curr_row == display_row)
			{
				break;
			}
		}
		PtrDirDrnt = readdir(CurDir);
	}
	gotoxy(0, 0);
	start_row = 0;
}

void DisplayRow(vector<string> &content, int start_row)
{
	const char *str_dir = contents[start_row].c_str();
	struct stat Status;
	mode_t St_Mode;
	stat(str_dir, &Status);
	if (str_dir == ".")
	{
		cout << "." << endl;
		return;
	}
	if (str_dir == "..")
	{
		cout << ".." << endl;
		return;
	}
	St_Mode = Status.st_mode;
	string Permissions = "";

	Permissions += ((St_Mode & S_IFDIR) ? 'd' : 'f'); // isADirectory
	Permissions += ((St_Mode & S_IRUSR) ? 'r' : '-');
	Permissions += ((St_Mode & S_IWUSR) ? 'w' : '-');
	Permissions += ((St_Mode & S_IXUSR) ? 'x' : '-');
	Permissions += ((St_Mode & S_IRGRP) ? 'r' : '-');
	Permissions += ((St_Mode & S_IWGRP) ? 'w' : '-');
	Permissions += ((St_Mode & S_IXGRP) ? 'x' : '-');
	Permissions += ((St_Mode & S_IROTH) ? 'r' : '-');
	Permissions += ((St_Mode & S_IWOTH) ? 'w' : '-');
	Permissions += ((St_Mode & S_IXOTH) ? 'x' : '-');

	string LastModTime = ctime(&Status.st_mtime);
	LastModTime.pop_back();
	string name = contents[start_row];
	// double FileSize = (Status.st_size) / 1024.0;
	double FileSize = (Status.st_size) / 1.0;
	string sizeOfFile = normalizeFileSize(FileSize);

	struct passwd *UserOwner = getpwuid(Status.st_uid);
	struct group *GroupOwner = getgrgid(Status.st_gid);
	cout << fixed << setprecision(2);
	cout << left << setw(20) << Permissions;
	cout << setw(10) << sizeOfFile << setw(10) << UserOwner->pw_name << setw(10) << GroupOwner->gr_name << setw(10) << LastModTime << "\t" << name;
	cout << endl;
}

char NormalMode()
{
	char ch;
	// vector<string> contents;
	ReadCurDir(contents);
	DisplayCurDir();
	int contents_size = contents.size();
	int ptr = 0;
	while (true)
	{
		ch = cin.get();
		switch (ch)
		{

		case 10:
		{
			if (ptr >= 0 && ptr < contents_size)
			{
				CLEAR;
				struct stat Status;
				stat(contents[ptr].c_str(), &Status);
				bool isDir = (Status.st_mode & S_IFDIR);
				if (isDir) // for directory
				{
					string DestinationDir = contents[ptr];
					DestinationDir.insert(0, "/");
					char str_dir[1024];
					getcwd(str_dir, 1024);
					DestinationDir = str_dir + DestinationDir;
					if (contents[ptr] == "..")
					{
						Push(forw, str_dir);
					}
					else
					{
						Push(back, str_dir);
					}
					int err = chdir(DestinationDir.c_str());
					if (err == -1)
					{
						cout << "unable to open" << endl;
					}
					getcwd(str_dir, 1024);
					contents.clear();
					ReadCurDir(contents);
					DisplayCurDir();
					contents_size = contents.size();
					ptr = 0;
					// ptr = contents.size();
				}
				else // for file handling
				{

					if (fork() == 0)
					{ // child process
						string command = "vi";
						string FileName = contents[ptr];
						char *arguments[3] = {(char *)command.c_str(), (char *)FileName.c_str(), NULL};
						// arguments[0] = (char *)command.c_str();
						// arguments[1] = (char *)FileName.c_str();
						// arguments[2] = NULL;
						execvp((char *)command.c_str(), arguments);
					}
					else
					{
						// cout << "Parent" << endl;
						wait(0);
					}
					return 'r';
				}
			}

			break;
		}

		case 'h':
		{
			while (!forw.empty())
			{
				forw.pop();
			}
			CLEAR;
			chdir(root.c_str());
			contents.clear();
			ReadCurDir(contents);
			DisplayCurDir();
			contents_size = contents.size();
			ptr = 0;
			// ptr = contents.size();
			break;
		}
		case 127: // backspace
		{
			char str_dir[1024];
			getcwd(str_dir, 1024);
			if (str_dir != root)
			{
				CLEAR;
				string DestinationDir = str_dir;
				while (DestinationDir[DestinationDir.size() - 1] != '/')
				{
					DestinationDir.pop_back();
				}
				DestinationDir.pop_back();
				Push(forw, str_dir);
				chdir(DestinationDir.c_str());
				contents.clear();
				ReadCurDir(contents);
				DisplayCurDir();
				contents_size = contents.size();
				ptr = 0;
				// ptr = contents.size();
			}
			break;
		}
		case 'A': // up arrow
		{		  // up
			if (ptr == 0 || ptr == start_row)
				break;
			ptr--;
			printf("\033[1A");
			break;
		}
		case 'B': // down arrow
		{
			if (ptr >= contents_size || ptr == start_row + display_row - 1)
				break;
			ptr++;
			printf("\033[1B");
			break;
		}
		case 'k':
		{ // scroll down

			if (ptr < contents.size() - 1 && ptr == start_row + display_row - 1) // start_row + display_row < contents_size)
			{
				CLEAR;
				start_row++;
				// ptr=start_row;
				for (int row = 0; row < display_row; row++)
				{
					DisplayRow(contents, start_row + row);
					// ptr++;
				}
				// printf("\033[1B");
				// gotoxy(0,0);
				printModeMsg("Normal Mode");
				gotoxy(display_row, 0);
				ptr = start_row + display_row - 1;
			}
			break;
		}
		case 'l':
		{										   // scroll up
			if (ptr == start_row && start_row > 0) // start_row  > 0)
			{
				start_row--;
				ptr = start_row;
				CLEAR;
				for (int row = 0; row < display_row; row++)
				{
					DisplayRow(contents, start_row + row);
				}
				// gotoxy(display_row,0);
				printModeMsg("Normal Mode");
				gotoxy(0, 0);
				// printf("\033[1A");
			}
			break;
		}
		case 'C': // left // backward
		{
			if (!back.empty())
			{
				char str_dir[1024];
				getcwd(str_dir, 1024);
				Push(forw, str_dir);
				CLEAR;
				string DestinationDir = back.top();
				back.pop();
				chdir(DestinationDir.c_str());
				contents.clear();
				ReadCurDir(contents);
				DisplayCurDir();
				contents_size = contents.size();
				ptr = 0;
				// ptr = contents.size();
			}
			break;
		}
		case 'D': // right arrow  //forward
		{
			if (!forw.empty())
			{
				char str_dir[1024];
				getcwd(str_dir, 1024);
				Push(back, str_dir);
				CLEAR;
				string DestinationDir = forw.top();
				forw.pop();
				chdir(DestinationDir.c_str());
				contents.clear();
				ReadCurDir(contents);
				DisplayCurDir();
				contents_size = contents.size();
				ptr = 0;
				// ptr = contents.size();
			}
			break;
		}
		case 'q':
		{
			CLEAR;
			return ch;
		}
		case ':':
		{
			return ch;
		}
		}
	}
}

string GetAbsPath(string path)
{

	string AbsPath = "";
	char I_CHAR = path[0];
	char str_dir[1024];
	getcwd(str_dir, 1024);

	if (I_CHAR == '.') // current Directory
	{
		if (path[1] == '.')
		{ //.. case
			AbsPath = string(getcwd(str_dir, 1024)) + path.substr(2, path.length());
		}
		else
		{
			AbsPath = string(getcwd(str_dir, 1024)) + path.substr(1, path.length());
		}
	}
	else if (I_CHAR == '~') // userhome
	{
		AbsPath = userHome + path.substr(1, path.length());
	}
	else if (I_CHAR == '/')
	{
		AbsPath = path;
		// AbsPath = string(getcwd(str_dir, 1024)) + "/" + path.substr(1, path.length());
	}
	else
	{ // relative to cwd
		AbsPath = string(getcwd(str_dir, 1024)) + "/" + path;
	}
	return AbsPath;
}

string NoramlizePath(string path)
{
	vector<string> vec;
	string temp = "";
	for (int i = 1; i < path.length(); i++)
	{
		if (path[i] == '/')
		{
			if (temp == ".")
			{
				temp = "";
				// continue;
			}
			else if (temp == "..")
			{
				vec.pop_back();
				temp = "";
				//	continue;
			}
			else
			{
				vec.push_back(temp);
				temp = "";
			}
		}
		else
		{
			temp = temp + path[i];
		}
	}
	string finalPath;
	;
	if (temp == "..")
		vec.pop_back();

	if (vec.size() == 0)
		return "/";
	for (string str : vec)
	{
		finalPath += "/" + str;
	}
	if (temp != ".." && temp != "." && temp.length() > 0)
		finalPath += "/" + temp;
	return finalPath;
}

bool Find(string Item, string Dir)
{
	if (chdir(Dir.c_str()) == -1)
	{
		return false;
	}
	char str_dir[1024];
	getcwd(str_dir, 1024);
	struct stat Status;
	mode_t Mode;
	DIR *dirptr = opendir(".");
	struct dirent *direntPtr = readdir(dirptr);
	while (direntPtr)
	{
		if (direntPtr->d_name[0] != '.')
		{
			stat(direntPtr->d_name, &Status);
			Mode = Status.st_mode;
			if (direntPtr->d_name == Item)
			{
				return true;
			}
			if (Mode && S_IFDIR)
			{
				string Inner_Dir = str_dir;
				Inner_Dir = Inner_Dir + "/";
				Inner_Dir = Inner_Dir + direntPtr->d_name;
				if (Find(Item, Inner_Dir))
				{
					return true;
				}
			}
		}
		direntPtr = readdir(dirptr);
	}

	return false;
}

void DeleteDir(string DelDirPath)
{
	cout << DelDirPath << endl;
	if (chdir(DelDirPath.c_str()) == -1)
	{
		cout << "Invalid Directory" << DelDirPath << endl;
		return;
	}

	char str_dir[1024];
	getcwd(str_dir, 1024);
	// cout << "cwd is " << str_dir << endl;
	struct stat Status;
	mode_t Mode;
	DIR *dirPtr = opendir(".");
	struct dirent *direntPtr = readdir(dirPtr);
	while (direntPtr)
	{
		if (direntPtr->d_name[0] != '.')
		{
			stat(direntPtr->d_name, &Status);
			Mode = Status.st_mode;

			if (Mode & S_IFDIR)
			{
				string DirOrFile = str_dir;
				DirOrFile = DirOrFile + "/";
				DirOrFile = DirOrFile + direntPtr->d_name;
				//	cout << DirOrFile << " is folder" << endl;
				DeleteDir(DirOrFile);
			}
			else
			{
				string DirOrFile = str_dir;
				DirOrFile = DirOrFile + "/";
				DirOrFile = DirOrFile + direntPtr->d_name;
				//	cout << DirOrFile << " is file" << endl;
				unlink(DirOrFile.c_str());
			}
		}
		direntPtr = readdir(dirPtr);
	}
	chdir("..");
	if (rmdir(DelDirPath.c_str()) == -1)
		perror("rmdir");
	return;
}

void CommandMode()
{

	CLEAR;
	string Org_Command = "";
	vector<string> vec_cmds;
	printModeMsg("Command Mode");
	cout << "Root= " << root << endl;
	cout << "User Home= " << userHome << endl;
	cout << endl;
	while (true)
	{

		char ch = cin.get();
		if (ch == 27)
		{ // esc character
			break;
		}

		switch (ch)
		{
		case 'q':
		{
			tcgetattr(STDIN_FILENO, &new_termios);
			tcsetattr(STDIN_FILENO, TCSAFLUSH, &org_termios);
			CLEAR;
			exit(1);
		}
		case 127:
		{
			if (Org_Command.size() > 0)
			{
				Org_Command.pop_back();
				printf("\033[%dD", (1));
				printf("\033[K");
			}
			break;
		}
		case 10:
		{
			int i = 0;
			for (; i < Org_Command.size(); i++)
			{
				if (Org_Command[i] != ' ')
					break;
			}
			string tempCmd = "";
			for (; i < Org_Command.size(); i++)
			{
				if (Org_Command[i] == ' ')
				{
					vec_cmds.push_back(tempCmd);
					tempCmd = "";
				}
				else
				{
					tempCmd += Org_Command[i];
				}
			}
			if (tempCmd != "")
			{
				vec_cmds.push_back(tempCmd);
			}
			Org_Command = "";

			if (vec_cmds.size() < 2 || vec_cmds[0] == "")
			{
				cout << endl
					 << "Invalid command" << endl;
				vec_cmds.clear();
				break;
			}
			else if (vec_cmds[0] == "copy")
			{
				if (vec_cmds.size() < 3)
				{
					cout << "Invalid command\n:- ";
					vec_cmds.clear();
					break;
				}
				string DestinationPath = vec_cmds[vec_cmds.size() - 1];
				string DestinationDirAbsPath = GetAbsPath(DestinationPath);
				string DestinationDir = NoramlizePath(DestinationDirAbsPath);
				vec_cmds.pop_back();
				for (int j = 1; j < vec_cmds.size(); j++)
				{
					string FileName = vec_cmds[j];
					ifstream source(FileName, ios::binary);
					ofstream dest(DestinationDir + "/" + FileName, ios::binary);
					dest << source.rdbuf();
					source.close();
					dest.close();
					cout << "Copying " << FileName << endl;
				}

			}
			else if (vec_cmds[0] == "move")
			{
				if (vec_cmds.size() < 3)
				{
					cout << "Invalid command\n:- ";
					vec_cmds.clear();
					break;
				}
				string DestinationPath = vec_cmds[vec_cmds.size() - 1];
				string DestinationDirAbsPath = GetAbsPath(DestinationPath);
				string DestinationDir = NoramlizePath(DestinationDirAbsPath);
				vec_cmds.pop_back();
				char str_dir[100];
				getcwd(str_dir, 100);
				for (int j = 1; j < vec_cmds.size(); j++)
				{
					string FileName = vec_cmds[j];

					ifstream source(FileName, ios::binary);
					ofstream dest(DestinationDir + "/" + FileName, ios::binary);

					dest << source.rdbuf();

					source.close();
					dest.close();
					// }
					string Del = string(str_dir) + "/" + FileName; // delete here
					// Del += "/" + str;
					unlink(Del.c_str());
					cout << "Moving " << FileName << endl;
				}
			}
			else if (vec_cmds[0] == "rename")
			{
				if (vec_cmds.size() != 3)
				{
					cout << "Invalid Command\n";
					vec_cmds.clear();
					break;
				}
				const char *prev_name, *new_name;
				prev_name = vec_cmds[1].c_str();
				new_name = vec_cmds[2].c_str();
				rename(prev_name, new_name);
				cout << "File Reanmed" << endl;
			}
			else if (vec_cmds[0] == "create_file")
			{
				if (vec_cmds.size() != 3)
				{
					cout << "invalid command\n";
					vec_cmds.clear();
					break;
				}
				string DestinationPath = vec_cmds[2];
				string DestinationDir;
				if (DestinationPath == ".")
				{
					char str_dir[1024];
					getcwd(str_dir, 1024);
					DestinationDir = str_dir;
				}
				else
				{
					DestinationDir = GetAbsPath(DestinationPath);
				}
				if (DestinationDir[DestinationDir.size() - 1] != '/')
				{
					DestinationDir += "/";
				}
				string FileName = vec_cmds[1];
				DestinationDir = DestinationDir + FileName;
				mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
				creat(DestinationDir.c_str(), mode);
				cout << "File Created" << endl;
			}
			else if (vec_cmds[0] == "create_dir")
			{
				if (vec_cmds.size() != 3)
				{
					cout << "invalid command\n";
					vec_cmds.clear();
					break;
				}
				string DestinationPath = vec_cmds[2];
				string DestinationDir;
				if (DestinationPath == ".")
				{
					char str_dir[1024];
					getcwd(str_dir, 1024);
					DestinationDir = str_dir;
				}
				else
				{
					DestinationDir = GetAbsPath(DestinationPath);
				}
				if (DestinationDir[DestinationDir.size() - 1] != '/')
				{
					DestinationDir += "/";
				}
				string DirName = vec_cmds[1];
				DestinationDir = DestinationDir + DirName;
				mode_t mode = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
				mkdir(DestinationDir.c_str(), mode);
				cout << "Directory Created" << endl;
			}
			else if (vec_cmds[0] == "delete_file")
			{
				if (vec_cmds.size() != 2)
				{
					cout << "invalid command\n";
					vec_cmds.clear();
					break;
				}
				string DestinationPath = vec_cmds[1];
				// string DestinationDir = GetAbsPath(DestinationPath);
				string DestinationDir = root + "/" + DestinationPath;
				cout << root << endl;
				if (unlink(DestinationDir.c_str()) != 0)
				{
					perror("unlink");
				}
				else
				{
					cout << "\nDeleted" << endl;
				}
			}
			else if (vec_cmds[0] == "delete_dir")
			{
				if (vec_cmds.size() != 2)
				{
					cout << "invalid command\n";
					vec_cmds.clear();
					break;
				}
				string DestinationPath = vec_cmds[1];
				string DestinationDir = root + DestinationPath;
				cout << "dir is" << DestinationDir << endl;
				DeleteDir(DestinationDir);
			}
			else if (vec_cmds[0] == "goto")
			{
				if (vec_cmds.size() != 2)
				{
					cout << "invalid command\n";
					vec_cmds.clear();
					break;
				}
				string DestinationPath = vec_cmds[1];
				string DestinationDir = root + "/" + DestinationPath;
				char str_dir[1024];
				getcwd(str_dir, 1024);
				chdir(DestinationDir.c_str());
				Push(back, str_dir);
			}
			else if (vec_cmds[0] == "search")
			{
				if (vec_cmds.size() != 2)
				{
					cout << "invalid command\n";
					vec_cmds.clear();
					break;
				}
				string SearchItem = vec_cmds[1];
				char str_dir[1024];
				getcwd(str_dir, 1024);
				if (Find(SearchItem, str_dir))
				{
					cout << "True" << endl;
				}
				else
				{
					cout << "False" << endl;
				}
				chdir(str_dir);
			}
			else
			{
				cout << "Invalid Command:" << endl;
				vec_cmds.clear();
				break;
			}
			vec_cmds.clear();
			break;
		}
		default:
		{
			Org_Command += ch;
			cout << ch; // printing character
		}
		}
	}
}

void InitialSettingsCommandMode()
{
	tcgetattr(STDIN_FILENO, &new_termios);
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &org_termios);

	struct termios cmd_termios;
	tcgetattr(STDIN_FILENO, &org_termios);
	new_termios = org_termios;
	new_termios.c_lflag &= (~ICANON);
	// new_termios.c_lflag &= (~ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
}

int main()
{

	struct passwd *pw = getpwuid(getuid());
	userHome = pw->pw_dir;
	win_row = tws.ws_row - 1;
	CLEAR;
	InitialSettingsOfNormalMode();
	while (true)
	{
		char ch = NormalMode();
		if (ch == ':')
		{
			// InitialSettingsCommandMode();
			CommandMode();
			CLEAR;
		}
		else if (ch == 'q')
		{
			break;
		}
	}
	tcgetattr(STDIN_FILENO, &new_termios);
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &org_termios);
	// string s;cin>>s;cout<<s<<endl;
	return 0;
}
