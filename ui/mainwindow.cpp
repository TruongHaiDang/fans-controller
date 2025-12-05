#include "mainwindow.h"

#include <QAbstractButton>
#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QRect>
#include <QScreen>
#include <QShowEvent>
#include <QScrollArea>
#include <QStringList>
#include <QSize>
#include <QSlider>
#include <QVBoxLayout>

#include <algorithm>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  // Cap nhat cache sensor truoc khi ve UI, sau do nap stylesheet.
  m_device.refreshSensors();
  buildUi();
  applyStyleSheet();
}

void MainWindow::buildUi() {
  // Thiet lap cua so co nen toi, kich co khac phuc va margin dong deu.
  setMinimumSize(1120, 890);
  QWidget *central = new QWidget(this);
  QVBoxLayout *rootLayout = new QVBoxLayout(central);
  rootLayout->setContentsMargins(20, 20, 20, 20);
  rootLayout->setSpacing(16);

  rootLayout->addWidget(createHeader());
  rootLayout->addWidget(createStatsRow());
  rootLayout->addWidget(createTrendAndDetailsRow());
  rootLayout->addWidget(createFanModeRow());
  rootLayout->addWidget(createFixedSpeedRow());
  rootLayout->addStretch(1);  // Day cac phan len tren, tao khoang thoang duoi.

  setCentralWidget(central);
}

QWidget *MainWindow::createHeader() {
  // Khu vuc tieu de tong, gom tieu de lon va mo ta ngan.
  QWidget *container = new QWidget(this);
  QVBoxLayout *layout = new QVBoxLayout(container);
  layout->setContentsMargins(0, 0, 0, 8);
  layout->setSpacing(6);
  layout->setAlignment(Qt::AlignHCenter);  // Can giua ca tieu de va phan mo ta.

  QLabel *title = new QLabel("Fan Monitoring & Control", container);
  title->setObjectName("titleLabel");
  title->setAlignment(Qt::AlignHCenter);

  QLabel *subtitle =
      new QLabel("Software used to control a laptop's fan.",
                 container);
  subtitle->setWordWrap(true);
  subtitle->setAlignment(Qt::AlignHCenter);
  subtitle->setMaximumWidth(820);  // Gioi han be ngang de chu khong trai dai.
  subtitle->setObjectName("subtitleLabel");

  layout->addWidget(title);
  layout->addWidget(subtitle);
  return container;
}

QWidget *MainWindow::createStatsRow() {
  // Hang gom ba the thong ke: CPU, Fan RPM, PCH.
  QWidget *row = new QWidget(this);
  QHBoxLayout *layout = new QHBoxLayout(row);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(12);

  // CPU Package
  const double cpuTemp = m_device.cpuPackageTempC();
  const QString cpuSeverity = temperatureSeverity(cpuTemp);
  layout->addWidget(createStatCard("CPU", "CPU Package", formatTemperature(cpuTemp),
                                   statusTextForSeverity(cpuSeverity), accentForStat(cpuSeverity)));

  // Fan RPM
  const auto fan = m_device.fan();
  layout->addWidget(createStatCard("FAN", "Fan RPM", QString::number(fan.rpm),
                                   "Status: Normal", "ok"));

  // PCH Temperature
  const double pchTemp = m_device.pchTempC();
  const QString pchSeverity = temperatureSeverity(pchTemp);
  layout->addWidget(createStatCard("PCH", "PCH Temperature", formatTemperature(pchTemp),
                                   statusTextForSeverity(pchSeverity), accentForStat(pchSeverity)));

  return row;
}

