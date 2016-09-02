// Team: Morgan Eckenroth


#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <sys/types.h>

#include "wrappers.h"
#include "dlist.h"
#include "dnode.h"

#define INPUT_SIZE 512
#define HISTORY_SIZE 20

char **tokenize(char *input, char *delimiter, char *op_arg);
char *get_input(char *input);
int get_cursor_pos(int *col, int *row);
int set_cursor_pos(int col, int row);
int add_to_history(char *in);
int print_history();
int pipe_funcs(char **args);
int nonpipe_funcs(char **args);

int on_whitespace = 1;
int curr_word_pos = 0;
int tab_state = 0;
char *saved_in;
struct dlist history;

int main(int argc, char **argv)
{
	saved_in = malloc(INPUT_SIZE);
	saved_in[0] = 0;
	char input[INPUT_SIZE];
	char **token_args;
	char **arg_sets;
	while(1)
	{
		printf("gshell> ");
		fflush(stdout);
		get_input(input);
		if(strlen(input) > 0)
		{
			add_to_history(input);
		}
		int i = strlen(input)-1;
		if(i>0 && (input[i] == '\n')) input[i] = 0;
		arg_sets = tokenize(input, ";", NULL);
		int num_args = 0;
		char *curr = NULL;
		while((curr = arg_sets[num_args]) != 0) num_args++;
		
		char **pipe_args;
		for(i = 0; i < num_args; i++)
		{
			int ret = 0;
			int count = 0;
			int count2 = 0;
			while(arg_sets[count2]) count2++;

			pipe_args = tokenize(arg_sets[i], "|", NULL);
			
			while(pipe_args[count]) count++;

			if(count != count2)
			{
				ret = pipe_funcs(pipe_args);
			}
			else
			{
				token_args = tokenize(arg_sets[i], " ", NULL);
				
				// Exit program
				if(strcmp(token_args[0], "exit") == 0 || strcmp(token_args[0], "Exit") == 0)
				{
					exit(0);
				}

				ret = nonpipe_funcs(token_args);
			}
			
			if(WEXITSTATUS(ret) != 0)
			{
				printf("[gshell: program terminated abnormally][%d]\n", WEXITSTATUS(ret));
			}
			else
			{
				printf("[gshell: program terminated successfully]\n");
			}
	
		}
	}
	return 1;
}

char **tokenize(char *input, char *delimiter, char *op_arg)
{
	int elems = 0;
	char **tokens = NULL;
	char *token = strtok(input, delimiter);
	
	char *str_block = malloc(sizeof(char *));

	int str_state = 0;

	while(token != NULL)
	{
		if(str_state != 1)
		{
			tokens = realloc(tokens, sizeof (char *) *++elems);
		}
		if(tokens == NULL) perror("Tokenize failed!\n");

		if(token[0] == '\"')
		{
			str_state = 1;
		}
		int test_len = strlen(token)-1;
		if(token[test_len] == '\"')
		{
			str_state = 0;
			strcat(str_block, token);
			token = str_block;
			str_block = "";
		}

		if(str_state == 0)
		{
			tokens[elems-1] = token;
		}
		else  // str_state == 1
		{
			strcat(str_block, token);
			strcat(str_block, " ");
		}
		token = strtok(NULL, delimiter);
	}
	if(op_arg != NULL)
	{
		tokens = realloc(tokens, sizeof(char *) *(elems+2));
		tokens[elems] = op_arg;
		tokens[elems+1] = 0;
	}
	else
	{
		tokens = realloc(tokens, sizeof (char *) *(elems+1));
		tokens[elems] = 0;
	}
	return tokens;
}

