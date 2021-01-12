#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>

void ft_putchar_err(char c)
{
	write(STDERR_FILENO, &c, 1);
}

int error(char *str)
{
	while (*str)
		ft_putchar_err(*str++);
	return (1);
}

long fatal()
{
	error("error: fatal\n");
	return (0);
}

int size_cmd(char **cmd)
{
	if (!cmd)
		return (0);
	int i = -1;
	while (cmd[++i]);
	return (i);
}

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

char** add_cmd(char **av, int *i)
{
	char **tmp = NULL;
	int size = size_cmd_char(&av[*i], ";");

	if (!size)
	{
		*i += 1;
		return (NULL);
	}
	if (!(tmp = malloc(sizeof(*tmp) * (size + 1))))
		return ((char **)fatal());

	int j = -1;
	while (++j < size)
		tmp[j] = av[j + *i];
	tmp[j] = NULL;
	*i += size;
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
		return (error("\n"));
	}
	return (0);
}

int exec_cmd(char **cmd, char **env, char **free_ptr)
{
	pid_t pid;
	int status = 0;

	if ((pid = fork()) < 0)
		return (fatal());
	if (!pid) // son
	{
		if (execve(cmd[0], cmd, env) < 0)
		{
			error("error: cannot execute ");
			error(cmd[0]);
			free(free_ptr);
			exit(error("\n"));
		}
	}
	waitpid(pid, &status, 0);
	return (WEXITSTATUS(status));
}

int exec_son(char **cmd, char **env, char **tmp, int fd_in, int fd_pipe[2])
{
	if (dup2(fd_in, STDIN_FILENO) < 0)
		fatal();
	if (find_next_pipe(tmp) && dup2(fd_pipe[1], STDOUT_FILENO) < 0)
		fatal();

	close(fd_in);
	close(fd_pipe[0]);
	close(fd_pipe[1]);

	if (find_next_pipe(tmp))
		tmp[size_cmd_char(tmp, "|")] = NULL;

	int ret = exec_cmd(tmp, env, cmd);
	free(cmd);
	exit(ret);
}

int execute(char **cmd, char **env)
{
	if (!find_next_pipe(cmd))
		return (exec_cmd(cmd, env, cmd));

	int status = 0;
	int fd_in;
	int fd_pipe[2];
	char **tmp = cmd;
	int nb_wait = 0;
	pid_t pid;

	if ((fd_in = dup(STDIN_FILENO)) < 0)
		return (fatal());

	while (tmp)
	{
		if (pipe(fd_pipe) < 0)
			fatal();
		if ((pid = fork()) < 0)
			fatal();
		if (!pid)
			exec_son(cmd, env, tmp, fd_in, fd_pipe);
		else
		{
			if (dup2(fd_pipe[0], fd_in) < 0)
				fatal();
			close(fd_pipe[0]);
			close(fd_pipe[1]);
			++nb_wait;
		}
		tmp = find_next_pipe(tmp);
	}

	close(fd_in);
	while (nb_wait-- >= 0)
		if (waitpid(0, &status, 0) == pid)
			status = WEXITSTATUS(status);
	return (status);
}

int main(int ac, char **av, char **env)
{
	int ret = 0;
	char **cmd = NULL;
	int i = 1;

	while (i < ac)
	{
		cmd = add_cmd(av, &i);
		if (cmd && !strcmp(cmd[0], "cd"))
			ret = builtin_cd(cmd);
		else if (cmd)
			ret = execute(cmd, env);
		free(cmd);
		cmd = NULL;
	}
	return (ret);
}