QFrame *MainWindow::createStatCard(const QString &iconText, const QString &title,
                                   const QString &valueText, const QString &statusText,
                                   const QString &accentProperty) {
  // Tao the thong ke voi icon, tieu de, gia tri lon va trang thai.
  QFrame *card = new QFrame(this);
  card->setObjectName("statCard");
  card->setProperty("accent", accentProperty);

  QVBoxLayout *layout = new QVBoxLayout(card);
  layout->setContentsMargins(14, 14, 14, 14);
  layout->setSpacing(8);

  QHBoxLayout *headerLayout = new QHBoxLayout();
  headerLayout->setSpacing(8);

  QLabel *icon = new QLabel(iconText, card);
  icon->setObjectName("statIcon");
  icon->setProperty("accent", accentProperty);
  icon->setAlignment(Qt::AlignCenter);

  QLabel *titleLabel = new QLabel(title, card);
  titleLabel->setObjectName("statTitle");

  headerLayout->addWidget(icon);
  headerLayout->addWidget(titleLabel, 1);

  QLabel *valueLabel = new QLabel(valueText, card);
  valueLabel->setObjectName("statValue");
  valueLabel->setProperty("accent", accentProperty);

  QLabel *statusLabel = new QLabel(statusText, card);
  statusLabel->setObjectName("statStatus");

  layout->addLayout(headerLayout);
  layout->addWidget(valueLabel);
  layout->addWidget(statusLabel);
  layout->addStretch(1);

  return card;
}

QWidget *MainWindow::createTrendAndDetailsRow() {
  // Hang giua gom bieu do nhiet do va danh sach chi tiet.
  QWidget *row = new QWidget(this);
  QHBoxLayout *layout = new QHBoxLayout(row);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(12);

  // The bieu do ben trai.
  QFrame *chartCard = new QFrame(row);
  chartCard->setObjectName("chartCard");
  QVBoxLayout *chartLayout = new QVBoxLayout(chartCard);
  chartLayout->setContentsMargins(14, 14, 14, 14);
  chartLayout->setSpacing(12);

  QLabel *chartTitle = new QLabel("Temperature Trend (CPU Package)", chartCard);
  chartTitle->setObjectName("sectionTitle");

  QFrame *chartArea = new QFrame(chartCard);
  chartArea->setObjectName("chartArea");
  QVBoxLayout *chartAreaLayout = new QVBoxLayout(chartArea);
  chartAreaLayout->setContentsMargins(0, 0, 0, 0);
  chartAreaLayout->setSpacing(0);

  QLabel *chartPlaceholder = new QLabel("Temperature Chart Area", chartArea);
  chartPlaceholder->setObjectName("placeholderLabel");
  chartPlaceholder->setAlignment(Qt::AlignCenter);
  chartAreaLayout->addWidget(chartPlaceholder);

  chartLayout->addWidget(chartTitle);
  chartLayout->addWidget(chartArea, 1);

  // The thong tin chi tiet ben phai.
  QFrame *detailCard = new QFrame(row);
  detailCard->setObjectName("detailCard");
  QVBoxLayout *detailLayout = new QVBoxLayout(detailCard);
  detailLayout->setContentsMargins(14, 14, 14, 14);
  detailLayout->setSpacing(10);

  QLabel *detailTitle = new QLabel("Detailed Readings", detailCard);
  detailTitle->setObjectName("sectionTitle");
  detailLayout->addWidget(detailTitle);

  // Scroll area de xem nhieu dong nhiet do neu danh sach dai.
  QScrollArea *scroll = new QScrollArea(detailCard);
  scroll->setWidgetResizable(true);
  scroll->setFrameShape(QFrame::NoFrame);
  scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  scroll->setObjectName("detailScroll");

  QWidget *scrollContent = new QWidget(scroll);
  scrollContent->setObjectName("detailList");
  scroll->viewport()->setObjectName("detailViewport");
  QVBoxLayout *listLayout = new QVBoxLayout(scrollContent);
  listLayout->setContentsMargins(0, 0, 0, 0);
  listLayout->setSpacing(10);

  const auto detailTemps = m_device.detailTemperatures();
  for (const auto &sample : detailTemps) {
    const QString severity = temperatureSeverity(sample.celsius);
    listLayout->addWidget(
        createDetailLine(sample.label, formatTemperature(sample.celsius), severity));
  }
  listLayout->addStretch(1);

  scrollContent->setLayout(listLayout);
  scroll->setWidget(scrollContent);
  detailLayout->addWidget(scroll);

  layout->addWidget(chartCard, 2);
  layout->addWidget(detailCard, 1);

  return row;
}

