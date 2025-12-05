#include "tuf_gaming_fx705ge.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QStringList>
#include <QtGlobal>

#include <algorithm>

namespace {
// Ten file thuong dung cho hwmon ASUS tren FX705GE (tham khao file report).
const QString kHwmonBase = "/sys/class/hwmon";
const QStringList kAsusNeedles = {"asus", "asus-nb-wmi"};
const QStringList kCoreTempNeedles = {"coretemp"};
const QStringList kPchNeedles = {"pch", "pch_cannonlake"};
const QStringList kNvmeNeedles = {"nvme"};
const QStringList kAcpiNeedles = {"acpitz", "acpi"};
}  // namespace

TufGamingFx705ge::TufGamingFx705ge() {
  loadMockData();
}

bool TufGamingFx705ge::refreshSensors() {
  // Thu doc thuc te; neu that bai thi mock 0.0 de khong gay nham lan.
  loadFromSysfs();
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
  const int clamped = std::clamp(percent, 0, 100);

  // Neu chua biet duong dan ASUS, co gang tim lai.
  if (m_asusHwmonPath.isEmpty()) {
    m_asusHwmonPath = findHwmonByName(kAsusNeedles);
  }

  if (m_asusHwmonPath.isEmpty()) {
    // Khong co hwmon ASUS -> khong the dieu khien.
    m_fan.percent = clamped;
    m_fan.rpm = 0;
    return false;
  }

  // Dat che do manual truoc khi ghi PWM.
  if (!writePwmEnableManual(m_asusHwmonPath)) {
    m_fan.percent = clamped;
    return false;
  }

  const int pwmMax = readPwmMax(m_asusHwmonPath);
  if (pwmMax <= 0) {
    m_fan.percent = clamped;
    return false;
  }

  const int pwmValue = static_cast<int>(clamped / 100.0 * pwmMax);
  if (!writePwmValue(m_asusHwmonPath, pwmValue)) {
    m_fan.percent = clamped;
    return false;
  }

  // Cap nhat cache: doc lai rpm neu co.
  m_fan.percent = clamped;
  m_fan.rpm = readFanRpm(m_asusHwmonPath);
  return true;
}

