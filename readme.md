Type the following command in terminal to compile the code: `gcc shell.c -lreadline`
Type the following command to see the output : `./a.out`

The expected format for the user input commands should be as follows:
	Normal command execution        :  cmd arg1 arg2 ... argn
	Start command in background     :  &cmd arg1 arg2 ... argn
	Piped command                   :  cmd1 arg1 ... argn | cmd2 arg1 ... argn
	Set environment variable        :  identifier = value
	
		where,
			cmd = cmd_history or ps_history or alphanumeric string
			arg_i = alphanumeric string or $identifier
			identifier , value = alphanumeric string
			alphanumeric string = one or more occurrences of elements from {A ,B ,C , D .... , Z ,a ,b ,c , d .... , z ,1 ,2 ,3 ,4...9 , - , _ }
