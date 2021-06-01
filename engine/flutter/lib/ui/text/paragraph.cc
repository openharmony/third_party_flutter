// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/text/paragraph.h"

#include "flutter/common/settings.h"
#include "flutter/common/task_runners.h"
#include "flutter/fml/logging.h"
#include "flutter/fml/task_runner.h"

namespace flutter {

IMPLEMENT_WRAPPERTYPEINFO(ui, Paragraph);

#define FOR_EACH_BINDING(V)             \
  V(Paragraph, width)                   \
  V(Paragraph, height)                  \
  V(Paragraph, longestLine)             \
  V(Paragraph, minIntrinsicWidth)       \
  V(Paragraph, maxIntrinsicWidth)       \
  V(Paragraph, alphabeticBaseline)      \
  V(Paragraph, ideographicBaseline)     \
  V(Paragraph, didExceedMaxLines)       \
  V(Paragraph, layout)                  \
  V(Paragraph, paint)                   \
  V(Paragraph, getWordBoundary)         \
  V(Paragraph, getRectsForRange)        \
  V(Paragraph, getRectsForPlaceholders) \
  V(Paragraph, getPositionForOffset)

DART_BIND_ALL(Paragraph, FOR_EACH_BINDING)

Paragraph::Paragraph(std::unique_ptr<txt::Paragraph> paragraph)
    : m_paragraph(std::move(paragraph)) {}

Paragraph::~Paragraph() = default;

size_t Paragraph::GetAllocationSize() {
  // We don't have an accurate accounting of the paragraph's memory consumption,
  // so return a fixed size to indicate that its impact is more than the size
  // of the Paragraph class.
  return 2000;
}

double Paragraph::width() {
  return m_paragraph->GetMaxWidth();
}

double Paragraph::height() {
  return m_paragraph->GetHeight();
}

double Paragraph::longestLine() {
  return m_paragraph->GetLongestLine();
}

double Paragraph::minIntrinsicWidth() {
  return m_paragraph->GetMinIntrinsicWidth();
}

double Paragraph::maxIntrinsicWidth() {
  return m_paragraph->GetMaxIntrinsicWidth();
}

double Paragraph::alphabeticBaseline() {
  return m_paragraph->GetAlphabeticBaseline();
}

double Paragraph::ideographicBaseline() {
  return m_paragraph->GetIdeographicBaseline();
}

bool Paragraph::didExceedMaxLines() {
  return m_paragraph->DidExceedMaxLines();
}

void Paragraph::layout(double width) {
  m_paragraph->Layout(width);
}

void Paragraph::paint(Canvas* canvas, double x, double y) {
  SkCanvas* sk_canvas = canvas->canvas();
  if (!sk_canvas)
    return;
  m_paragraph->Paint(sk_canvas, x, y);
}

std::vector<TextBox> Paragraph::getRectsForRange(unsigned start,
                                                 unsigned end,
                                                 unsigned boxHeightStyle,
                                                 unsigned boxWidthStyle) {
  std::vector<TextBox> result;
  std::vector<txt::Paragraph::TextBox> boxes = m_paragraph->GetRectsForRange(
      start, end, static_cast<txt::Paragraph::RectHeightStyle>(boxHeightStyle),
      static_cast<txt::Paragraph::RectWidthStyle>(boxWidthStyle));
  for (const txt::Paragraph::TextBox& box : boxes) {
    result.emplace_back(box.rect, static_cast<TextDirection>(box.direction));
  }
  return result;
}

std::vector<TextBox> Paragraph::getRectsForPlaceholders() {
  std::vector<TextBox> result;
  std::vector<txt::Paragraph::TextBox> boxes =
      m_paragraph->GetRectsForPlaceholders();
  for (const txt::Paragraph::TextBox& box : boxes) {
    result.emplace_back(box.rect, static_cast<TextDirection>(box.direction));
  }
  return result;
}

Dart_Handle Paragraph::getPositionForOffset(double dx, double dy) {
  return nullptr;
}

Dart_Handle Paragraph::getWordBoundary(unsigned offset) {
  return nullptr;
}

}  // namespace flutter
