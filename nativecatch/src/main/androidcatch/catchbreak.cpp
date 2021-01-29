//#define _GNU_SOURCE 
#include "catchbreak.h"
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>

//typedef unsigned long int	uintptr_t;
//声明全局的native_code_global_struct
struct native_code_global_struct native_code_g = NATIVE_CODE_GLOBAL_INITIALIZER;  

pthread_key_t native_code_thread;


struct backtrace_state_t{
  void** currrent;
  void** end;
}backtrace_state_t;

struct crash_msg{
  int size;//函数调用堆栈数组的大小
  int total;//总的字节数
  unsigned long ocurraddr;//发生的奔溃的地址
  long* funcAddrs;//函数调用堆栈的地址 数组
  const char* msg;//相关信号的内容信息
}crash_msg;


_Unwind_Reason_Code unwind_callback(struct _Unwind_Context* context,void *arg){
  struct backtrace_state_t *state = (struct backtrace_state_t*)arg;     
  unsigned long pc = _Unwind_GetIP(context);   
  if(pc){
    if(state->currrent == state->end){
      return _URC_END_OF_STACK;
    }else{
      *state->currrent++ = (void*)pc;
    }
  } 
  return _URC_NO_REASON;
}

void writeFile(struct crash_msg* error){
 //int fd = open("crash.sh",O_CREAT | O_WRONLY,S_IRWXU);
 FILE* fd = fopen("/sdcard/crash.sh", "wr");
 if(fd == NULL){
    LOGE("write log error /sdcard/carsh.sh \n");
    return;
 }
 int size = 0;
 /** size+=write(fd,&(error->size), sizeof(int)); */
 /** size+=write(fd, &(error->total), sizeof(int)); */
 /** size+=write(fd,(error->funcAddrs), sizeof(long)*20); */
 /** size += write(fd, error->msg, strlen(error->msg)+1); */
 /** p */
 //rintf("write long size:%d\n",size);
 //add sh file head
 const char* head = "#!/bin/bash\n";
 fprintf(fd,head,strlen(head)+1);

 fprintf(fd,"echo \"occur reason:%s\" > crash1.log\n", error->msg);
 fprintf(fd,"echo \"occur position:\" > crash2.log\n");
 fprintf(fd,"addr2line -f -e $1 %x > crash3.log\n",(unsigned int)error->ocurraddr);
  
 /** const char* cmd2 = "addr2line -f -e $1 "; */
 /** fprintf(fd,cmd2,strlen(cmd2)+1); */
 /** fprintf(fd,"%x",error->ocurraddr); */
 /** const char* file2 = " > crash2.log"; */
 /** fprintf(fd, file2, strlen(file2)); */
 /**   */
  

 const char* cmd = "addr2line -f -e $1 ";
 fprintf(fd, cmd, strlen(cmd)+1);
 int i = 0;
  
 for(i = 0; i < error->size;i++){
   char addr[20];  
   if(error->funcAddrs[i] == 0L)
     continue;
   sprintf(addr,"%x ",(unsigned int)error->funcAddrs[i]);
   fprintf(fd,addr, strlen(addr));
 }
 const char* cmd1 = " > crash4.log\n"; 
 fprintf(fd,cmd1,strlen(cmd1));
  
 fprintf(fd, "cat *.log > merge.log\n");
 fprintf(fd, "rm -f crash*.log");

  //close(fd);
  fclose(fd);
 }


size_t capture_backtrace(void** buffer,size_t max){
  struct backtrace_state_t state = {buffer,buffer+max}; 
  _Unwind_Backtrace(unwind_callback, &state);
  return state.currrent-buffer;
}

//定义打印的调用栈的层数
const int max_line = 20;

