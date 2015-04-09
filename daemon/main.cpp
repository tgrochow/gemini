#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <QCoreApplication>

#include <server.hpp>


int main(int argc , char * argv[])
{
  pid_t pid, sid;

 //Fork the Parent Process
  pid = fork();

  if (pid < 0) { exit(EXIT_FAILURE); }

  //We got a good pid, Close the Parent Process
  if (pid > 0) { exit(EXIT_SUCCESS); }

  //Change File Mask
  umask(0);

  //Create a new Signature Id for our child
  sid = setsid();
  if (sid < 0) { exit(EXIT_FAILURE); }

  //Change Directory
  //If we cant find the directory we exit with failure.
  if ((chdir("/")) < 0) { exit(EXIT_FAILURE); }

  //Close Standard File Descriptors
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);


  QCoreApplication app(argc,argv);

  gemini::server server;


  // server starts correct
  if(server.start())
  {
    // start event loop
    return app.exec();
  }


  return 0;
}
