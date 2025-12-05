#include <QApplication>

#include "main.h"
#include "mainwindow.h"

int main(int argc, char *argv[]) {
  // Khoi tao QApplication (bat buoc cho ung dung Qt Widgets).
  QApplication app(argc, argv);

  // Dat ten ung dung de phuc vu viec debug va lay thong tin trong he thong.
  QApplication::setApplicationName("Fan Monitoring & Control");

  // Tao cua so chinh va hien thi. Logic can giua man hinh duoc xu ly trong
  // MainWindow::showEvent de dam bao kich thuoc cuoi cung da on dinh.
  MainWindow window;
  window.show();

  return app.exec();
}
