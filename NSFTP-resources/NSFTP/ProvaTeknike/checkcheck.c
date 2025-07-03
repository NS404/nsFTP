//#include <iostream>
//#include <string>
//using namespace std; 
//#include <algorithm>

#include <stdio.h>
#define MAXLINE 1000

int getline(char line[], int maxline);
void copy(char to[], char from[]);

int main() {

	int len;				//current line length
	int max;				// maximum length seen so far
	char line[MAXLINE];		//current input line
	char longest[MAXLINE];  //longest line saved here


	max = 0;
	while ((len = getline(line, MAXLINE)) > 0)
		if (len > max) {
			max = len;
			copy(longest, line);
		}
	if (max > 0)
		printf("%s\n", longest);
	printf("length: %d", max);
	return 0;
	
	/*string command;
	do {
		cout << "FTP > ";
		getline(cin, command);
		transform(command.begin(), command.end(), command.begin(), ::toupper);
		cout << command << "\n";
	} while (command != "quit");*/

}

// getline: read a line into s, return length
int getline(char s[], int lim)
{
	int c, i;

	for (i = 0; i < lim - 1 && (c = getchar()) != EOF && c != '\n'; ++i)
		s[i] = c;
	/*if (c == '\n') {
		s[i] = c;
		++i;
	}*/
	s[i] = '\0';
	return i;
}


// copy: copy 'from' into 'to' ; assume to is big enough
void copy(char to[], char from[])
{
	int i;

	i = 0;
	while ((to[i] = from[i]) != '\0')
		++i;
}