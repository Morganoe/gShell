#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <sys/ioctl.h>

#include "wrappers.h"
#include "dlist.h"
#include "dnode.h"

#define INPUT_SIZE 512
#define HISTORY_SIZE 20

char **tokenize(char *input, char *delimiter);
char *get_input(char *input);
int get_cursor_pos(int *col, int *row);
int set_cursor_pos(int col, int row);
int add_to_history(char *in);
int print_history();

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
		printf("ishell> ");
		fflush(stdout);
		get_input(input);
		if(strlen(input) > 0)
		{
			add_to_history(input);
		}
		int i = strlen(input)-1;
		if(i>0 && (input[i] == '\n')) input[i] = 0;
		
		arg_sets = tokenize(input, ";");
		int num_args = 0;
		char *curr = NULL;
		while((curr = arg_sets[num_args]) != 0) num_args++;

		for(i = 0; i < num_args; i++)
		{
			token_args = tokenize(arg_sets[i], " ");
			
			if(strcmp(token_args[0], "exit") == 0 ||
			   strcmp(token_args[0], "Exit") == 0)
			{
				return 0;
			}

			if(strcmp(token_args[0], "history") == 0)
			{
				int ret = print_history();
				if(ret == 0)
				{
					printf("[ishell: program terminated successfully]\n");
				}
				else
				{
					printf("[ishell: program terminated abnormally][%d]\n", ret);
				}
			}
			else
			{
				pid_t pid = Fork();
	
				// Parent
				if(pid > 0)
				{
					int status = 0;
					waitpid(pid, &status, 0);
					if(WEXITSTATUS(status) != 0)
					{
						printf("[ishell: program terminated abnormally][%d]\n", WEXITSTATUS(status));
					}
					else
					{
						printf("[ishell: program terminated successfully]\n");
					}
	
				}
				else  // Child
				{
					if(strcmp(token_args[0], "cd") != 0)
					{
						execvp(token_args[0], token_args);
					}
					else
					{
						int ret = chdir(token_args[1]);
						if(ret == 0)
						{
							printf("[ishell: program terminated successfully]\n");
						}
						else
						{
							printf("[ishell: program terminated abnormally][%d]\n", ret);
						}
					}
				}
			}
		}
	}
	return 1;
		
}

char **tokenize(char *input, char *delimiter)
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
	tokens = realloc(tokens, sizeof (char *) *(elems+1));
	tokens[elems] = 0;
	return tokens;
}

char *get_input(char *input)
{
	memset(input, 0, INPUT_SIZE);
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
							for(i = 0; i < length; i++)
							{
								printf("\b \b");
							}

							memset(input, 0, length);
							memcpy(input, h_node->data, strlen(h_node->data));
							printf("%s", input);
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
						for(i = 0; i < length; i++)
						{
							printf("\b \b");
						}

						memset(input, 0, length);
						memcpy(input, h_node->data, strlen(h_node->data));
						printf("%s", input);
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
				set_cursor_pos(strlen("ishell> ")+cursor_x, curs_row);
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
			if(tab_state == 1)
			{
				strcpy(saved_in, input);
				strcpy(input, "ls");
				break;
			}
			tab_state = 1;
		}
		else if(buf >= 32 && buf <= 126) // Visible characters
		{
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
				set_cursor_pos(strlen("ishell> ")+cursor_x+2, curs_row);

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
		printf("%d  %s\n", i, curr->data);
		i++;
		curr = curr->next;
	}
	return 0;
}
