INVOKING gdb:
  gdb --args  <COMMAND_LINE>
  Example:  gdb --args ./a.out

COMMON gdb COMMANDS:
  break, continue
  run
  next, step, finish  (next line, step inside fnc, finish current fnc)
  where, frame, list (e.g.: where; frame 0; list)
  ptype, print  (   ptype argv[0]; print argv[0]   )
  quit
  < Cursor keys work, TAB provides auto-complete >
  IF PROGRAM IS IN INFINITE LOOP, TYPE ^C (ctrl-C) TO TALK TO gdb.
  help    e.g.: (gdb) help continue
  # For visual display of code, use:  ^Xa  (but switch back after that.  it's buggy.)

SPECIAL gdb COMMAND (for 'fork' system call':
  set follow-fork-mode child  [By default, gdb follows parent after call to fork;
                               This changes gdb to follow the child instead.]

Typical gdb session to understand better the functions getargs() and getword():

[ NOTICE THAT gdb WILL REQUIRE YOU TO GIVE THE 'run' COMMAND. ]
MSPACMAN:hw2% gdb --args ./myshell
GNU gdb (GDB) 7.0-ubuntu
Copyright (C) 2009 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.  Type "show copying"
and "show warranty" for details.
This GDB was configured as "i486-linux-gnu".
For bug reporting instructions, please see:
<http://www.gnu.org/software/gdb/bugs/>...
Reading symbols from /net/course/cs2600gc/hw2/myshell...done.
(gdb) where
No stack.
(gdb) next
The program is not being run.
(gdb) run
Starting program: /net/course/cs2600gc/hw2/myshell 
% exit
Program exited normally.
(gdb) quit


[ THE 'run' COMMAND NEVER STOPPED.  THIS TIME, WE WILL SET
  A BREAKPOINT AT 'main' FIRST. ]
MSPACMAN:hw2% gdb ./myshell
GNU gdb (GDB) 7.0-ubuntu
Copyright (C) 2009 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.  Type "show copying"
and "show warranty" for details.
This GDB was configured as "i486-linux-gnu".
For bug reporting instructions, please see:
<http://www.gnu.org/software/gdb/bugs/>...
Reading symbols from /net/course/cs2600gc/hw2/myshell...done.
(gdb) break main
Breakpoint 1 at 0x80488a0: file hw2.c, line 101.
(gdb) run
Starting program: /net/course/cs2600gc/hw2/myshell 

Breakpoint 1, main (argc=1, argv=0xbffff8b4) at hw2.c:101
101	{
(gdb) list
96	        waitpid(childpid, NULL, 0);  /* wait until child process finishes */
97	    return;
98	}
99	
100	int main(int argc, char *argv[])
101	{
102	    char cmd[MAXLINE];
103	    char *childargv[MAXARGS];
104	    int childargc;
105	
(gdb) list
106	    while (1) {
107	        printf("%% "); /* printf uses %d, %s, %x, etc.  See 'man 3 printf' */
108	        fflush(stdout); /* flush from output buffer to terminal itself */
109		getargs(cmd, &childargc, childargv); /* childargc and childargv are
110	            output args; on input they have garbage, but getargs sets them. */
111	        /* Check first for built-in commands. */
112		if ( childargc > 0 && strcmp(childargv[0], "exit") == 0 )
113	            exit(0);
114		else if ( childargc > 0 && strcmp(childargv[0], "logout") == 0 )
115	            exit(0);
(gdb) next
107	        printf("%% "); /* printf uses %d, %s, %x, etc.  See 'man 3 printf' */
(gdb) next
108	        fflush(stdout); /* flush from output buffer to terminal itself */
109		getargs(cmd, &childargc, childargv); /* childargc and childargv are
110	            output args; on input they have garbage, but getargs sets them. */
111             /* Check first for built-in commands. */
112             if ( childargc > 0 && strcmp(childargv[0], "exit") == 0 )
113                 exit(0);
114             else if ( childargc > 0 && strcmp(childargv[0], "logout") == 0 )
115                 exit(0);
(gdb) next
107             printf("%% "); /* printf uses %d, %s, %x, etc.  See 'man 3 printf' */
(gdb) list
102         char cmd[MAXLINE];
103         char *childargv[MAXARGS];
104         int childargc;
105
106         while (1) {
107             printf("%% "); /* printf uses %d, %s, %x, etc.  See 'man 3 printf' */
108             fflush(stdout); /* flush from output buffer to terminal itself */
109             getargs(cmd, &childargc, childargv); /* childargc and childargv are
110                 output args; on input they have garbage, but getargs sets them. */
111             /* Check first for built-in commands. */
(gdb) next
108             fflush(stdout); /* flush from output buffer to terminal itself */
(gdb) next
% 109           getargs(cmd, &childargc, childargv); /* childargc and childargv are
(gdb) step
getargs (cmd=0xbffff9b4 "\314\371\377\277\016", argcp=0xbffff9b0, 
    argv=0xbffff960) at hw2.c:55
55          char *cmdp = cmd;
(gdb) where
#0  getargs (cmd=0xbffff9b4 "\314\371\377\277\016", argcp=0xbffff9b0, 
    argv=0xbffff960) at hw2.c:55
#1  0x080488e5 in main (argc=1, argv=0xbffffb34) at hw2.c:109
(gdb) list
50       * non-zero number (traditionally "1") means true.
51       */
52      /* argc is _count_ of args (*argcp == argc); argv is array of arg _values_*/
53      static void getargs(char cmd[], int *argcp, char *argv[])
54      {
55          char *cmdp = cmd;
56          char *end;
57          int i = 0;
58
59          /* fgets creates null-terminated string. stdin is pre-defined C constant
(gdb) frame 1
#1  0x080488e5 in main (argc=1, argv=0xbffffb34) at hw2.c:109
109             getargs(cmd, &childargc, childargv); /* childargc and childargv are
(gdb) list
104         int childargc;
105
106         while (1) {
107             printf("%% "); /* printf uses %d, %s, %x, etc.  See 'man 3 printf' */
108             fflush(stdout); /* flush from output buffer to terminal itself */
109             getargs(cmd, &childargc, childargv); /* childargc and childargv are
110                 output args; on input they have garbage, but getargs sets them. */
111             /* Check first for built-in commands. */
112             if ( childargc > 0 && strcmp(childargv[0], "exit") == 0 )
113                 exit(0);
(gdb) frame 0
#0  getargs (cmd=0xbffff9b4 "\314\371\377\277\016", argcp=0xbffff9b0, 
    argv=0xbffff960) at hw2.c:55
55          char *cmdp = cmd;
(gdb) list
50       * non-zero number (traditionally "1") means true.
51       */
52      /* argc is _count_ of args (*argcp == argc); argv is array of arg _values_*/
53      static void getargs(char cmd[], int *argcp, char *argv[])
54      {
55          char *cmdp = cmd;
56          char *end;
57          int i = 0;
58
59          /* fgets creates null-terminated string. stdin is pre-defined C constant
(gdb) print cmd
$1 = 0xbffff9b4 "\314\371\377\277\016"
(gdb) p cmd
$2 = 0xbffff9b4 "\314\371\377\277\016"
(gdb) ptype cmd
type = char *
(gdb) p argcp
$3 = (int *) 0xbffff9b0
(gdb) p * argcp
$4 = 1073841096
(gdb) p argv
$5 = (char **) 0xbffff960
(gdb) p argv[0]
$6 = 0x8048400 "GLIBC_2.0"
(gdb) p argv[1]
$7 = 0x0
(gdb) list
60           *   for standard intput.  feof(stdin) tests for file:end-of-file.
61           */
62          if (fgets(cmd, MAXLINE, stdin) == NULL && feof(stdin)) {
63              printf("Couldn't read from standard input. End of file? Exiting ...\n");
64              exit(1);  /* any non-zero value for exit means failure. */
65          }
66          while ( (cmdp = getword(cmdp, &end)) != NULL ) { /* end is output param */
67              /* getword converts word into null-terminated string */
68              argv[i++] = cmdp;
69              /* "end" brings us only to the '\0' at end of string */
(gdb) where
#0  getargs (cmd=0xbffff9b4 "\314\371\377\277\016", argcp=0xbffff9b0, 
    argv=0xbffff960) at hw2.c:55
#1  0x080488e5 in main (argc=1, argv=0xbffffb34) at hw2.c:109
(gdb) frame 0
#0  getargs (cmd=0xbffff9b4 "\314\371\377\277\016", argcp=0xbffff9b0, 
    argv=0xbffff960) at hw2.c:55
55          char *cmdp = cmd;
(gdb) list
50       * non-zero number (traditionally "1") means true.
51       */
52      /* argc is _count_ of args (*argcp == argc); argv is array of arg _values_*/
53      static void getargs(char cmd[], int *argcp, char *argv[])
54      {
55          char *cmdp = cmd;
56          char *end;
57          int i = 0;
58
59          /* fgets creates null-terminated string. stdin is pre-defined C constant
(gdb) list
60           *   for standard intput.  feof(stdin) tests for file:end-of-file.
61           */
62          if (fgets(cmd, MAXLINE, stdin) == NULL && feof(stdin)) {
63              printf("Couldn't read from standard input. End of file? Exiting ...\n");
64              exit(1);  /* any non-zero value for exit means failure. */
65          }
66          while ( (cmdp = getword(cmdp, &end)) != NULL ) { /* end is output param */
67              /* getword converts word into null-terminated string */
68              argv[i++] = cmdp;
69              /* "end" brings us only to the '\0' at end of string */
(gdb) next
57          int i = 0;
(gdb) n
62          if (fgets(cmd, MAXLINE, stdin) == NULL && feof(stdin)) {
(gdb) n
ls README
66          while ( (cmdp = getword(cmdp, &end)) != NULL ) { /* end is output param */
(gdb) p cmd
$8 = 0xbffff9b4 "ls README\n"
(gdb) frame 0
#0  getargs (cmd=0xbffff9b4 "ls README\n", argcp=0xbffff9b0, argv=0xbffff960)
    at hw2.c:66
66          while ( (cmdp = getword(cmdp, &end)) != NULL ) { /* end is output param */
(gdb) list
61           */
62          if (fgets(cmd, MAXLINE, stdin) == NULL && feof(stdin)) {
63              printf("Couldn't read from standard input. End of file? Exiting ...\n");
64              exit(1);  /* any non-zero value for exit means failure. */
65          }
66          while ( (cmdp = getword(cmdp, &end)) != NULL ) { /* end is output param */
67              /* getword converts word into null-terminated string */
68              argv[i++] = cmdp;
69              /* "end" brings us only to the '\0' at end of string */
70              cmdp = end + 1;
(gdb) step
getword (begin=0xbffff9b4 "ls README\n", end_ptr=0xbffff928) at hw2.c:28
28          char * end = begin;
(gdb) where
#0  getword (begin=0xbffff9b4 "ls README\n", end_ptr=0xbffff928) at hw2.c:28
#1  0x080487df in getargs (cmd=0xbffff9b4 "ls README\n", argcp=0xbffff9b0, 
    argv=0xbffff960) at hw2.c:66
#2  0x080488e5 in main (argc=1, argv=0xbffffb34) at hw2.c:109
(gdb) list
23      /* In C, "static" means not visible outside of file.  This is different
24       * from the usage of "static" in Java.
25       * Note that end_ptr is an output parameter.
26       */
27      static char * getword(char * begin, char **end_ptr) {
28          char * end = begin;
29
30          while ( *begin == ' ' )
31              begin++;  /* Get rid of leading spaces. */
32          end = begin;
(gdb) ptype begin
type = char *
(gdb) ptype *begin
type = char
(gdb) next
30          while ( *begin == ' ' )
(gdb) p end
$9 = 0xbffff9b4 "ls README\n"
(gdb) p begin
$10 = 0xbffff9b4 "ls README\n"
(gdb) n
32          end = begin;
(gdb) list
27      static char * getword(char * begin, char **end_ptr) {
28          char * end = begin;
29
30          while ( *begin == ' ' )
31              begin++;  /* Get rid of leading spaces. */
32          end = begin;
33          while ( *end != '\0' && *end != '\n' && *end != ' ' )
34              end++;  /* Keep going. */
35          if ( end == begin )
36              return NULL;  /* if no more words, return NULL */
(gdb) next
33          while ( *end != '\0' && *end != '\n' && *end != ' ' )
(gdb) next
34              end++;  /* Keep going. */
(gdb) next
33          while ( *end != '\0' && *end != '\n' && *end != ' ' )
(gdb) p end
$11 = 0xbffff9b5 "s README\n"
(gdb) p begin
$12 = 0xbffff9b4 "ls README\n"
(gdb) next
34              end++;  /* Keep going. */
(gdb) next
33          while ( *end != '\0' && *end != '\n' && *end != ' ' )
(gdb) next
35          if ( end == begin )
(gdb) p begin
$13 = 0xbffff9b4 "ls README\n"
(gdb) p end
$14 = 0xbffff9b6 " README\n"
(gdb) ptype end
type = char *
(gdb) list
30          while ( *begin == ' ' )
31              begin++;  /* Get rid of leading spaces. */
32          end = begin;
33          while ( *end != '\0' && *end != '\n' && *end != ' ' )
34              end++;  /* Keep going. */
35          if ( end == begin )
36              return NULL;  /* if no more words, return NULL */
37          *end = '\0';  /* else put string terminator at end of this word. */
38          *end_ptr = end;
39          if (begin[0] == '$') { /* if this is a variable to be expanded */
(gdb) where
#0  getword (begin=0xbffff9b4 "ls README\n", end_ptr=0xbffff928) at hw2.c:35
#1  0x080487df in getargs (cmd=0xbffff9b4 "ls README\n", argcp=0xbffff9b0, 
    argv=0xbffff960) at hw2.c:66
#2  0x080488e5 in main (argc=1, argv=0xbffffb34) at hw2.c:109
(gdb) list
40              begin = getenv(begin+1); /* begin+1, to skip past '$' */
41              if (begin == NULL) {
42                  perror("getenv");
43                  begin = "UNDEFINED";
44              }
45          }
46          return begin; /* This word is now a null-terminated string.  return it. */
47      }
48
49      /* In C, "int" is used instead of "bool", and "0" means false, any
(gdb) frame 0
#0  getword (begin=0xbffff9b4 "ls README\n", end_ptr=0xbffff928) at hw2.c:35
35          if ( end == begin )
(gdb) list
30          while ( *begin == ' ' )
31              begin++;  /* Get rid of leading spaces. */
32          end = begin;
33          while ( *end != '\0' && *end != '\n' && *end != ' ' )
34              end++;  /* Keep going. */
35          if ( end == begin )
36              return NULL;  /* if no more words, return NULL */
37          *end = '\0';  /* else put string terminator at end of this word. */
38          *end_ptr = end;
39          if (begin[0] == '$') { /* if this is a variable to be expanded */
(gdb) p end == begin
$15 = 0
(gdb) next
37          *end = '\0';  /* else put string terminator at end of this word. */
(gdb) p begin
$16 = 0xbffff9b4 "ls README\n"
(gdb) next
38          *end_ptr = end;
(gdb) p begin
$17 = 0xbffff9b4 "ls"
(gdb) p end
$18 = 0xbffff9b6 ""
(gdb) list
33          while ( *end != '\0' && *end != '\n' && *end != ' ' )
34              end++;  /* Keep going. */
35          if ( end == begin )
36              return NULL;  /* if no more words, return NULL */
37          *end = '\0';  /* else put string terminator at end of this word. */
38          *end_ptr = end;
39          if (begin[0] == '$') { /* if this is a variable to be expanded */
40              begin = getenv(begin+1); /* begin+1, to skip past '$' */
41              if (begin == NULL) {
42                  perror("getenv");
(gdb) frame 0
#0  getword (begin=0xbffff9b4 "ls", end_ptr=0xbffff928) at hw2.c:38
38          *end_ptr = end;
(gdb) list
33          while ( *end != '\0' && *end != '\n' && *end != ' ' )
34              end++;  /* Keep going. */
35          if ( end == begin )
36              return NULL;  /* if no more words, return NULL */
37          *end = '\0';  /* else put string terminator at end of this word. */
38          *end_ptr = end;
39          if (begin[0] == '$') { /* if this is a variable to be expanded */
40              begin = getenv(begin+1); /* begin+1, to skip past '$' */
41              if (begin == NULL) {
42                  perror("getenv");
(gdb) n
39          if (begin[0] == '$') { /* if this is a variable to be expanded */
(gdb) n
46          return begin; /* This word is now a null-terminated string.  return it. */
(gdb) p begin
$19 = 0xbffff9b4 "ls"
(gdb) n
47      }
(gdb) list
42                  perror("getenv");
43                  begin = "UNDEFINED";
44              }
45          }
46          return begin; /* This word is now a null-terminated string.  return it. */
47      }
48
49      /* In C, "int" is used instead of "bool", and "0" means false, any
50       * non-zero number (traditionally "1") means true.
51       */
(gdb) next
getargs (cmd=0xbffff9b4 "ls", argcp=0xbffff9b0, argv=0xbffff960) at hw2.c:68
68              argv[i++] = cmdp;
(gdb) where
#0  getargs (cmd=0xbffff9b4 "ls", argcp=0xbffff9b0, argv=0xbffff960)
    at hw2.c:68
#1  0x080488e5 in main (argc=1, argv=0xbffffb34) at hw2.c:109
(gdb) list
63              printf("Couldn't read from standard input. End of file? Exiting ...\n");
64              exit(1);  /* any non-zero value for exit means failure. */
65          }
66          while ( (cmdp = getword(cmdp, &end)) != NULL ) { /* end is output param */
67              /* getword converts word into null-terminated string */
68              argv[i++] = cmdp;
69              /* "end" brings us only to the '\0' at end of string */
70              cmdp = end + 1;
71          }
72          argv[i] = NULL; /* Create additional null word at end for safety. */
(gdb) p cmdp
$20 = 0xbffff9b4 "ls"
(gdb) p i
$21 = 0
(gdb) p argv[0]
$22 = 0x8048400 "GLIBC_2.0"
(gdb) next
70              cmdp = end + 1;
(gdb) p argv[0]
$23 = 0xbffff9b4 "ls"
(gdb) p end
$24 = 0xbffff9b6 ""
(gdb) p cmdp
$25 = 0xbffff9b4 "ls"
(gdb) frame 0
#0  getargs (cmd=0xbffff9b4 "ls", argcp=0xbffff9b0, argv=0xbffff960)
    at hw2.c:70
70              cmdp = end + 1;
(gdb) list
65          }
66          while ( (cmdp = getword(cmdp, &end)) != NULL ) { /* end is output param */
67              /* getword converts word into null-terminated string */
68              argv[i++] = cmdp;
69              /* "end" brings us only to the '\0' at end of string */
70              cmdp = end + 1;
71          }
72          argv[i] = NULL; /* Create additional null word at end for safety. */
73          *argcp = i;
74      }
(gdb) next
68              argv[i++] = cmdp;
(gdb) p argv[0]
$26 = 0xbffff9b4 "ls"
(gdb) p argv[1]
$27 = 0x0
(gdb) next
70              cmdp = end + 1;
(gdb) p cmdp
$28 = 0xbffff9b7 "README"
(gdb) next
72          argv[i] = NULL; /* Create additional null word at end for safety. */
(gdb) p cmdp
$29 = 0x0
(gdb) list
67              /* getword converts word into null-terminated string */
68              argv[i++] = cmdp;
69              /* "end" brings us only to the '\0' at end of string */
70              cmdp = end + 1;
71          }
72          argv[i] = NULL; /* Create additional null word at end for safety. */
73          *argcp = i;
74      }
75
76      static void execute(int argc, char *argv[])
(gdb) next
73          *argcp = i;
(gdb) next
74      }
(gdb) p *argcp
$30 = 2
(gdb) next
main (argc=1, argv=0xbffffb34) at hw2.c:112
112             if ( childargc > 0 && strcmp(childargv[0], "exit") == 0 )
(gdb) list
107             printf("%% "); /* printf uses %d, %s, %x, etc.  See 'man 3 printf' */
108             fflush(stdout); /* flush from output buffer to terminal itself */
109             getargs(cmd, &childargc, childargv); /* childargc and childargv are
110                 output args; on input they have garbage, but getargs sets them. */
111             /* Check first for built-in commands. */
112             if ( childargc > 0 && strcmp(childargv[0], "exit") == 0 )
113                 exit(0);
114             else if ( childargc > 0 && strcmp(childargv[0], "logout") == 0 )
115                 exit(0);
116             else
(gdb) next
114             else if ( childargc > 0 && strcmp(childargv[0], "logout") == 0 )
(gdb) n
117                 execute(childargc, childargv);
(gdb) p childargc
$31 = 2
(gdb) p childargv
$32 = {0xbffff9b4 "ls", 0xbffff9b7 "README", 0x0, 0x0, 0x4001fb40 "", 
  0xbfff0002 "", 0x4000e1a0 "\205\300u\344\203\304\b\270\001", 
  0x804837c "libc.so.6", 0x4001c954 "", 0x4001bff4 ",\277\001", 
  0x40044b98 "/N=\366\316\030L\017\371\304-×                                            \"\233|\370Ô
                                                         \"\233|%\301\324\361\354\373\300=\260\"\225\303\070\307\031u\377\001\304\022\311BY\020\334\317쵶w\035\rG\336\315%\265V1\375\307r1\035\a;\372L\214\t)\020\t~\222\034\070\265\357\060j\335\371{\004\\H\261ԡ\034\240\002\352\331\017\071\265\357\060X?\227|\030\034s\354T\200\314sÙ\002;H\205\033\066\rf\375\062v\340ը\247Ká\217\326\036h\233\243\230\313\362\234\002Y1\n\264\006\337\275\350e\235J\032\223\250P\265\250\020\205)%~\016|\030\271\321\070\a\221\222\376\206\357\246:V\323\361I\265$\202\241\067\344Qho", <incomplete sequence \354>..., 0x1 <Address 0x1 out of bounds>, 
  0xbffffa00 "\264\372\377\277\211\217", 0x40008d25 "\205\300\017\205\262", 
  0x0, 0x0, 0x0, 0x4001cbb0 "\300\313\001@", 0x4001c958 "", 0x40044da4 ""}
(gdb) p childargv[0]
$33 = 0xbffff9b4 "ls"
(gdb) p childargv[1]
$34 = 0xbffff9b7 "README"
(gdb) p childargv[2]
$35 = 0x0
(gdb) p childargc
$36 = 2
(gdb) frame 0
#0  main (argc=1, argv=0xbffffb34) at hw2.c:117
117                 execute(childargc, childargv);
(gdb) list
112             if ( childargc > 0 && strcmp(childargv[0], "exit") == 0 )
113                 exit(0);
114             else if ( childargc > 0 && strcmp(childargv[0], "logout") == 0 )
115                 exit(0);
116             else
117                 execute(childargc, childargv);
118         }
119         /* NOT REACHED */
120     }
(gdb) step
execute (argc=2, argv=0xbffff960) at hw2.c:80
80          childpid = fork();
(gdb) list
75
76      static void execute(int argc, char *argv[])
77      {
78          pid_t childpid; /* child process ID */
79
80          childpid = fork();
81          if (childpid == -1) { /* in parent (returned error) */
82              perror("fork"); /* perror => print error string of last system call */
83              printf("  (failed to execute command)\n");
84          }
(gdb) 


# ==============================================================
# When you are writing a program to redirect stdin or stdout,
#   it is useful to look at what a file descriptor is currently pointing to.
#   Here's how to do that from within GDB:

(gdb) info proc
process 5380
...
# So, 5380 is the current process id (pid).
(gdb) shell ls -l /proc/5380/fd
total 0
lrwx------ 1 gene faculty 64 Feb 12 09:35 0 -> /dev/pts/4
lrwx------ 1 gene faculty 64 Feb 12 09:35 1 -> /dev/pts/4
lrwx------ 1 gene faculty 64 Feb 12 09:35 2 -> /dev/pts/4
# So, stdin/stdout/stderr are all still pointing to the terminal.

# While we're here, let's look at the memory map (the programmer's model)
#   just for fun..
(gdb) shell cat /proc/5563/maps
08048000-08049000 r-xp 00000000 00:1b 2096293    /home/gene/a.out
08049000-0804a000 r--p 00000000 00:1b 2096293    /home/gene/a.out
0804a000-0804b000 rw-p 00001000 00:1b 2096293    /home/gene/a.out
f7e09000-f7e0a000 rw-p 00000000 00:00 0 
f7e0a000-f7fad000 r-xp 00000000 08:01 655388     /lib/i386-linux-gnu/libc-2.15.so
f7fad000-f7fae000 ---p 001a3000 08:01 655388     /lib/i386-linux-gnu/libc-2.15.so
f7fae000-f7fb0000 r--p 001a3000 08:01 655388     /lib/i386-linux-gnu/libc-2.15.so
f7fb0000-f7fb1000 rw-p 001a5000 08:01 655388     /lib/i386-linux-gnu/libc-2.15.so
# Here we see the test and data segments for a.out and for libc.
# A text segment has permissions "r-x" and a data segment has
#  permissions "rw-".  A read-only data segment has permissions "r--".