#ifndef FANS_CONTROLLER_MAINWINDOW_H
#define FANS_CONTROLLER_MAINWINDOW_H

#include <QButtonGroup>
#include <QFrame>
#include <QLabel>
#include <QMainWindow>
#include <QSlider>

#include "main.h"

class QShowEvent;

// MainWindow dong vai tro la cua so chinh gom toan bo bang dieu khien
// quan ly quat. Mo ta layout, tao widget con va nap stylesheet.
class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow() override = default;

 protected:
  void showEvent(QShowEvent *event) override;

 private:
  // Cac ham tao cac khu vuc UI rieng le de code ro rang va de dieu chinh.
  void buildUi();
  QWidget *createHeader();
  QWidget *createStatsRow();
  QWidget *createTrendAndDetailsRow();
  QWidget *createFanModeRow();
  QWidget *createProfilesRow();
  QWidget *createFixedSpeedRow();

  // Ham tro giup tao cac thanh phan nho hon.
  QFrame *createStatCard(const QString &iconText, const QString &title,
                         const QString &valueText, const QString &statusText,
                         const QString &accentProperty);
  QWidget *createDetailLine(const QString &label, const QString &value,
                            const QString &severityProperty);

  // Xu ly stylesheet va can giua man hinh.
  void applyStyleSheet();
  QString resolveStylePath() const;
  void centerOnScreen();

  // Trang thai noi bo.
  bool m_hasCentered = false;            // Dam bao chi can giua mot lan khi hien.
  QLabel *m_fixedSpeedValueLabel = nullptr;  // Hien thi % cua slider.
  QSlider *m_fixedSpeedSlider = nullptr;     // Dieu khien toc do co dinh.
  QButtonGroup *m_modeGroup = nullptr;       // Nhom nut chon che do quat.
};

#endif  // FANS_CONTROLLER_MAINWINDOW_H