char *get_input(char *input)
{
	memset(input, 0, INPUT_SIZE);
	int tt = 0;
	int hist_pos = 0;
	struct dnode *h_node = NULL;
	char buf = 0;
	int length = 0;
	int cursor_x = 0;
	int cursor_x_max = 0;
	struct termios old = {0};
	if(saved_in[0] != 0)
	{
		printf("%s", saved_in);
		strcpy(input, saved_in);
		int len = strlen(saved_in);
		cursor_x = len;
		cursor_x_max = len;
		length = len;
		saved_in[0] = 0;
	}
	

	while(buf != '\n')
	{
		buf = -1;
		if (tcgetattr(0, &old) < 0)
		      perror("tcsetattr()");
		old.c_lflag &= ~ICANON;
		old.c_lflag &= ~ECHO;
		old.c_cc[VMIN] = 1;
		old.c_cc[VTIME] = 0;
		if (tcsetattr(0, TCSANOW, &old) < 0)
		      perror("tcsetattr ICANON");
		
		buf = getchar();

		old.c_lflag |= ICANON;
		old.c_lflag |= ECHO;
		if (tcsetattr(0, TCSADRAIN, &old) < 0)
		      perror ("tcsetattr ~ICANON");
		
		if(buf == 27) // Escape
		{
			tab_state = 0;
			buf = getchar();
			if(buf == 91) // [
			{
				char a_ch = getchar();

				// Up Arrow
				if(a_ch == 65)
				{
					if(history.back != NULL)
					{
						if(hist_pos == 0)
						{
							h_node = history.back;
						}
						else if(h_node->prev != NULL)
						{
							h_node = h_node->prev;
						}

						if(h_node != NULL)
						{
							int i; 
							if(strncmp(input, "ls", 2) == 0)
							{
								for(i = 0; i < 2; i++)
								{
									printf("\b \b");
								}
							}
							else
							{
								for(i = 0; i < length; i++)
								{
									printf("\b \b");
								}
							}
							memset(input, 0, length);
							memcpy(input, h_node->data, strlen(h_node->data));
							if(strncmp(input, "ls", 2) == 0)
							{
								printf("ls");
							}
							else
							{
								printf("%s", input);
							}
							cursor_x = cursor_x_max = length = strlen(h_node->data);
							hist_pos++;
						}
					}
				}
				// Down Arrow
				if(a_ch == 66)
				{
					if(hist_pos != 0 && h_node->next != NULL)
					{
						h_node = h_node->next;
						
						int i;	
						if(strncmp(input, "ls", 2) == 0)
						{
							for(i = 0; i < 2; i++)
							{
								printf("\b \b");
							}
						}
						else
						{
							for(i = 0; i < length; i++)
							{
								printf("\b \b");
							}
						}
						memset(input, 0, length);
						memcpy(input, h_node->data, strlen(h_node->data));
						if(strncmp(input, "ls!", 2) == 0)
						{
							printf("ls");
						}
						else
						{
							printf("%s", input);
						}
						cursor_x = cursor_x_max = length = strlen(h_node->data);
						hist_pos--;
					}
				}
				// Right Arrow
				if(a_ch == 67)
				{
					if(cursor_x < cursor_x_max)
					{
						cursor_x++;
						printf("%c", 27);
						printf("%c", 91);
						printf("%c", 67);
					}
				}
				// Left Arrow
				if(a_ch == 68)
				{
					if(cursor_x > 0)
					{
						cursor_x--;
						printf("%c", 27);
						printf("%c", 91);
						printf("%c", 68);
					}
				}
			}
		}
		else if(buf == 127 || buf == 8) // Backspace
		{
			tab_state = 0;
			if(cursor_x > 0)
			{
				// Copy subsection of input into new
				// input and then print it plus a
				// space character to stdout,
				// simulating shifting characters
				char new_out[length];
				int curs_col, curs_row;
				memcpy(new_out, &input[cursor_x], (length - cursor_x)*sizeof(char));
				new_out[length-cursor_x] = ' ';
				new_out[length-cursor_x+1] = 0;
				
				/* Shift input array down one */
				int i;
				for(i = cursor_x-1; i < length; i++)
				{
					input[i] = input[i+1];
				}

				printf("\b \b");
				printf("%s", new_out);
				get_cursor_pos(&curs_col, &curs_row);
				set_cursor_pos(strlen("gshell> ")+cursor_x, curs_row);
				length--;
				cursor_x--;
			}
			if(cursor_x_max > 0 && (cursor_x+1) != 0)
			{
				cursor_x_max--;
			}
		}
		else if(buf == 9) // Tab
		{
			tab_state++;
			if(on_whitespace == 0)
			{
				char *temp = input+curr_word_pos;
				DIR *d;
				struct dirent *dir;
				d = opendir(".");
				if(d)
				{
					while((dir = readdir(d)) != NULL)
					{
						if(strncmp(temp, dir->d_name, strlen(temp)) == 0 &&
							strlen(temp) != strlen(dir->d_name))
						{
							input[curr_word_pos] = 0;
							strcat(input, dir->d_name);
							strcat(input, " ");
							cursor_x = strlen(input);
							cursor_x_max = cursor_x;
							length = cursor_x;
							on_whitespace = 1;
							printf("%s", (input+curr_word_pos+1));
							tab_state = 0;
							break;
						}
					}
					closedir(d);
				}
			}
			if(tab_state > 1)
			{
				tt = 1;
				strcpy(saved_in, input);
				strcpy(input, "ls");
				break;
			}
		}
		else if(buf >= 32 && buf <= 126) // Visible characters
		{
			if(buf == ' ') 
			{
				on_whitespace = 1;
			}
			else if(on_whitespace != 0)
			{
				on_whitespace = 0;
				curr_word_pos = cursor_x;
			}
			tab_state = 0;
			// Take in char from stdin and insert where cursor is,
			// shifting other input to the right.
			if(cursor_x < cursor_x_max)
			{
				char new_out[length];
				int curs_col, curs_row;
				memcpy(new_out, &input[cursor_x], (length - cursor_x) * sizeof(char));
				new_out[length-cursor_x] = 0;
				printf("%c", buf);

				if(length > 0)
				{
					printf("%s", new_out);
				}

				get_cursor_pos(&curs_col, &curs_row);
				set_cursor_pos(strlen("gshell> ")+cursor_x+2, curs_row);

				/* Shift current input array down one to insert new char */
				int i;
				for(i = length; i > cursor_x; i--)
				{
					input[i] = input[i-1];
				}
			}
			else
			{
				printf("%c", buf);
			}
			input[cursor_x] = buf;

			cursor_x_max++;
			cursor_x++;
			length++;
		}
		fflush(stdout);
	}
	
	curr_word_pos = 0;
	if(tt == 0)
	{
		on_whitespace = 1;
	}
	printf("\n");
	return input;
}

