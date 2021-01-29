#ifndef _CATCH_CATCH_BREAK_
#define _CATCH_CATCH_BREAK_
#include <signal.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <unwind.h>
#include "log.h"
#define SIG_STACK_BUFFER_SIZE SIGSTKSZ

#define SIG_CATCH_COUNT 5

uintptr_t pc_from_ucontext(const ucontext_t *uc); 

static const int sig_catch[] = {
  SIGILL,SIGBUS,SIGFPE,SIGSEGV,
#ifdef SIGSTKFLT
  //协处理器栈错误没有在linux系统中使用
  SIGSTKFLT
#endif
  ,0
};

typedef struct native_code_global_struct{
  int initialized;  
  pthread_mutex_t mutex;  
  struct sigaction *sa_old;

} native_code_global_struct;

#define NATIVE_CODE_GLOBAL_INITIALIZER {0,PTHREAD_MUTEX_INITIALIZER,NULL}

typedef struct native_code_handler_struct{
  char* stack_buff; 
  size_t stack_buff_size; 
  stack_t stack_old;

  int code; 
  siginfo_t si; 
  
  size_t frames_size;
  size_t frames_skip;
  
  const char* file;
  int line;
  int alarm;


}native_code_handler_struct;



/* //还原可选栈 */
// static void revert_alternate_stack(){
//   stack_t ss;
//   if(sigaltstack(NULL,&ss) == 0){
//     //0001  1110 & 0001
//     ss.ss_flags &= ~SS_ONSTACK;//ss_flags 重置为0
//     sigaltstack(&ss, NULL);
//   }
// }
/*  */


int native_code_handler_struct_free(native_code_handler_struct *const t);
/**
 *  const int *a; // 修饰指向的对象，a可以变，a指向的内容不能变
    int const *a; // 修饰指向的对象，a可以变，a指向的内容不能变
    int *const a; // 修饰a指针，a不可变，a指向的对象可变
    const int *const a; // a及其指向的对象都不能变

 *
 */
native_code_handler_struct* native_code_struct_init();

static const char* catch_desc_sig(int sig,int code){
  switch(sig){
    case SIGILL:
      switch(code){
        case ILL_ILLOPC:
          return "illegal opcode"; 
        case ILL_ILLOPN:
          return "illegal operand";
        case ILL_ILLADR:
          return "illegal addressing mode";
        case ILL_ILLTRP:
          return "illegal trap";
        case ILL_PRVOPC:
          return "privileged opcode";
        case ILL_PRVREG:
          return "privileged register";
        case ILL_COPROC:
          return "copprocessor error";
        case ILL_BADSTK:
          return "internal stack error";
        default:
          return "illegal operation";

      }
      break;
    case SIGFPE:
      switch(code){
        case FPE_INTDIV:
          return "integer divided by zero";
        case FPE_INTOVF:
          return "intger overflow";
        case FPE_FLTDIV:
          return "floatting-point divided by zero";
        case FPE_FLTOVF:
          return "floatting-point overflow";
        case FPE_FLTUND:
          return "floatting-point underflow";
        case FPE_FLTRES:
          return "floatting-point inexact result";
        case FPE_FLTINV:
          return "invalid floatting-point operation";
        case FPE_FLTSUB:
          return "subscript out of range";
        default:
          return "floatting-point";
      } 
      break;
    case SIGSEGV:
      switch(code){
        case SEGV_MAPERR:
          return "address not mapped to object";
        case SEGV_ACCERR:
          return "invalid permission for mapped object";
        default:
          return "segmentation violation";

      }
      break;
    case SIGBUS:
      switch(code){
        case BUS_ADRALN:
          return "invalid address alignment";
        case BUS_ADRERR:
          return "nonexistent pysical address";
        case BUS_OBJERR:
          return "Object-specfic hardware error";
        default:
          return "bus error";
      }
      break;
    default:
      return "unknow error";
  } 
} 
native_code_handler_struct* get_native_handler_struct();
void signal_handler(const int code,siginfo_t *const si,void *const sc);
int setup_signal_global();


int catch_handler_setup(int setup_thread);

int cleanup();

 
#endif