QWidget *MainWindow::createDetailLine(const QString &label, const QString &value,
                                      const QString &severityProperty) {
  // Tao mot dong thong tin voi nhan va vien mau hien thi muc do nhiet.
  QWidget *line = new QWidget(this);
  QHBoxLayout *layout = new QHBoxLayout(line);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(8);

  QLabel *labelWidget = new QLabel(label, line);
  labelWidget->setObjectName("detailLabel");

  QLabel *pill = new QLabel(value, line);
  pill->setObjectName("pill");
  pill->setProperty("severity", severityProperty);
  pill->setAlignment(Qt::AlignCenter);
  pill->setMinimumWidth(54);

  layout->addWidget(labelWidget, 1);
  layout->addWidget(pill);

  return line;
}

QWidget *MainWindow::createFanModeRow() {
  // Khu vuc chon che do quat (Silent, Performance, Turbo, Custom).
  QFrame *card = new QFrame(this);
  card->setObjectName("sectionCard");
  QVBoxLayout *layout = new QVBoxLayout(card);
  layout->setContentsMargins(14, 14, 14, 14);
  layout->setSpacing(12);

  QLabel *title = new QLabel("Fan Mode", card);
  title->setObjectName("sectionTitle");
  QLabel *subtitle = new QLabel("Select a preset fan mode for quick adjustments.", card);
  subtitle->setObjectName("sectionSubtitle");

  layout->addWidget(title);
  layout->addWidget(subtitle);

  QWidget *buttonRow = new QWidget(card);
  QHBoxLayout *buttonLayout = new QHBoxLayout(buttonRow);
  buttonLayout->setContentsMargins(0, 0, 0, 0);
  buttonLayout->setSpacing(10);

  m_modeGroup = new QButtonGroup(card);
  m_modeGroup->setExclusive(true);

  const QStringList modes = {"Silent", "Performance", "Turbo", "Custom"};
  int modeId = 0;
  for (const QString &mode : modes) {
    QPushButton *modeButton = new QPushButton(mode, buttonRow);
    modeButton->setCheckable(true);
    modeButton->setObjectName("modeButton");
    if (mode == "Performance") {
      modeButton->setChecked(true);  // Dat mac dinh theo hinh mau.
    }
    m_modeGroup->addButton(modeButton, modeId++);
    buttonLayout->addWidget(modeButton);
  }

  // Ap dung preset khi chon che do.
  connect(m_modeGroup, QOverload<int>::of(&QButtonGroup::idClicked), this,
          &MainWindow::handleModeSelected);

  layout->addWidget(buttonRow);
  return card;
}

