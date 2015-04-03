#include <QCoreApplication>

#include <server.hpp>


int main(int argc , char * argv[])
{
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
