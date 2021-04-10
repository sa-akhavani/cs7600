// This consists of excerpts of C code, showing how to use
//   setcontext/getcontext (not LONGJMP).
//   You will need to extract the sections of code and place them
//   in the appropriate locations in your own code..

// *** Add the include file:
#ifdef LONGJMP
# include <setjmp.h>
#else
# include <ucontext.h>
#endif

// *** In your signal handler, you will need to save the context (registers).
// *** This version makes 'context' a local variabble in the signal handler
// ***   function.  If you use this style, then pass a pointer to
// ***   this local 'context' variable to whichever function writes
// ***   to 'myckpt'.
#ifdef LONGJMP
  sigjmp_buf context;  // A context includes the register values
  if (sigsetjmp(context, 1) != 0) {
    // jumped from setlongjmp (restart)
    return;
  }
#else
  ucontext_t context;  // A context includes the register values
  static int is_restart; // static local variables are stored in data segment
  is_restart = 0;
  getcontext(&context);
  if (is_restart == 1) {
    is_restart = 0;
    return;
  }
  // Wehn we set 'is_restart' is important:
  //   is_restart = 0; // before entering 'getcontext(&context)'
  //   is_restart = 1; // after returning from 'getcontext(&context)'
  //                   // and before the memory segments are written out
  // This guarantees if we enter getcontext(&context) directly (during ckpt),
  //   then when gettcontext returns, 'is_restart == 0'.
  // If we enter getcontext(&context) via setcontext (during restart),
  //   then when gettcontext returns, 'is_restart == 1'.
  is_restart = 1;
#endif

// *** If you are using getcontext() [recommended], then be sure to:
// ***   (i) call getcontext() to fill in the 'context' variable;
// ***   (ii) set 'is_restart = 1'; and _then_
// ***   (iii) save your 'context' and your memory segments
// *** in that order.

// *** When you calculate the size of your 'context' variable, a more portable
// ***   way is 'sizeof(context)' instead of 'sizeof(DATA_TYPE_OF_CONTEXT)'. 
  header.data_size = sizeof(context);   

// *** During restart, declare your context variable (probably as
// ***   a local variable inside your restart function.
#ifdef LONGJMP
  sigjmp_buf context;
#else
  ucontext_t context;
#endif

// *** And read the context from the 'myckpt' file
// ***   (and place read() in a while loop as usual):
  read(fd, &context, header.data_size);

// *** And when you have restored all memory segments, and read the context:
#ifdef LONGJMP
  siglongjmp(context, 1);
#else
  setcontext(&context); 
#endif
