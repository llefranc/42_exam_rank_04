/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   microshell.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: llefranc <llefranc@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/01/14 11:35:56 by llefranc          #+#    #+#             */
/*   Updated: 2021/01/14 12:16:16 by llefranc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>

void ft_putchar_err(char c)
{
	write(STDERR_FILENO, &c, 1);
}

int error(char* str)
{
	while (*str)
		ft_putchar_err(*str++);
	return (1);
}

int fatal(char** free_ptr)
{
	free(free_ptr);
	exit(error("error: fatal\n"));
}

int size_cmd(char **cmd)
{
	if (!cmd)
		return (0);
	int i = -1;
	while (cmd[++i]);
	return (i);
}

// Return the size of **cmd until cmd[i] == str (useful for ";" and "|")
int size_cmd_char(char **cmd, char *str)
{
	if (!cmd)
		return (0);
	int i = -1;
	while (cmd[++i])
		if (!strcmp(cmd[i], str))
			return (i);
	return (i);
}

// Return a **char pointing just after the first pipe the func will meet
char** find_next_pipe(char **cmd)
{
	if (!cmd)
		return (NULL);
	int i = -1;
	while (cmd[++i])
		if (!strcmp(cmd[i], "|"))
			return (&cmd[i + 1]);
	return (NULL);
}

// Return a **char containing a copy of av[i] until next ";"
char** add_cmd(char **av, int *i)
{
	int size = size_cmd_char(&av[*i], ";"); // size of new **char until next ";". We start from i position
	if (!size)
		return (NULL); // case ";" ";" with nothing between them

	char **tmp = NULL;
	if (!(tmp = malloc(sizeof(*tmp) * (size + 1))))
		fatal(NULL);

	int j = -1;
	while (++j < size)
		tmp[j] = av[j + *i];
	tmp[j] = NULL;
	*i += size; // adds the number of elements copied, av[i] will be on the next ";"
	return (tmp);
}

int builtin_cd(char **cmd)
{
	if (size_cmd(cmd) != 2)
		return (error("error: cd: bad arguments\n"));
	if (chdir(cmd[1]) < 0)
	{
		error("error: cd: cannot change directory ");
		error(cmd[1]);
		error("\n");
	}
	return (0);
}

// Executes a command and free free_ptr in the forked process if an error occured
int exec_cmd(char **cmd, char **env, char **free_ptr)
{
	pid_t pid;

	if ((pid = fork()) < 0)
		fatal(free_ptr);
	if (!pid) // son
	{
		if (execve(cmd[0], cmd, env) < 0)
		{
			error("error: cannot execute ");
			error(cmd[0]);
			free(free_ptr); // Freeing the char** cmd previously allocated
			exit(error("\n"));
		}
	}
	waitpid(0, NULL, 0);
	return (0);
}

// Do the pipes and then execute only the part of the command before next pipe.
// free_ptr is char** cmd previously allocated. char** tmp isn't allocated,
// it's just a ptr to **cmd so no need to free it.
int exec_son(char** free_ptr, char** env, char** tmp, int fd_in, int fd_pipe[2])
{
	if (dup2(fd_in, STDIN_FILENO) < 0)
		fatal(free_ptr);
	if (find_next_pipe(tmp) && dup2(fd_pipe[1], STDOUT_FILENO) < 0) // If there is still a pipe after this command
		fatal(free_ptr);

	// Closing all fds to avoid leaking files descriptors
	close(fd_in);
	close(fd_pipe[0]);
	close(fd_pipe[1]);

	// Replaces first pipe met with NULL (modifying **cmd in the son, **cmd in
	// the parent is still the same!) then executing the command
	tmp[size_cmd_char(tmp, "|")] = NULL;
	exec_cmd(tmp, env, free_ptr);
	
	// Freeing char** cmd in the fork process (still exists in the parent!)
	free(free_ptr);
	exit(0);
}

int execute(char **cmd, char **env)
{
	/* CASE NO PIPES */
	if (!find_next_pipe(cmd))
		return (exec_cmd(cmd, env, cmd));

	/* CASE PIPES */
	int fd_in;
	int fd_pipe[2];
	char **tmp = cmd;
	int nb_wait = 0;
	pid_t pid;

	if ((fd_in = dup(STDIN_FILENO)) < 0)
		return (fatal(cmd));

	while (tmp)
	{
		if (pipe(fd_pipe) < 0 || (pid = fork()) < 0)
			fatal(cmd);
			
		// Son is executing commands
		if (!pid)
			exec_son(cmd, env, tmp, fd_in, fd_pipe);
		
		// Parent is just saving fd_pipe[0] for next son execution and correctly closing pipes
		else
		{
			if (dup2(fd_pipe[0], fd_in) < 0)	// Really important to protect syscalls using fd,
				fatal(cmd);						// tests with wrong fds will be done during grademe
			close(fd_pipe[0]);
			close(fd_pipe[1]);
			++nb_wait;
			tmp = find_next_pipe(tmp); // Goes to the next command to be executed, just after first pipe met
		}
	}

	//closing last dup2 that happen in the last parent loop tour
	close(fd_in); 

	//waiting for each command launched to bed executed
	while (nb_wait-- >= 0)
		waitpid(0, NULL, 0);
	return (0);
}

int main(int ac, char **av, char **env)
{
	char **cmd = NULL;
	int i = 1;

	while (i < ac)
	{
		// cmd = command until next ";". i is increased of the size of cmd,
		// av[i] will now be equal to next ";"
		cmd = add_cmd(av, &i);
		
		if (cmd && !strcmp(cmd[0], "cd"))
			builtin_cd(cmd);
		else if (cmd)
			execute(cmd, env);

		free(cmd);
		cmd = NULL;
	}
	return (0);
}
