#include "mainwindow.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRect>
#include <QScreen>
#include <QShowEvent>
#include <QStringList>
#include <QSize>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  // Tao UI va nap stylesheet ngay khi khoi tao.
  buildUi();
  applyStyleSheet();
}

void MainWindow::buildUi() {
  // Thiet lap cua so co nen toi, kich co khac phuc va margin dong deu.
  setMinimumSize(1120, 960);
  QWidget *central = new QWidget(this);
  QVBoxLayout *rootLayout = new QVBoxLayout(central);
  rootLayout->setContentsMargins(20, 20, 20, 20);
  rootLayout->setSpacing(16);

  rootLayout->addWidget(createHeader());
  rootLayout->addWidget(createStatsRow());
  rootLayout->addWidget(createTrendAndDetailsRow());
  rootLayout->addWidget(createFanModeRow());
  rootLayout->addWidget(createProfilesRow());
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

  QLabel *title = new QLabel("Fan Monitoring & Control", container);
  title->setObjectName("titleLabel");
  title->setAlignment(Qt::AlignHCenter);

  QLabel *subtitle =
      new QLabel("A unified dashboard for real-time sensor data and comprehensive fan control for your ASUS TUF laptop.",
                 container);
  subtitle->setWordWrap(true);
  subtitle->setAlignment(Qt::AlignHCenter);
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

  layout->addWidget(createStatCard("CPU", "CPU Package", "68 C", "Status: High", "warning"));
  layout->addWidget(createStatCard("FAN", "Fan RPM", "2450", "Status: Normal", "ok"));
  layout->addWidget(createStatCard("PCH", "PCH Temperature", "42 C", "Status: Normal", "info"));

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

  detailLayout->addWidget(createDetailLine("CPU Core 1", "67 C", "warning"));
  detailLayout->addWidget(createDetailLine("CPU Core 2", "69 C", "warning"));
  detailLayout->addWidget(createDetailLine("NVMe Drive", "38 C", "cool"));
  detailLayout->addWidget(createDetailLine("PCH", "42 C", "info"));
  detailLayout->addWidget(createDetailLine("ACPI Zone 1", "55 C", "caution"));
  detailLayout->addWidget(createDetailLine("ACPI Zone 2", "45 C", "info"));
  detailLayout->addStretch(1);

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
  for (const QString &mode : modes) {
    QPushButton *modeButton = new QPushButton(mode, buttonRow);
    modeButton->setCheckable(true);
    modeButton->setObjectName("modeButton");
    if (mode == "Performance") {
      modeButton->setChecked(true);  // Dat mac dinh theo hinh mau.
    }
    m_modeGroup->addButton(modeButton);
    buttonLayout->addWidget(modeButton);
  }

  layout->addWidget(buttonRow);
  return card;
}

QWidget *MainWindow::createProfilesRow() {
  // Khu vuc quan ly profile luu san va nut thao tac.
  QFrame *card = new QFrame(this);
  card->setObjectName("sectionCard");
  QVBoxLayout *layout = new QVBoxLayout(card);
  layout->setContentsMargins(14, 14, 14, 14);
  layout->setSpacing(12);

  QLabel *title = new QLabel("Fan Profiles", card);
  title->setObjectName("sectionTitle");
  QLabel *subtitle = new QLabel("Save, load, and manage your custom fan settings.", card);
  subtitle->setObjectName("sectionSubtitle");

  layout->addWidget(title);
  layout->addWidget(subtitle);

  QWidget *row = new QWidget(card);
  QHBoxLayout *rowLayout = new QHBoxLayout(row);
  rowLayout->setContentsMargins(0, 0, 0, 0);
  rowLayout->setSpacing(10);

  QLabel *selectLabel = new QLabel("Select Profile", row);
  selectLabel->setObjectName("inputLabel");

  QComboBox *combo = new QComboBox(row);
  combo->addItems({"Gaming*", "Silent", "Performance"});
  combo->setObjectName("profileCombo");

  // Nhom nut thao tac: Apply, Save, Delete.
  QPushButton *applyBtn = new QPushButton("Apply", row);
  applyBtn->setObjectName("primaryButton");

  QPushButton *saveBtn = new QPushButton("Save", row);
  saveBtn->setObjectName("ghostButton");

  QPushButton *deleteBtn = new QPushButton("Delete", row);
  deleteBtn->setObjectName("ghostButton");

  rowLayout->addWidget(selectLabel);
  rowLayout->addWidget(combo, 1);
  rowLayout->addStretch(1);
  rowLayout->addWidget(saveBtn);
  rowLayout->addWidget(deleteBtn);
  rowLayout->addWidget(applyBtn);

  layout->addWidget(row);
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
  m_fixedSpeedSlider->setValue(65);
  m_fixedSpeedSlider->setObjectName("speedSlider");

  // Cap nhat nhan % khi keo slider.
  connect(m_fixedSpeedSlider, &QSlider::valueChanged, this, [this](int value) {
    if (m_fixedSpeedValueLabel) {
      m_fixedSpeedValueLabel->setText(QString::number(value) + "%");
    }
  });

  QPushButton *applyBtn = new QPushButton("Apply", controlRow);
  applyBtn->setObjectName("primaryButton");

  controlLayout->addWidget(m_fixedSpeedSlider, 1);
  controlLayout->addWidget(applyBtn);

  layout->addWidget(header);
  layout->addWidget(subtitle);
  layout->addWidget(controlRow);

  return card;
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
