#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

namespace smsc{
namespace emailsme{
namespace util{

bool childRunning=false;
static bool brPipe=false;

static bool sigInit=false;

extern "C" typedef void (*SignalHandler)(int);
static void sig_pipe(int signo)
{
//  fprintf(stderr,"broken pipe or sig child\n");
  childRunning=false;
  brPipe=true;
}

struct PipeGuard{
  int* pipe;
  PipeGuard(int* argPipe):pipe(argPipe){}
  void release(){pipe=0;}
  ~PipeGuard()
  {
    if(pipe)
    {
      if(pipe[0]!=-1)close(pipe[0]);
      if(pipe[1]!=-1)close(pipe[1]);
    }
  }
};

static pid_t piped_child(char **command, int *f_in, int *f_out)
{
  pid_t pid;
  int to_child_pipe[2];
  int from_child_pipe[2];

  if (pipe(to_child_pipe) < 0 )
  {
    return -1;
  }
  PipeGuard inG(to_child_pipe);
  if(pipe(from_child_pipe) < 0)
  {
    return -1;
  }
  PipeGuard outG(from_child_pipe);

  pid = fork();
  if (pid < 0)
  {
    return -1;
  }

  if (pid == 0)
  {
    /* child */
    close(to_child_pipe[1]);to_child_pipe[1]=-1;
    close(from_child_pipe[0]);to_child_pipe[1]=-1;

    if(to_child_pipe[0] != STDIN_FILENO)
    {
      if(dup2(to_child_pipe[0], STDIN_FILENO) < 0)
      {
        exit(-1);
      }
      close(to_child_pipe[0]);
      to_child_pipe[0]=-1;
    }
    if(from_child_pipe[1] != STDOUT_FILENO)
    {
      if(dup2(from_child_pipe[1], STDOUT_FILENO) < 0)
      {
        exit(-1);
      }
      close(from_child_pipe[1]);
      from_child_pipe[1]=-1;
    }
    inG.release();
    outG.release();
    execvp(command[0], command);
    exit(-1);
  }

  if (close(from_child_pipe[1]) < 0)
  {
    from_child_pipe[1]=-1;
    return -1;
  }
  if(close(to_child_pipe[0]) < 0)
  {
    return -1;
  }
  to_child_pipe[0]=-1;
  inG.release();
  outG.release();

  *f_out = to_child_pipe[1];
  *f_in = from_child_pipe[0];

  return pid;
}



pid_t ForkPipedCmd(const char *cmd, FILE*& f_in,FILE*& f_out)
{
  if(!sigInit)
  {
    if(sigset(SIGPIPE, (SignalHandler)sig_pipe) == SIG_ERR)fprintf(stderr, "signal SIGPIPE error.\n");
    if(sigset(SIGCHLD, (SignalHandler)sig_pipe) == SIG_ERR)fprintf(stderr, "signal SIGCHILD error.\n");
    sigInit=true;
  }
  std::vector<char*> args;
  pid_t ret;
  char *tok;
  int in,out;

  char *str=new char[strlen(cmd)+1];
  strcpy(str,cmd);

  for(tok=strtok(str," ");tok;tok=strtok(NULL," "))
  {
    args.push_back(tok);
  }
  args.push_back(0);
  brPipe=false;
  ret = piped_child(&args[0],&in,&out);
  sleep(1);
  if(ret>0 && !brPipe)
  {
    f_in=fdopen(in,"rb");
    f_out=fdopen(out,"wb");
    childRunning=true;
  }
  delete [] str;
  return brPipe?-1:ret;
}

}
}
}
