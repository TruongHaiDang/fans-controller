#!/usr/bin/env bash
# ASUS TUF Fan & Sensor Capability Inspector (full check, safe syntax)
# Usage: sudo bash device-check/asus_fan_report.sh

set -euo pipefail

########################################
# Helpers: colors & formatting
########################################
if [ -t 1 ]; then
  bold="$(tput bold)"
  dim="$(tput dim)"
  red="$(tput setaf 1)"
  green="$(tput setaf 2)"
  yellow="$(tput setaf 3)"
  blue="$(tput setaf 4)"
  magenta="$(tput setaf 5)"
  cyan="$(tput setaf 6)"
  reset="$(tput sgr0)"
else
  bold=""; dim=""; red=""; green=""; yellow=""; blue=""; magenta=""; cyan=""; reset=""
fi

line() {
  printf '%s\n' "────────────────────────────────────────────────────────────"
}

section() {
  printf '\n%s%s%s\n' "$bold" "══════════ $1 ══════════" "$reset"
}

note() {
  printf '%s[NOTE]%s %s\n' "$cyan" "$reset" "$1"
}

warn() {
  printf '%s[WARN]%s %s\n' "$yellow" "$reset" "$1"
}

ok() {
  printf '%s[ OK ]%s %s\n' "$green" "$reset" "$1"
}

fail() {
  printf '%s[FAIL]%s %s\n' "$red" "$reset" "$1"
}

have_cmd() {
  command -v "$1" >/dev/null 2>&1
}

########################################
# Ensure root
########################################
if [ "${EUID:-$(id -u)}" -ne 0 ]; then
  echo "This script must run as root. Re-running with sudo..."
  exec sudo "$0" "$@"
fi

########################################
# Detect distro / package manager
########################################
PKG_MGR=""
DISTRO_ID=""
if [ -f /etc/os-release ]; then
  # shellcheck disable=SC1091
  . /etc/os-release
  DISTRO_ID="${ID:-}"
fi

if have_cmd apt-get; then
  PKG_MGR="apt"
elif have_cmd dnf; then
  PKG_MGR="dnf"
elif have_cmd yum; then
  PKG_MGR="yum"
elif have_cmd pacman; then
  PKG_MGR="pacman"
fi

########################################
# Install required packages
########################################
section "CÀI ĐẶT GÓI CẦN THIẾT"

PACKAGES_COMMON=("lm-sensors" "pciutils")
PACKAGES_FANCONTROL=("fancontrol")

install_packages_apt() {
  note "Distro: Debian/Ubuntu (apt)"
  apt-get update -y >/dev/null 2>&1 || true
  apt-get install -y "${PACKAGES_COMMON[@]}" "${PACKAGES_FANCONTROL[@]}" || true
}

install_packages_dnf() {
  note "Distro: Fedora/RHEL (dnf)"
  dnf install -y lm_sensors "${PACKAGES_FANCONTROL[@]}" pciutils || true
}

install_packages_yum() {
  note "Distro: RHEL/CentOS (yum)"
  yum install -y lm_sensors "${PACKAGES_FANCONTROL[@]}" pciutils || true
}

install_packages_pacman() {
  note "Distro: Arch (pacman)"
  pacman -Sy --noconfirm lm_sensors "${PACKAGES_FANCONTROL[@]}" pciutils || true
}

case "$PKG_MGR" in
  apt)    install_packages_apt ;;
  dnf)    install_packages_dnf ;;
  yum)    install_packages_yum ;;
  pacman) install_packages_pacman ;;
  *)
    warn "Không nhận diện được package manager. Bỏ qua bước cài gói tự động."
    ;;
esac

for cmd in sensors; do
  if have_cmd "$cmd"; then
    ok "Đã có lệnh: $cmd"
  else
    warn "Thiếu: $cmd (có thể cài thủ công = package 'lm-sensors')"
  fi
done

########################################
# Run sensors-detect (auto if available)
########################################
section "CẤU HÌNH LM-SENSORS"

if have_cmd sensors-detect; then
  note "Chạy sensors-detect ở chế độ tự động (có thể mất vài giây)..."
  # --auto không có ở tất cả bản, nên fallback sang yes | sensors-detect
  if sensors-detect --auto >/tmp/sensors-detect.log 2>&1; then
    ok "sensors-detect --auto chạy thành công."
  else
    warn "sensors-detect --auto không hỗ trợ. Thử yes | sensors-detect (có thể hỏi nhiều câu hỏi)."
    yes "" | sensors-detect >/tmp/sensors-detect.log 2>&1 || true
  fi
else
  warn "Không có sensors-detect, bỏ qua bước cấu hình."
fi

########################################
# Start building report
########################################
mkdir -p device-check 2>/dev/null || true
REPORT_FILE="device-check/asus_fan_report_$(date +%Y%m%d_%H%M%S).txt"
: > "$REPORT_FILE"  # clear

