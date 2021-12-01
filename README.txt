
% Precliam: This code take partial reference from Youtube, "UDP Programming in C"
% following:  https://www.youtube.com/watch?v=Emuw71lozdA

To run the code in either /Assignemnt1 or /Assignemnt2, please execute the following commands:

```shell
gcc file_name.c -o compiled_name
./compiled_name
```

for example, `gcc server.c -o server` and `./server`, then the server will run.

* IMPORTANT: You must run the server first! 

-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
                            Assignemnt 1
-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
To test the different cases, you may have to manually modifiy the text file read by the program.
There are two text files, "normal_case.txt" and "abnormal_case.txt".
% Assignemnt1/client.c line 41

1. "normal_case.txt" contains the normal situation cases that are the standard inputs
2. "abnormal_case.txt" contains the abnormal situation cases that may let the server reject these illegal messages
3. Timeout can be tested if you run the `client` only, as the server stays idle, the timeout cases will happen

Testing Process:
1. Go to the coresponding directory, for this case, it should be `COEN233/Assignemnt1`
2. Run `gcc server.c -o server` and `./server` to initialize the server
3. Make sure you have identify which scenarios you want to test, by manually revising the text file
4. Save the file (cmd + s for MacOS, ctrl + s for Windows/Linux) and run `gcc client.c -o client` and `./client` in the terminal
5. Verify the result



-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
                            Assignemnt 2
-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
Similar to Assignment 1, Assignment 2 is built based on the framewrok of Assignemnt1
There are two text files as well:
- `requests.txt` contains the request (valid and invalid) that are ready to be sent
- `verification_database.txt` contains what database contains

The testing proceduers are basicaaly the same as claimed in Assignemnt1 sections

Testing Process:
1. Go to the coresponding directory, for this case, it should be `COEN233/Assignemnt2`
2. Test timeout case, directly compile `client.c` and run `clinet`
2. Test other (valid and invalid packet cases), initialize the `server` first, then run the client