QWidget *MainWindow::createFixedSpeedRow() {
  // Khu vuc dieu khien toc do co dinh bang slider.
  QFrame *card = new QFrame(this);
  card->setObjectName("sectionCard");
  QVBoxLayout *layout = new QVBoxLayout(card);
  layout->setContentsMargins(14, 14, 14, 14);
  layout->setSpacing(12);

  QWidget *header = new QWidget(card);
  QHBoxLayout *headerLayout = new QHBoxLayout(header);
  headerLayout->setContentsMargins(0, 0, 0, 0);
  headerLayout->setSpacing(6);

  QLabel *title = new QLabel("Fixed Speed", header);
  title->setObjectName("sectionTitle");

  m_fixedSpeedValueLabel = new QLabel("65%", header);
  m_fixedSpeedValueLabel->setObjectName("linkLabel");

  headerLayout->addWidget(title);
  headerLayout->addWidget(m_fixedSpeedValueLabel);
  headerLayout->addStretch(1);

  QLabel *subtitle = new QLabel("Manually set a fixed fan speed percentage.", card);
  subtitle->setObjectName("sectionSubtitle");

  QWidget *controlRow = new QWidget(card);
  QHBoxLayout *controlLayout = new QHBoxLayout(controlRow);
  controlLayout->setContentsMargins(0, 0, 0, 0);
  controlLayout->setSpacing(10);

  m_fixedSpeedSlider = new QSlider(Qt::Horizontal, controlRow);
  m_fixedSpeedSlider->setRange(0, 100);
  m_fixedSpeedSlider->setValue(m_device.fan().percent);
  m_fixedSpeedSlider->setObjectName("speedSlider");

  // Cap nhat nhan % khi keo slider.
  connect(m_fixedSpeedSlider, &QSlider::valueChanged, this, [this](int value) {
    if (m_fixedSpeedValueLabel) {
      m_fixedSpeedValueLabel->setText(QString::number(value) + "%");
    }
    if (!m_updatingFromPreset) {
      selectModeButton("Custom");
    }
  });

  QPushButton *applyBtn = new QPushButton("Apply", controlRow);
  applyBtn->setObjectName("primaryButton");

  controlLayout->addWidget(m_fixedSpeedSlider, 1);
  controlLayout->addWidget(applyBtn);

  // Ap dung gia tri slider hien tai xuong thiet bi.
  connect(applyBtn, &QPushButton::clicked, this, [this]() {
    if (!m_device.setFixedFanPercent(m_fixedSpeedSlider->value())) {
      qWarning("Khong the dat toc do quat co dinh");
      showPwmErrorDialog("Cannot set fixed fan speed");
    }
  });

  // Dat gia tri ban dau theo cache sensor.
  m_fixedSpeedValueLabel->setText(QString::number(m_device.fan().percent) + "%");
  syncModeButtonForPercent(m_fixedSpeedSlider->value());

  layout->addWidget(header);
  layout->addWidget(subtitle);
  layout->addWidget(controlRow);

  return card;
}

// Xu ly su kien nguoi dung chon mot preset o cum Fan Mode, dat PWM tuong ung
// va dong bo thanh slider.
void MainWindow::handleModeSelected(int buttonId) {
  QAbstractButton *button = m_modeGroup ? m_modeGroup->button(buttonId) : nullptr;
  if (!button) {
    return;
  }

  const QString modeName = button->text();
  int targetPercent = -1;
  if (modeName.compare("Silent", Qt::CaseInsensitive) == 0) {
    targetPercent = 0;
  } else if (modeName.compare("Performance", Qt::CaseInsensitive) == 0) {
    targetPercent = 50;
  } else if (modeName.compare("Turbo", Qt::CaseInsensitive) == 0) {
    targetPercent = 100;
  }

  if (targetPercent < 0) {
    return;  // Custom: khong ep gia tri slider hay PWM.
  }

  m_updatingFromPreset = true;
  if (m_fixedSpeedSlider) {
    m_fixedSpeedSlider->setValue(targetPercent);
  }
  m_updatingFromPreset = false;

  applyPresetPercent(targetPercent);
}

// Goi vao lop thiet bi de dat phan tram PWM, dong thoi cap nhat nhan hien thi.
void MainWindow::applyPresetPercent(int percent) {
  const int clamped = std::clamp(percent, 0, 100);
  const bool ok = m_device.setFixedFanPercent(clamped);
  if (!ok) {
    qWarning("Khong the dat toc do quat preset");
    showPwmErrorDialog("Cannot set fan preset");
  }

  if (m_fixedSpeedValueLabel) {
    m_fixedSpeedValueLabel->setText(QString::number(clamped) + "%");
  }
}

// Dat lai nut preset theo gia tri % hien co (0, 50, 100 thi map preset, khac se
// chon Custom).
void MainWindow::syncModeButtonForPercent(int percent) {
  if (percent == 0) {
    selectModeButton("Silent");
  } else if (percent == 50) {
    selectModeButton("Performance");
  } else if (percent == 100) {
    selectModeButton("Turbo");
  } else {
    selectModeButton("Custom");
  }
}

// Chon mot nut preset theo ten, chan phat sinh tin hieu khong mong muon tu
// QButtonGroup.
void MainWindow::selectModeButton(const QString &modeName) {
  if (!m_modeGroup) {
    return;
  }

  for (QAbstractButton *button : m_modeGroup->buttons()) {
    if (button && button->text().compare(modeName, Qt::CaseSensitive) == 0) {
      QSignalBlocker blocker(m_modeGroup);
      button->setChecked(true);
      return;
    }
  }
}

