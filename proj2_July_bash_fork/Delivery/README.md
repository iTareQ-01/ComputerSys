Server-Client App to exec shell in "C Language"

1- How to run:
	Run the following command in the directory to compile
	make all - the Makefile will generate a server and client executable
	Open two separate terminals and run server_conf.sh in the first open
	Please, make sure to change the **USERNAME** you login with inside the **client_conf.sh**
	Once the server is running, run client_conf.sh in the other Once
	Play around and enjoy!
	
2- Features:
	A remote shell used for simulate core concepts of OS system calls
	server_socket_bash runs a process which listens on **localhost:4444**
	client_server_bash runs a process to look for that socket address
	If the server is there and accepting, create a new socket connection
	The server create a child process for each client needs to connect
	Connection stays open indefinitely, until the client inputs q
	The client reads input line by line from stdin
	The client sends this data to the server, the server handles the processing
	The server makes a pipe system call to create a child process and execute the input
	The **exec System call** is looking to execute a bash command and store it in variable result
	Finally, the server will send back the contents of the output to the client
	The client renders the output, and continues to sit on the connection
	You can exit the parent process which run the server and kill all the child processes using **commandline quit**
