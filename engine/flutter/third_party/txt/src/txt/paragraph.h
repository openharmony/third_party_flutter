/*
 * Copyright 2017 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LIB_TXT_SRC_PARAGRAPH_H_
#define LIB_TXT_SRC_PARAGRAPH_H_

#include "paragraph_style.h"

class SkCanvas;

namespace txt {

// Interface for text layout engines.  The original implementation was based on
// the Minikin text layout library used by Android.  Another implementation is
// available based on Skia's SkShaper/SkParagraph text layout module.
class Paragraph {
 public:
  enum Affinity { UPSTREAM, DOWNSTREAM };

  // Options for various types of bounding boxes provided by
  // GetRectsForRange(...).
  enum class RectHeightStyle {
    // Provide tight bounding boxes that fit heights per run.
    kTight,

    // The height of the boxes will be the maximum height of all runs in the
    // line. All rects in the same line will be the same height.
    kMax,

    // Extends the top and/or bottom edge of the bounds to fully cover any line
    // spacing. The top edge of each line should be the same as the bottom edge
    // of the line above. There should be no gaps in vertical coverage given any
    // ParagraphStyle line_height.
    //
    // The top and bottom of each rect will cover half of the
    // space above and half of the space below the line.
    kIncludeLineSpacingMiddle,
    // The line spacing will be added to the top of the rect.
    kIncludeLineSpacingTop,
    // The line spacing will be added to the bottom of the rect.
    kIncludeLineSpacingBottom,

    // Calculate boxes based on the strut's metrics.
    kStrut
  };

  enum class RectWidthStyle {
    // Provide tight bounding boxes that fit widths to the runs of each line
    // independently.
    kTight,

    // Extends the width of the last rect of each line to match the position of
    // the widest rect over all the lines.
    kMax
  };

  struct PositionWithAffinity {
    const size_t position;
    const Affinity affinity;

    PositionWithAffinity(size_t p, Affinity a) : position(p), affinity(a) {}
  };

  struct TextBox {
    SkRect rect;
    TextDirection direction;

    TextBox(SkRect r, TextDirection d) : rect(r), direction(d) {}
  };

  template <typename T>
  struct Range {
    Range() : start(), end() {}
    Range(T s, T e) : start(s), end(e) {}

    T start, end;

    bool operator==(const Range<T>& other) const {
      return start == other.start && end == other.end;
    }

    T width() const { return end - start; }

    void Shift(T delta) {
      start += delta;
      end += delta;
    }
  };

  virtual ~Paragraph() = default;

  // Returns the width provided in the Layout() method. This is the maximum
  // width any line in the laid out paragraph can occupy. We expect that
  // GetMaxWidth() >= GetLayoutWidth().
  virtual double GetMaxWidth() = 0;

  // Returns the height of the laid out paragraph. NOTE this is not a tight
  // bounding height of the glyphs, as some glyphs do not reach as low as they
  // can.
  virtual double GetHeight() = 0;

  // Returns the width of the longest line as found in Layout(), which is
  // defined as the horizontal distance from the left edge of the leftmost glyph
  // to the right edge of the rightmost glyph. We expect that
  // GetLongestLine() <= GetMaxWidth().
  virtual double GetLongestLine() = 0;

  // Returns the actual max width of the longest line after Layout().
  virtual double GetMinIntrinsicWidth() = 0;

  // Returns the total width covered by the paragraph without linebreaking.
  virtual double GetMaxIntrinsicWidth() = 0;

  // Distance from top of paragraph to the Alphabetic baseline of the first
  // line. Used for alphabetic fonts (A-Z, a-z, greek, etc.)
  virtual double GetAlphabeticBaseline() = 0;

  // Distance from top of paragraph to the Ideographic baseline of the first
  // line. Used for ideographic fonts (Chinese, Japanese, Korean, etc.)
  virtual double GetIdeographicBaseline() = 0;

  // Checks if the layout extends past the maximum lines and had to be
  // truncated.
  virtual bool DidExceedMaxLines() = 0;

  // Layout calculates the positioning of all the glyphs. Must call this method
  // before Painting and getting any statistics from this class.
  virtual void Layout(double width) = 0;

  // Paints the laid out text onto the supplied SkCanvas at (x, y) offset from
  // the origin. Only valid after Layout() is called.
  virtual void Paint(SkCanvas* canvas, double x, double y) = 0;

  // Returns a vector of bounding boxes that enclose all text between start and
  // end glyph indexes, including start and excluding end.
  virtual std::vector<TextBox> GetRectsForRange(
      size_t start,
      size_t end,
      RectHeightStyle rect_height_style,
      RectWidthStyle rect_width_style) = 0;

  // Returns a vector of bounding boxes that bound all inline placeholders in
  // the paragraph.
  //
  // There will be one box for each inline placeholder. The boxes will be in the
  // same order as they were added to the paragraph. The bounds will always be
  // tight and should fully enclose the area where the placeholder should be.
  //
  // More granular boxes may be obtained through GetRectsForRange, which will
  // return bounds on both text as well as inline placeholders.
  virtual std::vector<TextBox> GetRectsForPlaceholders() = 0;

  // Returns the index of the glyph that corresponds to the provided coordinate,
  // with the top left corner as the origin, and +y direction as down.
  virtual PositionWithAffinity GetGlyphPositionAtCoordinate(double dx,
                                                            double dy) = 0;

  virtual PositionWithAffinity GetGlyphPositionAtCoordinateWithCluster(double dx,
                                                                       double dy) = 0;

  // Finds the first and last glyphs that define a word containing the glyph at
  // index offset.
  virtual Range<size_t> GetWordBoundary(size_t offset) = 0;

  virtual size_t GetLineCount() = 0;

  virtual double GetLineHeight(int lineNumber) = 0;

  virtual double GetLineWidth(int lineNumber) = 0;
};

}  // namespace txt

#endif  // LIB_TXT_SRC_PARAGRAPH_H_