void MainWindow::applyStyleSheet() {
  // Thu nap file CSS tu cac duong dan kha nang nhat de ho tro chay trong build dir.
  const QString stylePath = resolveStylePath();
  QFile cssFile(stylePath);
  if (!cssFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qWarning("Khong the mo stylesheet: %s", qPrintable(stylePath));
    return;
  }

  const QString style = QString::fromUtf8(cssFile.readAll());
  this->setStyleSheet(style);
}

QString MainWindow::resolveStylePath() const {
  // Tim file stylesheet theo thu tu uu tien:
  // 1) Duong dan tu thu muc lam viec hien tai.
  // 2) Duong dan gan file thuc thi (khi chay tu build).
  // 3) Thu muc cha cua file thuc thi (khi binary nam trong build/bin).
  QStringList candidates;
  candidates << kStyleSheetPath;

  const QString appDir = QCoreApplication::applicationDirPath();
  candidates << QDir(appDir).filePath(kStyleSheetPath);
  candidates << QDir(appDir).filePath("../" + kStyleSheetPath);

  for (const QString &path : candidates) {
    if (QFile::exists(path)) {
      return path;
    }
  }

  // Neu khong tim thay, tra ve ten mac dinh de log canh bao.
  return kStyleSheetPath;
}

void MainWindow::centerOnScreen() {
  // Tinh toan vi tri trung tam man hinh hien tai va di chuyen cua so toi vi tri do.
  QScreen *screen = QGuiApplication::primaryScreen();
  if (!screen) {
    return;
  }

  const QRect available = screen->availableGeometry();
  const QSize winSize = size();

  const int x = available.x() + (available.width() - winSize.width()) / 2;
  const int y = available.y() + (available.height() - winSize.height()) / 2;
  move(x, y);
}

void MainWindow::showEvent(QShowEvent *event) {
  // Chi can giua cua so ngay lan hien dau tien de tranh tinh toan nhieu lan.
  QMainWindow::showEvent(event);
  if (!m_hasCentered) {
    centerOnScreen();
    m_hasCentered = true;
  }
}

QString MainWindow::formatTemperature(double tempC) const {
  // Dinh dang nhiet do lam tron va them ky hieu do.
  return QString::number(qRound(tempC)) + QChar(0x00B0) + "C";
}

QString MainWindow::temperatureSeverity(double tempC) const {
  // Phan loai nhiet do de to mau UI.
  if (tempC >= 70.0) {
    return "warning";
  }
  if (tempC >= 55.0) {
    return "caution";
  }
  if (tempC >= 40.0) {
    return "info";
  }
  return "cool";
}

QString MainWindow::statusTextForSeverity(const QString &severity) const {
  // Tra ve chu thich trang thai tu muc do nhiet.
  if (severity == "warning") {
    return "Status: High";
  }
  if (severity == "caution") {
    return "Status: Elevated";
  }
  if (severity == "info") {
    return "Status: Normal";
  }
  return "Status: Cool";
}

QString MainWindow::accentForStat(const QString &severity) const {
  // Map severity sang accent de tiep tuc dung style card san co.
  if (severity == "warning" || severity == "caution") {
    return "warning";
  }
  if (severity == "info") {
    return "info";
  }
  return "ok";
}

void MainWindow::showPwmErrorDialog(const QString &title) {
  // Thong bao loi PWM chi mot lan de tranh lam phien nguoi dung.
  if (m_shownPwmErrorDialog) {
    return;
  }
  m_shownPwmErrorDialog = true;

  const QString detail =
      m_device.lastError().isEmpty()
          ? "Kiem tra quyen truy cap /sys/class/hwmon/*/pwm1 va pwm1_enable (can sudo/root)."
          : m_device.lastError();

  QMessageBox::warning(
      this, title,
      QString("Khong the ghi gia tri PWM cho quat.\n\nLy do: %1\n\nThu chay ung dung "
              "voi quyen root/sudo hoac dat permission/phc cap quyen ghi vao pwm1.")
          .arg(detail));
}
