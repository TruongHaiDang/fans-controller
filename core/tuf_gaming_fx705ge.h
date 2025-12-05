#ifndef FANS_CONTROLLER_TUF_GAMING_FX705GE_H
#define FANS_CONTROLLER_TUF_GAMING_FX705GE_H

#include <QString>
#include <QVector>
#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QStringList>
#include <QtGlobal>
#include <algorithm>

// Doc thong tin sensor va dieu khien quat cho ASUS TUF Gaming FX705GE
// thong qua cac file sysfs (hwmon/pwm) ma script asus_fan_report.sh da phat hien.
// Neu khong doc/ghi duoc (quyen hoac thieu thiet bi), cac gia tri tra ve se la 0.
class TufGamingFx705ge {
 public:
  // Mau du lieu nhiet do.
  struct TemperatureSample {
    QString label;
    double celsius;
  };

  // Mau du lieu quat.
  struct FanSample {
    int rpm;
    int percent;  // Gia tri phan tram neu dang o che do fixed.
  };

  TufGamingFx705ge();

  // Cap nhat cache sensor. Tra ve false neu doc that bai.
  bool refreshSensors();

  double cpuPackageTempC() const;
  double pchTempC() const;
  FanSample fan() const;
  QVector<TemperatureSample> detailTemperatures() const;

  // Thiet lap che do quat co dinh theo phan tram (0-100). Tra ve true neu thanh cong.
  bool setFixedFanPercent(int percent);

  // Ap dung preset ("Silent", "Performance", "Turbo", "Custom"...).
  bool applyPresetMode(const QString &presetName);

  // Tra ve chuoi loi gan nhat (neu co) de hien thi cho nguoi dung.
  QString lastError() const { return m_lastError; }

 private:
  // Doc tu sysfs; neu that bai thi tra ve 0 an toan.
  void loadFromSysfs();

  // Du lieu gia lap an toan (0.0) khi khong doc duoc.
  void loadMockData();

  QString findHwmonByName(const QStringList &needles) const;
  QVector<TemperatureSample> readTemps(const QString &hwmonPath) const;
  int readFanRpm(const QString &hwmonPath) const;
  int readPwmMax(const QString &hwmonPath) const;
  int readPwmValue(const QString &hwmonPath) const;
  bool writePwmValue(const QString &hwmonPath, int pwmValue) const;
  bool writePwmEnableManual(const QString &hwmonPath) const;
  QString readTextFile(const QString &path) const;
  double parseTempMilli(const QString &raw) const;

  TemperatureSample m_cpuPackage;
  TemperatureSample m_pch;
  FanSample m_fan;
  QVector<TemperatureSample> m_details;

  // Luu duong dan hwmon asus de set PWM sau khi da phat hien o refreshSensors.
  QString m_asusHwmonPath;
  QString m_lastError;
};

#endif  // FANS_CONTROLLER_TUF_GAMING_FX705GE_H