bool TufGamingFx705ge::applyPresetMode(const QString &presetName) {
  // Map preset sang % co dinh.
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

void TufGamingFx705ge::loadFromSysfs() {
  // Reset cache truoc khi doc lai.
  m_cpuPackage = {"CPU Package", 0.0};
  m_pch = {"PCH", 0.0};
  m_fan = {0, m_fan.percent};
  m_details.clear();
  bool anySensor = false;

  // 1) Coretemp: CPU package + cac core chi tiet.
  const QString corePath = findHwmonByName(kCoreTempNeedles);
  if (!corePath.isEmpty()) {
    const auto temps = readTemps(corePath);
    for (const auto &t : temps) {
      anySensor = true;
      if (t.label.contains("Package", Qt::CaseInsensitive) ||
          t.label.contains("id 0", Qt::CaseInsensitive)) {
        m_cpuPackage = {"CPU Package", t.celsius};
      }
      m_details.append(t);
    }
  }

  // 2) PCH.
  const QString pchPath = findHwmonByName(kPchNeedles);
  if (!pchPath.isEmpty()) {
    const auto temps = readTemps(pchPath);
    if (!temps.isEmpty()) {
      anySensor = true;
      m_pch = {"PCH", temps.first().celsius};
      m_details.append(temps.first());
    }
  }

  // 3) NVMe (bo sung vao details).
  const QString nvmePath = findHwmonByName(kNvmeNeedles);
  if (!nvmePath.isEmpty()) {
    const auto temps = readTemps(nvmePath);
    for (const auto &t : temps) {
      anySensor = true;
      m_details.append({"NVMe Drive", t.celsius});
    }
  }

  // 4) ACPI zones (acpitz) neu co.
  const QString acpiPath = findHwmonByName(kAcpiNeedles);
  if (!acpiPath.isEmpty()) {
    const auto temps = readTemps(acpiPath);
    int idx = 1;
    for (const auto &t : temps) {
      anySensor = true;
      m_details.append({QString("ACPI Zone %1").arg(idx++), t.celsius});
    }
  }

  // 5) ASUS fan/pwm.
  m_asusHwmonPath = findHwmonByName(kAsusNeedles);
  if (!m_asusHwmonPath.isEmpty()) {
    m_fan.rpm = readFanRpm(m_asusHwmonPath);

    const int pwmMax = readPwmMax(m_asusHwmonPath);
    const int pwmVal = readPwmValue(m_asusHwmonPath);
    if (pwmMax > 0) {
      m_fan.percent = qRound(pwmVal * 100.0 / pwmMax);
    } else {
      m_fan.percent = 0;
    }
    anySensor = true;
  }

  // 6) Neu thieu du lieu chinh, dung mock an toan.
  if (!anySensor) {
    loadMockData();
  }
}

void TufGamingFx705ge::loadMockData() {
  // Du lieu an toan, tranh hieu nham khi khong doc duoc sysfs.
  m_cpuPackage = {"CPU Package", 0.0};
  m_pch = {"PCH", 0.0};
  m_fan = {0, 0};
  m_details = {
      {"CPU Core 1", 0.0},
      {"CPU Core 2", 0.0},
      {"NVMe Drive", 0.0},
      {"PCH", 0.0},
      {"ACPI Zone 1", 0.0},
      {"ACPI Zone 2", 0.0},
  };
}

QString TufGamingFx705ge::findHwmonByName(const QStringList &needles) const {
  QDir hwmonDir(kHwmonBase);
  const auto dirs = hwmonDir.entryList(QStringList() << "hwmon*", QDir::Dirs | QDir::NoDotAndDotDot);
  for (const QString &d : dirs) {
    const QString path = hwmonDir.filePath(d);
    const QString name = readTextFile(path + "/name").toLower();
    for (const QString &needle : needles) {
      if (name.contains(needle.toLower())) {
        return path;
      }
    }
  }
  return {};
}

QVector<TufGamingFx705ge::TemperatureSample> TufGamingFx705ge::readTemps(const QString &hwmonPath) const {
  QVector<TemperatureSample> result;
  QDir dir(hwmonPath);
  const QStringList files = dir.entryList(QStringList() << "temp*_input", QDir::Files);
  for (const QString &f : files) {
    const QString baseName = f.left(f.indexOf("_input"));
    const QString labelPath = dir.filePath(baseName + "_label");
    const QString labelRaw = readTextFile(labelPath);
    const QString label = labelRaw.isEmpty() ? baseName : labelRaw;
    const QString raw = readTextFile(dir.filePath(f));
    result.append({label, parseTempMilli(raw)});
  }
  return result;
}

int TufGamingFx705ge::readFanRpm(const QString &hwmonPath) const {
  QDir dir(hwmonPath);
  const QStringList files = dir.entryList(QStringList() << "fan*_input", QDir::Files);
  for (const QString &f : files) {
    const QString raw = readTextFile(dir.filePath(f));
    bool ok = false;
    int rpm = raw.toInt(&ok);
    if (ok) {
      return rpm;
    }
  }
  return 0;
}

int TufGamingFx705ge::readPwmMax(const QString &hwmonPath) const {
  const QString maxPath = hwmonPath + "/pwm1_max";
  const QString rawMax = readTextFile(maxPath);
  bool ok = false;
  int maxVal = rawMax.toInt(&ok);
  if (ok && maxVal > 0) {
    return maxVal;
  }
  // Fallback mac dinh 255 neu khong co file pwm1_max.
  return 255;
}

int TufGamingFx705ge::readPwmValue(const QString &hwmonPath) const {
  const QString raw = readTextFile(hwmonPath + "/pwm1");
  bool ok = false;
  int val = raw.toInt(&ok);
  return ok ? val : 0;
}

bool TufGamingFx705ge::writePwmValue(const QString &hwmonPath, int pwmValue) const {
  QFile file(hwmonPath + "/pwm1");
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    return false;
  }
  const QByteArray data = QByteArray::number(pwmValue);
  const qint64 written = file.write(data);
  file.flush();
  return written == data.size();
}

bool TufGamingFx705ge::writePwmEnableManual(const QString &hwmonPath) const {
  QFile file(hwmonPath + "/pwm1_enable");
  if (!file.exists()) {
    return false;
  }
  if (!file.open(QIODevice::ReadWrite)) {
    return false;
  }
  const QString current = QString::fromUtf8(file.readAll()).trimmed();
  if (current == "2") {
    return true;  // Da o che do manual.
  }
  file.seek(0);
  const QByteArray data("2");
  const qint64 written = file.write(data);
  file.flush();
  return written == data.size();
}

QString TufGamingFx705ge::readTextFile(const QString &path) const {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return {};
  }
  const QByteArray data = file.readAll();
  return QString::fromUtf8(data).trimmed();
}

double TufGamingFx705ge::parseTempMilli(const QString &raw) const {
  bool ok = false;
  double val = raw.toDouble(&ok);
  if (!ok) {
    return 0.0;
  }
  // sysfs nhiet do thuong o don vi millidegree C.
  if (val > 200.0) {
    val /= 1000.0;
  }
  return val;
}
