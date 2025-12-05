#include "tuf_gaming_fx705ge.h"

#include <algorithm>

TufGamingFx705ge::TufGamingFx705ge() {
  loadMockData();
}

bool TufGamingFx705ge::refreshSensors() {
  // TODO: Bo sung doc tu ASUS WMI hoac hwmon khi co driver.
  // Hien tai giu du lieu gia lap de hien thi UI.
  loadMockData();
  return true;
}

double TufGamingFx705ge::cpuPackageTempC() const {
  return m_cpuPackage.celsius;
}

double TufGamingFx705ge::pchTempC() const {
  return m_pch.celsius;
}

TufGamingFx705ge::FanSample TufGamingFx705ge::fan() const {
  return m_fan;
}

QVector<TufGamingFx705ge::TemperatureSample> TufGamingFx705ge::detailTemperatures() const {
  return m_details;
}

bool TufGamingFx705ge::setFixedFanPercent(int percent) {
  // TODO: Gui lenh thuc su toi firmware/EC. Tam thoi cap nhat cache.
  const int clamped = std::clamp(percent, 0, 100);
  m_fan.percent = clamped;

  // Mo phong RPM thay doi theo phan tram cai dat.
  m_fan.rpm = 800 + static_cast<int>(clamped * 24);
  return true;
}

bool TufGamingFx705ge::applyPresetMode(const QString &presetName) {
  // TODO: Map preset sang bang fan curve that su. Tam thoi chon phan tram co dinh.
  if (presetName.compare("Silent", Qt::CaseInsensitive) == 0) {
    return setFixedFanPercent(30);
  }
  if (presetName.compare("Performance", Qt::CaseInsensitive) == 0) {
    return setFixedFanPercent(65);
  }
  if (presetName.compare("Turbo", Qt::CaseInsensitive) == 0) {
    return setFixedFanPercent(85);
  }
  // Custom: giu nguyen gia tri hien tai.
  return true;
}

void TufGamingFx705ge::loadMockData() {
  m_cpuPackage = {"CPU Package", 68.0};
  m_pch = {"PCH", 42.0};
  m_fan = {2450, 65};

  m_details = {
      {"CPU Core 1", 67.0},
      {"CPU Core 2", 69.0},
      {"NVMe Drive", 38.0},
      {"PCH", 42.0},
      {"ACPI Zone 1", 55.0},
      {"ACPI Zone 2", 45.0},
  };
}