void getStackBuffer(struct crash_msg* error){

  void *buffer[max_line];
  error->size = max_line;
  error->funcAddrs =static_cast<long*>(calloc(sizeof(long), max_line));
  error->total = max_line*sizeof(long);

  //捕捉崩溃的时候调用方法栈
  int frames_size = capture_backtrace(buffer, max_line);

  for (int i = 0; i < frames_size; i++) {

    Dl_info info;  
    const void *addr = buffer[i];

    if (dladdr(addr, &info) && info.dli_fname) {  
      void * const nearest = info.dli_saddr;
      //an offset in a section of relocatable object ,addr2line can covert it to get filename and line
      unsigned int* addr_relative =reinterpret_cast<unsigned int*> (static_cast<const char*>(addr) - static_cast<const char*>(info.dli_fbase));  
      error->funcAddrs[i] = (long)addr_relative;
      LOGD("dl name:%s  addr:%p nearest:%p\n",info.dli_fname,addr_relative,nearest);

    }
  }
}


 

int native_code_handler_struct_free(native_code_handler_struct *const t){
  int code = 0;
  if(t == NULL){
    return -1;
  }
  //之前设置过可选的栈 回复它
  if(t->stack_old.ss_sp != NULL && sigaltstack(&t->stack_old, NULL) != 0){
  } 

  //free alternative stack
  if(t->stack_buff != NULL){
    free(t->stack_buff); 
    t->stack_buff = NULL;
    t->stack_buff_size = 0;
  }  
  free(t);
  return code;
}
/**
 *  const int *a; // 修饰指向的对象，a可以变，a指向的内容不能变
    int const *a; // 修饰指向的对象，a可以变，a指向的内容不能变
    int *const a; // 修饰a指针，a不可变，a指向的对象可变
    const int *const a; // a及其指向的对象都不能变

 *
 */
native_code_handler_struct* native_code_struct_init(){
  stack_t stack;
  //t指针不可变，t指向的对象可变
  //calloc 分配数组空间
  native_code_handler_struct *const t = reinterpret_cast<native_code_handler_struct*>(calloc(sizeof(native_code_handler_struct), 1));    
  if(t == NULL){
    return NULL;
  } 
  LOGD("install alternative thread stack\n"); 
  
  t->stack_buff_size = SIG_STACK_BUFFER_SIZE;
  t->stack_buff =reinterpret_cast<char*>(malloc(SIG_STACK_BUFFER_SIZE)); 
  if(t->stack_buff == NULL){
    native_code_handler_struct_free(t);   
    return NULL;
  } 
  
  //setup alternative stack
  memset(&stack, 0, sizeof(stack));  
  stack.ss_sp = t->stack_buff;
  stack.ss_size = t->stack_buff_size;
  stack.ss_flags = 0;

  if(sigaltstack(&stack, &t->stack_old) != 0){
    native_code_handler_struct_free(t);
    return NULL;
  }
  return t;

}
 
native_code_handler_struct* get_native_handler_struct(){
  return (native_code_handler_struct*)pthread_getspecific(native_code_thread);
}
//根据崩溃时的上下文环境 获取奔溃时候的内存位置
uintptr_t pc_from_ucontext(const ucontext_t *uc) {
#if (defined(__arm__))
  return uc->uc_mcontext.arm_pc;
#elif defined(__aarch64__)
  return uc->uc_mcontext.pc;
#elif (defined(__x86_64__))
  return (long long int)uc->uc_mcontext.gregs[REG_RIP];
#elif (defined(__i386))
  return uc->uc_mcontext.gregs[REG_EIP];
#elif (defined (__ppc__)) || (defined (__powerpc__))
  return uc->uc_mcontext.regs->nip;
#elif (defined(__hppa__))
  return uc->uc_mcontext.sc_iaoq[0] & ~0x3UL;
#elif (defined(__sparc__) && defined (__arch64__))
  return uc->uc_mcontext.mc_gregs[MC_PC];
#elif (defined(__sparc__) && !defined (__arch64__))
  return uc->uc_mcontext.gregs[REG_PC];
#else
  return 0;
#error "Architecture is unknown, please report me!"

#endif
}

void getOccuredAddr(const void* addr,struct crash_msg* msg){
  Dl_info info;  
  if (dladdr(addr, &info) && info.dli_fname) {  
    void * const nearest = info.dli_saddr;  
    unsigned int*  addr_relative = (unsigned int *) (reinterpret_cast<long>(addr) - reinterpret_cast<long> (info.dli_fbase));  
    LOGD("dl name:%s  addr:%p sname:%s\n",info.dli_fname,addr_relative,info.dli_sname);
    msg->ocurraddr =reinterpret_cast<unsigned long>( addr_relative);
  }

}