report() {
  printf '%s\n' "$1" | tee -a "$REPORT_FILE" >/dev/null
}

report_line() {
  line | tee -a "$REPORT_FILE" >/dev/null
}

########################################
# Section: System info
########################################
section "THÔNG TIN HỆ THỐNG"
report "ASUS TUF Fan & Sensor Capability Report"
report_line

HOSTNAME="$(hostname 2>/dev/null || echo unknown)"
KERNEL="$(uname -r 2>/dev/null || echo unknown)"
ARCH="$(uname -m 2>/dev/null || echo unknown)"
MODEL="$(cat /sys/class/dmi/id/product_name 2>/dev/null || echo unknown)"
VENDOR="$(cat /sys/class/dmi/id/sys_vendor 2>/dev/null || echo unknown)"

report "Host       : $HOSTNAME"
report "Vendor     : $VENDOR"
report "Model      : $MODEL"
report "Kernel     : $KERNEL"
report "Arch       : $ARCH"
report "Distro ID  : ${DISTRO_ID:-unknown}"

########################################
# Section: GPU info (PCI & modules)
########################################
section "GPU / VGA THÔNG TIN"

if have_cmd lspci; then
  report "Thiết bị VGA/3D theo lspci:"
  lspci -nn | grep -Ei 'VGA|3D' | tee -a "$REPORT_FILE" >/dev/null || true
else
  warn "Không có lspci (pciutils)."
fi

GPU_MODULES=$(lsmod 2>/dev/null | grep -E 'nvidia|nouveau|amdgpu|radeon|i915' || true)
if [ -n "$GPU_MODULES" ]; then
  report_line
  ok "Module GPU đã load:"
  printf '%s\n' "$GPU_MODULES" | tee -a "$REPORT_FILE" >/dev/null
else
  warn "Không thấy module GPU phổ biến (nvidia / amdgpu / radeon / i915 / nouveau)."
fi

########################################
# Section: Kernel modules (ASUS / EC)
########################################
section "KERNEL MODULES LIÊN QUAN ASUS / EC"

MODULES_ASUS=$(lsmod 2>/dev/null | grep -E 'asus(_nb_wmi|_wmi)?|ec_sys|asus_ec_sensors|asus-ec-sensors' || true)
if [ -n "$MODULES_ASUS" ]; then
  ok "Module liên quan ASUS / EC đã load:"
  printf '%s\n' "$MODULES_ASUS" | tee -a "$REPORT_FILE" >/dev/null
else
  warn "Không thấy module ASUS/EC đặc biệt trong lsmod (có thể vẫn ok)."
fi

########################################
# Section: hwmon devices
########################################
section "THIẾT BỊ HWMON (SENSOR & QUẠT)"

HWMON_BASE="/sys/class/hwmon"
CAN_TEMP="no"
CAN_FAN="no"
CAN_PWM="no"
FAN_COUNT=0
PWM_COUNT=0

if [ -d "$HWMON_BASE" ]; then
  for dev in "$HWMON_BASE"/hwmon*; do
    [ -e "$dev" ] || continue
    NAME_FILE="$dev/name"
    NAME="$(cat "$NAME_FILE" 2>/dev/null || echo unknown)"
    report_line
    report "[Device] $(basename "$dev")  (name: $NAME)"

    report "  Path: $dev"

    # Temperatures
    TEMP_FILES=$(ls "$dev"/temp*_* 2>/dev/null || true)
    if [ -n "$TEMP_FILES" ]; then
      CAN_TEMP="yes"
      for tf in $TEMP_FILES; do
        base="$(basename "$tf")"
        if [[ "$base" == temp*_input ]]; then
          label_file="${tf%_input}_label"
          label="$(cat "$label_file" 2>/dev/null || echo "$base")"
          val_raw="$(cat "$tf" 2>/dev/null || echo "N/A")"
          if [ "$val_raw" != "N/A" ] && [[ "$val_raw" =~ ^[0-9]+$ ]]; then
            val_c=$(awk "BEGIN { printf \"%.1f\", $val_raw/1000 }")
            report "  Temp: $label = ${val_c}°C"
          else
            report "  Temp: $label = $val_raw"
          fi
        fi
      done
    fi

    # Fans
    FAN_FILES=$(ls "$dev"/fan*_* 2>/dev/null || true)
    if [ -n "$FAN_FILES" ]; then
      for ff in $FAN_FILES; do
        base="$(basename "$ff")"
        if [[ "$base" == fan*_input ]]; then
          CAN_FAN="yes"
          FAN_COUNT=$((FAN_COUNT + 1))
          label_file="${ff%_input}_label"
          label="$(cat "$label_file" 2>/dev/null || echo "$base")"
          rpm="$(cat "$ff" 2>/dev/null || echo "N/A")"
          report "  Fan : $label = ${rpm} RPM"
        fi
      done
    fi

    # PWM
    PWM_FILES=$(ls "$dev"/pwm* 2>/dev/null || true)
    if [ -n "$PWM_FILES" ]; then
      for pf in $PWM_FILES; do
        base="$(basename "$pf")"
        if [[ "$base" == pwm* ]] && [[ "$base" != *_enable ]]; then
          CAN_PWM="yes"
          PWM_COUNT=$((PWM_COUNT + 1))
          val="$(cat "$pf" 2>/dev/null || echo "N/A")"
          writable="no"
          [ -w "$pf" ] && writable="yes"
          report "  PWM : $base = $val (writable: $writable)"

          enable_file="${pf}_enable"
          if [ -f "$enable_file" ]; then
            mode_val="$(cat "$enable_file" 2>/dev/null || echo "N/A")"
            mode_desc="unknown"
            case "$mode_val" in
              0) mode_desc="disabled (off / full speed)" ;;
              1) mode_desc="BIOS / auto" ;;
              2) mode_desc="manual (software control)" ;;
            esac
            report "        ${base}_enable = $mode_val ($mode_desc)"
          fi
        fi
      done
    fi
  done