int get_cursor_pos(int *col, int *row)
{
	struct termios old = {0};
	struct termios save = {0};
	tcgetattr(0, &save);
	if (tcgetattr(0, &old) < 0)
	      perror("tcsetattr()");
	old.c_lflag &= ~ICANON;
	old.c_lflag &= ~ECHO;
	old.c_lflag &= ~CREAD;
	old.c_cc[VMIN] = 1;
	old.c_cc[VTIME] = 0;
	if (tcsetattr(0, TCSANOW, &old) < 0)
	      perror("tcsetattr ICANON");
	
	/* Print cursor pos get ANSI string */
	printf("\033[6n");

	/* Parse out row;col string from return */
	char buf = getchar();
	if(buf == 27)  // ESC
	{
		buf = getchar();
		if(buf == '[')
		{
			*col = 0;
			*row = 0;
			char res = getchar();
			/* Parse row */
			while(res >= '0' && res <= '9')
			{
				*row = 10 * *row + res - '0';
				res = getchar();
			}

			if(res == ';')
			{
				res = getchar();
				/* Parse col */
				while(res >= '0' && res <= '9')
				{
					*col = 10 * *col + res - '0';
					res = getchar();
				}
			}
		}
	}

	tcsetattr(0, TCSANOW, &save);	
	return 0;
}

int set_cursor_pos(int col, int row)
{
	printf("\033[%d;%dH", row, col);
	return 0;
}

int add_to_history(char *in)
{
	dlist_add_back(&history, in);
	return 0;
}

int print_history()
{
	int i = 1;
	struct dnode *curr = history.front;
	while(curr != NULL)
	{
		if(strncmp(curr->data, "ls", 2) == 0)
		{
			printf("%d  %s\n", i, "ls");
		}
		else
		{
			printf("%d  %s\n", i, curr->data);
		}
		i++;
		curr = curr->next;
	}
	return 0;
}

int nonpipe_funcs(char **args)
{
	
	pid_t pid = Fork();

	// Child
	if(pid == 0)
	{
		// Print history
		if(strcmp(args[0], "history") == 0)
		{
			int ret = print_history();
			return ret;
		}
		
		// Change directory
		else if(strcmp(args[0], "cd") == 0)
		{
			int ret = chdir(args[1]);
			return ret;
		}
		else
		{
			execvp(args[0], args);
		}
	}
	else // Parent
	{
		int status = 0;
		waitpid(pid, &status, 0);
		return status;
	}
	return -1;
}

int pipe_funcs(char **args)
{
	int filedes[2];
	Pipe(filedes);

	pid_t pid = Fork();

	// Child
	if(pid == 0)
	{
		Close(filedes[0]); // Does not read
		char **targs = tokenize(args[0], " ", NULL);
		while(dup2(filedes[1],STDOUT_FILENO) == -1) {}
		Close(filedes[1]);
		
		// Print history
		if(strcmp(args[0], "history") == 0)
		{
			int ret = print_history();
			return ret;
		}
		
		// Change directory
		else if(strcmp(args[0], "cd") == 0)
		{
			int ret = chdir(args[1]);
			return ret;
		}
		else
		{
			execvp(targs[0], targs);
		}
	}
	else  // Parent
	{
		Close(filedes[1]); // Does not write
		int status;
		waitpid(pid, &status , 0);
		if(status == 0)
		{
			pid_t pid2 = Fork();

			if(pid2 == 0) // Child
			{
				while(dup2(filedes[0], STDIN_FILENO) == -1){}
				Close(filedes[0]);
				char **targs = tokenize(args[1], " ", NULL);
		
				// Print history
				if(strcmp(args[0], "history") == 0)		
				{
					int ret = print_history();
					return ret;
				}
				
				// Change directory
				else if(strcmp(args[0], "cd") == 0)
				{
					int ret = chdir(args[1]);
					return ret;
				}
				else
				{
					execvp(targs[0], targs);
				}
			}
			else // Parent
			{
				waitpid(pid2, &status, 0);
				return status;
			}
		}
		return status;
	}
	

	return -1;
}
