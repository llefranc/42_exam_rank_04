# CPP_07 (@42Paris)

## About

Third exam of the new 42 cursus. You will have to recode a small shell (cd, execve and pipes).

- Here is the [subject][1]

`This project was code for MACOS`

### Building and running the project

1. Download/Clone this repo

        git clone https://github.com/lucaslefrancq/42_exam_rank_4.git

2. `cd` into the root directory then run `make`.

        cd 42_exam_rank_4
        make

3.  You can test your own microshell with some tests in the `some_tests` file and compare your results with bash.
	Don't forget to pass comas and pipes between quotes.
	It's really important to protect your syscalls with the `fatal` function (especially syscalls implicating 
	file descriptors like dup, pipe... They will test your microshell with wrong fds).
	Checks also if you're not leaking file descriptors with `lsof -c microshell` (put an infinite loop at the
	end of your main when testing this). You should have only stdin, stdout and stderr open (respectively 1u, 2u
	and 3u). Tests `/bin/cat "|" /bin/ls` and `/bin/cat /dev/urandom "|" /usr/bin/head -c 10` should work like
	in bash. Looks SIGPIPE signal for more information.
    
## Sources

- [Some examples for lsof][2]

[1]: https://github.com/lucaslefrancq/42_CPP_piscine/blob/main/CPP07/cpp07.en.subject.pdf
[2]: https://www.thegeekstuff.com/2012/08/lsof-command-examples/