else
  warn "Không tìm thấy /sys/class/hwmon – có thể kernel rất tối giản."
fi

########################################
# Section: ASUS platform devices
########################################
section "THIẾT BỊ ASUS TRONG /sys/devices/platform"

PLATFORM_ASUS=$(ls /sys/devices/platform 2>/dev/null | grep -i asus || true)
if [ -n "$PLATFORM_ASUS" ]; then
  ok "Tìm thấy thiết bị ASUS trong /sys/devices/platform:"
  printf '%s\n' "$PLATFORM_ASUS" | tee -a "$REPORT_FILE" >/dev/null

  for dev in $PLATFORM_ASUS; do
    path="/sys/devices/platform/$dev"
    hwmons=$(find "$path" -maxdepth 4 -type d -name 'hwmon*' 2>/dev/null || true)
    if [ -n "$hwmons" ]; then
      report_line
      report "[ASUS Platform: $dev]"
      for h in $hwmons; do
        report "  hwmon path: $h"
        ls "$h" 2>/dev/null | sed 's/^/    - /' | tee -a "$REPORT_FILE" >/dev/null
      done
    fi
  done
else
  warn "Không tìm thấy thiết bị ASUS rõ ràng trong /sys/devices/platform."
fi

########################################
# Section: Thermal zones (ACPI / SoC)
########################################
section "THERMAL ZONES (/sys/class/thermal)"

if [ -d /sys/class/thermal ]; then
  for tz in /sys/class/thermal/thermal_zone*; do
    [ -e "$tz" ] || continue
    tz_name="$(basename "$tz")"
    type_file="$tz/type"
    temp_file="$tz/temp"
    mode_file="$tz/mode"
    policy_file="$tz/policy"

    type_val="$(cat "$type_file" 2>/dev/null || echo unknown)"
    temp_raw="$(cat "$temp_file" 2>/dev/null || echo "N/A")"
    mode_val="$(cat "$mode_file" 2>/dev/null || echo "N/A")"
    policy_val="$(cat "$policy_file" 2>/dev/null || echo "N/A")"

    if [ "$temp_raw" != "N/A" ] && [[ "$temp_raw" =~ ^[0-9]+$ ]]; then
      temp_c=$(awk "BEGIN { printf \"%.1f\", $temp_raw/1000 }")
    else
      temp_c="$temp_raw"
    fi

    report_line
    report "[Thermal Zone] $tz_name"
    report "  Type   : $type_val"
    report "  Temp   : $temp_c °C"
    report "  Mode   : $mode_val"
    report "  Policy : $policy_val"
  done
else
  warn "Không tìm thấy /sys/class/thermal."
fi

########################################
# Section: GPU hwmon chi tiết (nếu có)
########################################
section "GPU HWMON CHI TIẾT"