void signal_handler(const int code,siginfo_t *const si,void *const sc){
  LOGD("caught singal\n");
  signal(code, SIG_DFL);  
  alarm(30);

  struct crash_msg error; 
  const char* msg = catch_desc_sig(code, si->si_code); 

  error.msg = msg;
  getStackBuffer(&error);
  error.total+=strlen(msg)+1;
  uintptr_t pc = pc_from_ucontext(reinterpret_cast<const ucontext_t *>(sc));  
  getOccuredAddr((unsigned long long int*)pc, &error);

  //LOGD("write error total %d\n",error.total);
  writeFile(&error);

  free(error.funcAddrs);
  
  LOGD("signal msg:%s\n",msg);
      //cleanup();
  cleanup();
  //exit(0);

}

int setup_signal_global(){
  if(native_code_g.initialized++ != 0){
    return 0;
  }
  size_t i; 
  struct sigaction sa; 
  memset(&sa, 0,sizeof(sa));
  sigemptyset(&sa.sa_mask); 
  sa.sa_sigaction = signal_handler; 
  sa.sa_flags = SA_ONSTACK | SA_SIGINFO; 
  //为每一个信号分配一个空间保存它之前的siaction 
  native_code_g.sa_old =reinterpret_cast<struct sigaction*>(calloc(sizeof(struct sigaction), 32));
  if(native_code_g.sa_old == NULL){
    return -1; 
  } 
  //安装信号处理函数 
  for(i = 0; sig_catch[i] != 0; i++){
    const int sig = sig_catch[i];  
    const struct sigaction *const action = &sa;  
    if(sigaction(sig,action, &native_code_g.sa_old[sig]) != 0){
      return -1; 
    } 
  } 
  //initiate thread var
  if(pthread_key_create(&native_code_thread, NULL) != 0){
    return -1;  
  }
  return 0;
}



int catch_handler_setup(int setup_thread){
  int code;  
  LOGD("setup a new handler"); 
  
  if(pthread_mutex_lock(&native_code_g.mutex) != 0){
    return -1; 
  }
  //信号处理函数安装
  code = setup_signal_global(); 
  if(pthread_mutex_unlock(&native_code_g.mutex) != 0){
    return -1;
  }
  if(code != 0)
    return -1;
  if(setup_thread && get_native_handler_struct() == NULL){
    //全局变量存储handler程序执行栈
    native_code_handler_struct *const t = 
      native_code_struct_init();
      
    if(t == NULL)
      return -1;
   //将空间保存到线程区域里
   if(pthread_setspecific(native_code_thread,t) != 0){
      native_code_handler_struct_free(t); 
      return -1;
   }
  }
  return 0;

}


int cleanup(){
  native_code_handler_struct *const t = get_native_handler_struct();
  if(t != NULL){
    if(pthread_setspecific(native_code_thread, NULL) != 0){
      LOGD("pthread_setspecific failed\n");
    }
    if(native_code_handler_struct_free(t) != 0){
      return -1; 
    } 
    LOGD("removed alternative stack\n"); 
      
  }
  if(pthread_mutex_lock(&native_code_g.mutex) != 0){
    LOGD("mutex lock failed\n");   
  }
  
  if(--native_code_g.initialized == 0){
    size_t i; 
    LOGD("removing global handler signals\n"); 
    //restore signal handlers
    for(i = 0; 0 != sig_catch[i]; i++){
      const int sig = sig_catch[i]; 
      if(sigaction(sig,&native_code_g.sa_old[sig_catch[i]],NULL) != 0 ){
        return -1;
      }
    }   
    free(native_code_g.sa_old);
    native_code_g.sa_old = NULL; 
    if(pthread_key_delete(native_code_thread) != 0){
      
    }
    
  } 
  if(pthread_mutex_unlock(&native_code_g.mutex) != 0){

  }  
  return 0;
}

/* void crash(int i ){ */
//   int a = 0;
//   printf("crash addr: %p\n",&a);
//   int* addr = NULL;
//   *addr = 0;
//   //crash(i+1);
// }
//
// int main(){
//   catch_handler_setup(1);
//   crash(0);
/* } */
