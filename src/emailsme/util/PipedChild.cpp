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

static void sig_pipe(int signo)
{
//  fprintf(stderr,"broken pipe or sig child\n");
  childRunning=false;
  brPipe=true;
}

static pid_t piped_child(char **command, int *f_in, int *f_out)
{
  pid_t pid;
  int to_child_pipe[2];
  int from_child_pipe[2];

  if (pipe(to_child_pipe) < 0 || pipe(from_child_pipe) < 0) {
    return -1;
  }

  pid = fork();
  if (pid < 0) {
    return -1;
  }

  if (pid == 0) {
    /* child */
    close(to_child_pipe[1]);
    close(from_child_pipe[0]);

    if(to_child_pipe[0] != STDIN_FILENO)
    {
      if(dup2(to_child_pipe[0], STDIN_FILENO) < 0)
      {
        return -1;
      }
      close(to_child_pipe[0]);
    }
    if(from_child_pipe[1] != STDOUT_FILENO)
    {
      if(dup2(from_child_pipe[1], STDOUT_FILENO) < 0)
      {
        return -1;
      }
      close(from_child_pipe[1]);
    }

    execvp(command[0], command);
    exit(-1);
  }

  if (close(from_child_pipe[1]) < 0 || close(to_child_pipe[0]) < 0)
  {
    return -1;
  }

  *f_out = to_child_pipe[1];
  *f_in = from_child_pipe[0];

  return pid;
}



pid_t ForkPipedCmd(const char *cmd, FILE*& f_in,FILE*& f_out)
{
  if(!sigInit)
  {
    if(signal(SIGPIPE, sig_pipe) == SIG_ERR)fprintf(stderr, "signal SIGPIPE error.\n");
    if(signal(SIGCHLD, sig_pipe) == SIG_ERR)fprintf(stderr, "signal SIGCHILD error.\n");
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

};
};
};
