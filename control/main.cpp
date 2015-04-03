#include <QApplication>
#include <QDesktopWidget>

#include "main_window.hpp"


int main(int argc, char * argv[])
{
  QApplication app(argc,argv);

  main_window window;

  // gui starts correct
  if(window.init())
  {
    // center image on screen
    window.setGeometry(QStyle::alignedRect(Qt::LeftToRight,
                                           Qt::AlignCenter,
                                           window.size(),
                                           app.desktop()->availableGeometry()));

    window.show();

    return app.exec();
  }

  return 0;
}
