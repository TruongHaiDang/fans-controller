#ifndef FANS_CONTROLLER_MAINWINDOW_H
#define FANS_CONTROLLER_MAINWINDOW_H

#include <QAbstractButton>
#include <QButtonGroup>
#include <QComboBox>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QRect>
#include <QScreen>
#include <QScrollArea>
#include <QShowEvent>
#include <QSignalBlocker>
#include <QSize>
#include <QSlider>
#include <QStringList>
#include <QVBoxLayout>
#include <QtGlobal>

#include <algorithm>

#include "main.h"
#include "tuf_gaming_fx705ge.h"

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

  // Xu ly tuong tac preset va dong bo slider.
  void handleModeSelected(int buttonId);
  void applyPresetPercent(int percent);
  void syncModeButtonForPercent(int percent);
  void selectModeButton(const QString &modeName);

  // Xu ly stylesheet va can giua man hinh.
  void applyStyleSheet();
  QString resolveStylePath() const;
  void centerOnScreen();

  // Ham tro giup dinh dang/suy luan thong so sensor.
  QString formatTemperature(double tempC) const;
  QString temperatureSeverity(double tempC) const;
  QString statusTextForSeverity(const QString &severity) const;
  QString accentForStat(const QString &severity) const;

  // Trang thai noi bo.
  bool m_hasCentered = false;            // Dam bao chi can giua mot lan khi hien.
  QLabel *m_fixedSpeedValueLabel = nullptr;  // Hien thi % cua slider.
  QSlider *m_fixedSpeedSlider = nullptr;     // Dieu khien toc do co dinh.
  QButtonGroup *m_modeGroup = nullptr;       // Nhom nut chon che do quat.
  bool m_updatingFromPreset = false;         // Co de bo qua set Custom khi set bang code.

  // Lop doc sensor/dieu khien quat tach rieng ra core/.
  TufGamingFx705ge m_device;
};

#endif  // FANS_CONTROLLER_MAINWINDOW_H
