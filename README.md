Developer: Rosemary Espinal

Build/Test/Run Environment: 
 - CentOS 7 Virtual Box VM running on MacOS

How to compile my shell:

gcc myshellapp.c -o myshellapp

NOTES: 
- When running the 'cd' command you must type in the full path that you would like to go to. Auto path completion using TAB doesn't work.
- Pipe functionality is causing a segmentation fault. Needs to be fixed in resubmission.
- After running a process in the background, when issuing another command in the shell, the output of that command comes out inline with the shell prompt followed by moving the cursor to a new line. You can still type in other commands and get the output without seeing the shell prompt right away.