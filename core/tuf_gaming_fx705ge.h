#ifndef FANS_CONTROLLER_TUF_GAMING_FX705GE_H
#define FANS_CONTROLLER_TUF_GAMING_FX705GE_H

#include <QString>
#include <QVector>

// Luu tru thong tin sensor va dieu khien quat cho may ASUS TUF Gaming FX705GE.
// Hien tai chi co du lieu gia lap de phuc vu UI. Khi co h/w va driver,
// bo sung doc/ghi thuc su tu /sys/ hoac ASUS WMI tai cac ham ben duoi.
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

 private:
  void loadMockData();  // Tam thoi: du lieu minh hoa UI.

  TemperatureSample m_cpuPackage;
  TemperatureSample m_pch;
  FanSample m_fan;
  QVector<TemperatureSample> m_details;
};

#endif  // FANS_CONTROLLER_TUF_GAMING_FX705GE_H