if [ -d "$HWMON_BASE" ]; then
  found_gpu_hwmon="no"
  for dev in "$HWMON_BASE"/hwmon*; do
    [ -e "$dev" ] || continue
    NAME_FILE="$dev/name"
    NAME="$(cat "$NAME_FILE" 2>/dev/null || echo unknown)"
    case "$NAME" in
      nvidia|amdgpu|radeon|nouveau)
        found_gpu_hwmon="yes"
        report_line
        report "[GPU hwmon] $(basename "$dev") (name: $NAME)"
        report "  Path: $dev"

        # Temp
        for tf in "$dev"/temp*_*; do
          [ -e "$tf" ] || continue
          base="$(basename "$tf")"
          if [[ "$base" == temp*_input ]]; then
            label_file="${tf%_input}_label"
            label="$(cat "$label_file" 2>/dev/null || echo "$base")"
            val_raw="$(cat "$tf" 2>/dev/null || echo "N/A")"
            if [ "$val_raw" != "N/A" ] && [[ "$val_raw" =~ ^[0-9]+$ ]]; then
              val_c=$(awk "BEGIN { printf \"%.1f\", $val_raw/1000 }")
              report "  Temp: $label = ${val_c}°C"
            else
              report "  Temp: $label = $val_raw"
            fi
          fi
        done

        # Fan & PWM
        for ff in "$dev"/fan*_*; do
          [ -e "$ff" ] || continue
          base="$(basename "$ff")"
          if [[ "$base" == fan*_input ]]; then
            label_file="${ff%_input}_label"
            label="$(cat "$label_file" 2>/dev/null || echo "$base")"
            rpm="$(cat "$ff" 2>/dev/null || echo "N/A")"
            report "  Fan : $label = ${rpm} RPM"
          fi
        done

        for pf in "$dev"/pwm*; do
          [ -e "$pf" ] || continue
          base="$(basename "$pf")"
          if [[ "$base" == pwm* ]] && [[ "$base" != *_enable ]]; then
            val="$(cat "$pf" 2>/dev/null || echo "N/A")"
            writable="no"
            [ -w "$pf" ] && writable="yes"
            report "  PWM : $base = $val (writable: $writable)"
            enable_file="${pf}_enable"
            if [ -f "$enable_file" ]; then
              mode_val="$(cat "$enable_file" 2>/dev/null || echo "N/A")"
              mode_desc="unknown"
              case "$mode_val" in
                0) mode_desc="disabled (off / full speed)" ;;
                1) mode_desc="BIOS / auto" ;;
                2) mode_desc="manual (software control)" ;;
              esac
              report "        ${base}_enable = $mode_val ($mode_desc)"
            fi
          fi
        done
        ;;
    esac
  done

  if [ "$found_gpu_hwmon" = "no" ]; then
    warn "Không tìm thấy hwmon GPU (nvidia/amdgpu/radeon/nouveau)."
  fi
else
  warn "Không tìm thấy /sys/class/hwmon để kiểm tra GPU."
fi

########################################
# Section: Summary
########################################
section "TÓM TẮT KHẢ NĂNG MONITOR / CONTROL"

summary_line() {
  local label="$1"
  local value="$2"
  local desc="$3"
  if [ "$value" = "yes" ]; then
    printf "%s[YES]%s %-22s %s\n" "$green" "$reset" "$label" "$desc" | tee -a "$REPORT_FILE" >/dev/null
  else
    printf "%s[NO ]%s %-22s %s\n" "$red" "$reset" "$label" "$desc" | tee -a "$REPORT_FILE" >/dev/null
  fi
}

summary_line "Temperature sensors" "$CAN_TEMP" "Đọc nhiệt độ CPU/GPU/VRM qua hwmon"

temp_count="$(grep -c 'Temp:' "$REPORT_FILE" 2>/dev/null || echo 0)"
printf '  -> Tổng số temp*_input tìm thấy: %s\n' "$temp_count" | tee -a "$REPORT_FILE" >/dev/null

summary_line "Fan RPM sensors"    "$CAN_FAN"  "Đọc tốc độ quạt (RPM) qua hwmon"
printf '  -> Số fan*_input (quạt) tìm thấy: %s\n' "$FAN_COUNT" | tee -a "$REPORT_FILE" >/dev/null

summary_line "Fan PWM control"    "$CAN_PWM"  "Có file pwm* trong sysfs (có tiềm năng điều khiển quạt)"
printf '  -> Số kênh PWM tìm thấy: %s\n' "$PWM_COUNT" | tee -a "$REPORT_FILE" >/dev/null

report_line
report "Ghi chú:"
report "  - 'Fan PWM control = YES' nghĩa là có file pwm*; để điều khiển thực sự cần:"
report "      + 'pwmX_enable = 2' (manual) và"
report "      + quyền ghi (writable) vào pwmX."
report "  - Bạn có thể dùng thông tin này để viết chương trình C++ điều khiển quạt,"
report "    đọc nhiệt độ, fan RPM dựa trên các đường dẫn đã liệt kê."
report "  - Nếu máy có 2 quạt nhưng chỉ thấy 1 fan*_input, rất có thể 2 quạt dùng chung"
report "    1 kênh PWM / 1 cảm biến RPM hoặc fan GPU do driver riêng quản lý."

########################################
# Finished
########################################
section "HOÀN TẤT"
ok "Report đã được lưu vào: $REPORT_FILE"
note "Bạn có thể đính kèm file này khi hỏi thêm hoặc dùng làm input cho code C++."
