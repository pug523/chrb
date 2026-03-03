// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "core/check.h"

namespace core {

// base styles
enum class Style : uint8_t {
  Reset = 0,
  Bold = 1 << 0,       // 1
  Dim = 1 << 1,        // 2
  Italic = 1 << 2,     // 4
  Underline = 1 << 3,  // 8
  Blink = 1 << 4,      // 16
  Reverse = 1 << 5,    // 32
  Hidden = 1 << 6,     // 64
  Strike = 1 << 7,     // 128

  // alias
  BoldUnderline = Bold | Underline,
  BoldItalic = Bold | Italic,

  Default = Reset,
};

constexpr const char* kReset = "\033[0m";
constexpr const char* kBold = "\033[1m";
constexpr const char* kDim = "\033[2m";
constexpr const char* kItalic = "\033[3m";
constexpr const char* kUnderline = "\033[4m";
constexpr const char* kBlink = "\033[5m";
constexpr const char* kReverse = "\033[7m";
constexpr const char* kHidden = "\033[8m";
constexpr const char* kStrike = "\033[9m";

inline constexpr std::array<const char*, 8> kStyleCodes = {
    kBold, kDim, kItalic, kUnderline, kBlink, kReverse, kHidden, kStrike,
};

// foreground colors
enum class Color : uint8_t {
  Black = 0,
  Red = 1,
  Green = 2,
  Yellow = 3,
  Blue = 4,
  Magenta = 5,
  Cyan = 6,
  White = 7,
  Gray = 8,
  BrightRed = 9,
  BrightGreen = 10,
  BrightYellow = 11,
  BrightBlue = 12,
  BrightMagenta = 13,
  BrightCyan = 14,
  BrightWhite = 15,

  Default = 255,
};

constexpr const char* kBlack = "\033[30m";
constexpr const char* kRed = "\033[31m";
constexpr const char* kGreen = "\033[32m";
constexpr const char* kYellow = "\033[33m";
constexpr const char* kBlue = "\033[34m";
constexpr const char* kMagenta = "\033[35m";
constexpr const char* kCyan = "\033[36m";
constexpr const char* kWhite = "\033[37m";
constexpr const char* kGray = "\033[90m";
constexpr const char* kBrightRed = "\033[91m";
constexpr const char* kBrightGreen = "\033[92m";
constexpr const char* kBrightYellow = "\033[93m";
constexpr const char* kBrightBlue = "\033[94m";
constexpr const char* kBrightMagenta = "\033[95m";
constexpr const char* kBrightCyan = "\033[96m";
constexpr const char* kBrightWhite = "\033[97m";

inline constexpr std::array<const char*, 16> kColorCodes = {
    kBlack,      kRed,           kGreen,       kYellow,
    kBlue,       kMagenta,       kCyan,        kWhite,
    kGray,       kBrightRed,     kBrightGreen, kBrightYellow,
    kBrightBlue, kBrightMagenta, kBrightCyan,  kBrightWhite,
};

// background colors
enum class BgColor : uint8_t {
  Black = 0,
  Red = 1,
  Green = 2,
  Yellow = 3,
  Blue = 4,
  Magenta = 5,
  Cyan = 6,
  White = 7,
  Gray = 8,
  BrightRed = 9,
  BrightGreen = 10,
  BrightYellow = 11,
  BrightBlue = 12,
  BrightMagenta = 13,
  BrightCyan = 14,
  BrightWhite = 15,

  Default = 255,
};

constexpr const char* kBgBlack = "\033[40m";
constexpr const char* kBgRed = "\033[41m";
constexpr const char* kBgGreen = "\033[42m";
constexpr const char* kBgYellow = "\033[43m";
constexpr const char* kBgBlue = "\033[44m";
constexpr const char* kBgMagenta = "\033[45m";
constexpr const char* kBgCyan = "\033[46m";
constexpr const char* kBgWhite = "\033[47m";
constexpr const char* kBgGray = "\033[100m";
constexpr const char* kBgBrightRed = "\033[101m";
constexpr const char* kBgBrightGreen = "\033[102m";
constexpr const char* kBgBrightYellow = "\033[103m";
constexpr const char* kBgBrightBlue = "\033[104m";
constexpr const char* kBgBrightMagenta = "\033[105m";
constexpr const char* kBgBrightCyan = "\033[106m";
constexpr const char* kBgBrightWhite = "\033[107m";

inline constexpr std::array<const char*, 16> kBgColorCodes = {
    kBgBlack,      kBgRed,           kBgGreen,       kBgYellow,
    kBgBlue,       kBgMagenta,       kBgCyan,        kBgWhite,
    kBgGray,       kBgBrightRed,     kBgBrightGreen, kBgBrightYellow,
    kBgBrightBlue, kBgBrightMagenta, kBgBrightCyan,  kBgBrightWhite,
};

constexpr const char* kFgRgbPrefix = "\033[38;2;";
constexpr const char* kBgRgbPrefix = "\033[48;2;";
constexpr const char kRgbSuffix = 'm';
constexpr const char kSemicolon = ';';

constexpr const std::size_t kStyleCodeLength = 4;
constexpr const std::size_t kResetCodeLength = kStyleCodeLength;
constexpr const std::size_t kRgbCodeLength = 20;

inline constexpr const char* style_str(Style style) {
  dcheck(static_cast<uint8_t>(style) <= static_cast<uint8_t>(Style::Strike));
  if (style == Style::Reset) {
    return kReset;
  }
  return kStyleCodes[static_cast<uint8_t>(style)];
}

inline constexpr const char* color_str(Color color) {
  if (color == Color::Default) {
    return kReset;
  }
  return kColorCodes[static_cast<uint8_t>(color)];
}

inline constexpr const char* color_str(BgColor bg_color) {
  if (bg_color == BgColor::Default) {
    return kReset;
  }
  return kBgColorCodes[static_cast<uint8_t>(bg_color)];
}

}  // namespace core
