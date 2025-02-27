/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTextBlob_DEFINED
#define SkTextBlob_DEFINED

#include <string>
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/private/SkTemplates.h"

#include <atomic>

struct SkRSXform;
struct SkSerialProcs;
struct SkDeserialProcs;

/** \class SkTextBlob
    SkTextBlob combines multiple text runs into an immutable container. Each text
    run consists of glyphs, SkPaint, and position. Only parts of SkPaint related to
    fonts and text rendering are used by run.
*/
class SK_API SkTextBlob final : public SkNVRefCnt<SkTextBlob> {
public:

    /** Returns conservative bounding box. Uses SkPaint associated with each glyph to
        determine glyph bounds, and unions all bounds. Returned bounds may be
        larger than the bounds of all glyphs in runs.

        @return  conservative bounding box
    */
    const SkRect& bounds() const { return fBounds; }

    /** Returns a non-zero value unique among all text blobs.

        @return  identifier for SkTextBlob
    */
    uint32_t uniqueID() const { return fUniqueID; }

    /** Returns the number of intervals that intersect bounds.
        bounds describes a pair of lines parallel to the text advance.
        The return count is zero or a multiple of two, and is at most twice the number of glyphs in
        the the blob.

        Pass nullptr for intervals to determine the size of the interval array.

        Runs within the blob that contain SkRSXform are ignored when computing intercepts.

        @param bounds     lower and upper line parallel to the advance
        @param intervals  returned intersections; may be nullptr
        @param paint      specifies stroking, SkPathEffect that affects the result; may be nullptr
        @return           number of intersections; may be zero
     */
    int getIntercepts(const SkScalar bounds[2], SkScalar intervals[],
                      const SkPaint* paint = nullptr) const;

    /** Creates SkTextBlob with a single run.

        font contains attributes used to define the run text.

        When encoding is SkTextEncoding::kUTF8, SkTextEncoding::kUTF16, or
        SkTextEncoding::kUTF32, this function uses the default
        character-to-glyph mapping from the SkTypeface in font.  It does not
        perform typeface fallback for characters not found in the SkTypeface.
        It does not perform kerning or other complex shaping; glyphs are
        positioned based on their default advances.

        @param text        character code points or glyphs drawn
        @param byteLength  byte length of text array
        @param font        text size, typeface, text scale, and so on, used to draw
        @param encoding    text encoding used in the text array
        @return            SkTextBlob constructed from one run
    */
    static sk_sp<SkTextBlob> MakeFromText(const void* text, size_t byteLength, const SkFont& font,
                                          SkTextEncoding encoding = SkTextEncoding::kUTF8);

    /** Creates SkTextBlob with a single run. string meaning depends on SkTextEncoding;
        by default, string is encoded as UTF-8.

        font contains attributes used to define the run text.

        When encoding is SkTextEncoding::kUTF8, SkTextEncoding::kUTF16, or
        SkTextEncoding::kUTF32, this function uses the default
        character-to-glyph mapping from the SkTypeface in font.  It does not
        perform typeface fallback for characters not found in the SkTypeface.
        It does not perform kerning or other complex shaping; glyphs are
        positioned based on their default advances.

        @param string   character code points or glyphs drawn
        @param font     text size, typeface, text scale, and so on, used to draw
        @param encoding text encoding used in the text array
        @return         SkTextBlob constructed from one run
    */
    static sk_sp<SkTextBlob> MakeFromString(const char* string, const SkFont& font,
                                            SkTextEncoding encoding = SkTextEncoding::kUTF8) {
        if (!string) {
            return nullptr;
        }
        return MakeFromText(string, strlen(string), font, encoding);
    }

    /** Returns a textblob built from a single run of text with x-positions and a single y value.
        This is equivalent to using SkTextBlobBuilder and calling allocRunPosH().
        Returns nullptr if byteLength is zero.

        @param text        character code points or glyphs drawn (based on encoding)
        @param byteLength  byte length of text array
        @param xpos    array of x-positions, must contain values for all of the character points.
        @param constY  shared y-position for each character point, to be paired with each xpos.
        @param font    SkFont used for this run
        @param encoding specifies the encoding of the text array.
        @return        new textblob or nullptr
     */
    static sk_sp<SkTextBlob> MakeFromPosTextH(const void* text, size_t byteLength,
                                      const SkScalar xpos[], SkScalar constY, const SkFont& font,
                                      SkTextEncoding encoding = SkTextEncoding::kUTF8);

    /** Returns a textblob built from a single run of text with positions.
        This is equivalent to using SkTextBlobBuilder and calling allocRunPos().
        Returns nullptr if byteLength is zero.

        @param text        character code points or glyphs drawn (based on encoding)
        @param byteLength  byte length of text array
        @param pos     array of positions, must contain values for all of the character points.
        @param font    SkFont used for this run
        @param encoding specifies the encoding of the text array.
        @return        new textblob or nullptr
     */
    static sk_sp<SkTextBlob> MakeFromPosText(const void* text, size_t byteLength,
                                             const SkPoint pos[], const SkFont& font,
                                             SkTextEncoding encoding = SkTextEncoding::kUTF8);

    static sk_sp<SkTextBlob> MakeFromRSXform(const void* text, size_t byteLength,
                                             const SkRSXform xform[], const SkFont& font,
                                             SkTextEncoding encoding = SkTextEncoding::kUTF8);

    /** Writes data to allow later reconstruction of SkTextBlob. memory points to storage
        to receive the encoded data, and memory_size describes the size of storage.
        Returns bytes used if provided storage is large enough to hold all data;
        otherwise, returns zero.

        procs.fTypefaceProc permits supplying a custom function to encode SkTypeface.
        If procs.fTypefaceProc is nullptr, default encoding is used. procs.fTypefaceCtx
        may be used to provide user context to procs.fTypefaceProc; procs.fTypefaceProc
        is called with a pointer to SkTypeface and user context.

        @param procs       custom serial data encoders; may be nullptr
        @param memory      storage for data
        @param memory_size size of storage
        @return            bytes written, or zero if required storage is larger than memory_size
    */
    size_t serialize(const SkSerialProcs& procs, void* memory, size_t memory_size) const;

    /** Returns storage containing SkData describing SkTextBlob, using optional custom
        encoders.

        procs.fTypefaceProc permits supplying a custom function to encode SkTypeface.
        If procs.fTypefaceProc is nullptr, default encoding is used. procs.fTypefaceCtx
        may be used to provide user context to procs.fTypefaceProc; procs.fTypefaceProc
        is called with a pointer to SkTypeface and user context.

        @param procs  custom serial data encoders; may be nullptr
        @return       storage containing serialized SkTextBlob
    */
    sk_sp<SkData> serialize(const SkSerialProcs& procs) const;

    /** Recreates SkTextBlob that was serialized into data. Returns constructed SkTextBlob
        if successful; otherwise, returns nullptr. Fails if size is smaller than
        required data length, or if data does not permit constructing valid SkTextBlob.

        procs.fTypefaceProc permits supplying a custom function to decode SkTypeface.
        If procs.fTypefaceProc is nullptr, default decoding is used. procs.fTypefaceCtx
        may be used to provide user context to procs.fTypefaceProc; procs.fTypefaceProc
        is called with a pointer to SkTypeface data, data byte length, and user context.

        @param data   pointer for serial data
        @param size   size of data
        @param procs  custom serial data decoders; may be nullptr
        @return       SkTextBlob constructed from data in memory
    */
    static sk_sp<SkTextBlob> Deserialize(const void* data, size_t size,
                                         const SkDeserialProcs& procs);

    void dump(std::string &desc, int depth) const;

private:
    friend class SkNVRefCnt<SkTextBlob>;
    class RunRecord;

    enum GlyphPositioning : uint8_t;

    explicit SkTextBlob(const SkRect& bounds);

    ~SkTextBlob();

    // Memory for objects of this class is created with sk_malloc rather than operator new and must
    // be freed with sk_free.
    void operator delete(void* p);
    void* operator new(size_t);
    void* operator new(size_t, void* p);

    static unsigned ScalarsPerGlyph(GlyphPositioning pos);

    // Call when this blob is part of the key to a cache entry. This allows the cache
    // to know automatically those entries can be purged when this SkTextBlob is deleted.
    void notifyAddedToCache(uint32_t cacheID) const {
        fCacheID.store(cacheID);
    }

    friend class SkGlyphRunList;
    friend class GrTextBlobCache;
    friend class SkTextBlobBuilder;
    friend class SkTextBlobPriv;
    friend class SkTextBlobRunIterator;

    const SkRect                  fBounds;
    const uint32_t                fUniqueID;
    mutable std::atomic<uint32_t> fCacheID;

    SkDEBUGCODE(size_t fStorageSize;)

    // The actual payload resides in externally-managed storage, following the object.
    // (see the .cpp for more details)

    typedef SkRefCnt INHERITED;
};

/** \class SkTextBlobBuilder
    Helper class for constructing SkTextBlob.
*/
class SK_API SkTextBlobBuilder {
public:

    /** Constructs empty SkTextBlobBuilder. By default, SkTextBlobBuilder has no runs.

        @return  empty SkTextBlobBuilder
    */
    SkTextBlobBuilder();

    /** Deletes data allocated internally by SkTextBlobBuilder.
    */
    ~SkTextBlobBuilder();

    /** Returns SkTextBlob built from runs of glyphs added by builder. Returned
        SkTextBlob is immutable; it may be copied, but its contents may not be altered.
        Returns nullptr if no runs of glyphs were added by builder.

        Resets SkTextBlobBuilder to its initial empty state, allowing it to be
        reused to build a new set of runs.

        @return  SkTextBlob or nullptr
    */
    sk_sp<SkTextBlob> make();

    /** \struct SkTextBlobBuilder::RunBuffer
        RunBuffer supplies storage for glyphs and positions within a run.

        A run is a sequence of glyphs sharing font metrics and positioning.
        Each run may position its glyphs in one of three ways:
        by specifying where the first glyph is drawn, and allowing font metrics to
        determine the advance to subsequent glyphs; by specifying a baseline, and
        the position on that baseline for each glyph in run; or by providing SkPoint
        array, one per glyph.
    */
    struct RunBuffer {
        SkGlyphID* glyphs;   //!< storage for glyphs in run
        SkScalar*  pos;      //!< storage for positions in run
        char*      utf8text; //!< reserved for future use
        uint32_t*  clusters; //!< reserved for future use

        // Helpers, since the "pos" field can be different types (always some number of floats).
        SkPoint*    points() const { return reinterpret_cast<SkPoint*>(pos); }
        SkRSXform*  xforms() const { return reinterpret_cast<SkRSXform*>(pos); }
    };

    /** Returns run with storage for glyphs. Caller must write count glyphs to
        RunBuffer::glyphs before next call to SkTextBlobBuilder.

        RunBuffer::utf8text, and RunBuffer::clusters should be ignored.

        Glyphs share metrics in font.

        Glyphs are positioned on a baseline at (x, y), using font metrics to
        determine their relative placement.

        bounds defines an optional bounding box, used to suppress drawing when SkTextBlob
        bounds does not intersect SkSurface bounds. If bounds is nullptr, SkTextBlob bounds
        is computed from (x, y) and RunBuffer::glyphs metrics.

        @param font    SkFont used for this run
        @param count   number of glyphs
        @param x       horizontal offset within the blob
        @param y       vertical offset within the blob
        @param bounds  optional run bounding box
        @return        writable glyph buffer
    */
    const RunBuffer& allocRun(const SkFont& font, int count, SkScalar x, SkScalar y,
                              const SkRect* bounds = nullptr);

    /** Returns run with storage for glyphs and positions along baseline. Caller must
        write count glyphs to RunBuffer::glyphs, and count scalars to RunBuffer::pos;
        before next call to SkTextBlobBuilder.

        RunBuffer::utf8text, and RunBuffer::clusters should be ignored.

        Glyphs share metrics in font.

        Glyphs are positioned on a baseline at y, using x-axis positions written by
        caller to RunBuffer::pos.

        bounds defines an optional bounding box, used to suppress drawing when SkTextBlob
        bounds does not intersect SkSurface bounds. If bounds is nullptr, SkTextBlob bounds
        is computed from y, RunBuffer::pos, and RunBuffer::glyphs metrics.

        @param font    SkFont used for this run
        @param count   number of glyphs
        @param y       vertical offset within the blob
        @param bounds  optional run bounding box
        @return        writable glyph buffer and x-axis position buffer
    */
    const RunBuffer& allocRunPosH(const SkFont& font, int count, SkScalar y,
                                  const SkRect* bounds = nullptr);

    /** Returns run with storage for glyphs and SkPoint positions. Caller must
        write count glyphs to RunBuffer::glyphs, and count SkPoint to RunBuffer::pos;
        before next call to SkTextBlobBuilder.

        RunBuffer::utf8text, and RunBuffer::clusters should be ignored.

        Glyphs share metrics in font.

        Glyphs are positioned using SkPoint written by caller to RunBuffer::pos, using
        two scalar values for each SkPoint.

        bounds defines an optional bounding box, used to suppress drawing when SkTextBlob
        bounds does not intersect SkSurface bounds. If bounds is nullptr, SkTextBlob bounds
        is computed from RunBuffer::pos, and RunBuffer::glyphs metrics.

        @param font    SkFont used for this run
        @param count   number of glyphs
        @param bounds  optional run bounding box
        @return        writable glyph buffer and SkPoint buffer
    */
    const RunBuffer& allocRunPos(const SkFont& font, int count,
                                 const SkRect* bounds = nullptr);

    // RunBuffer.pos points to SkRSXform array
    const RunBuffer& allocRunRSXform(const SkFont& font, int count);

private:
    const RunBuffer& allocRunText(const SkFont& font,
                                  int count,
                                  SkScalar x,
                                  SkScalar y,
                                  int textByteCount,
                                  SkString lang,
                                  const SkRect* bounds = nullptr);
    const RunBuffer& allocRunTextPosH(const SkFont& font, int count, SkScalar y,
                                      int textByteCount, SkString lang,
                                      const SkRect* bounds = nullptr);
    const RunBuffer& allocRunTextPos(const SkFont& font, int count,
                                     int textByteCount, SkString lang,
                                     const SkRect* bounds = nullptr);
    const RunBuffer& allocRunRSXform(const SkFont& font, int count,
                                     int textByteCount, SkString lang,
                                     const SkRect* bounds = nullptr);

    void reserve(size_t size);
    void allocInternal(const SkFont& font, SkTextBlob::GlyphPositioning positioning,
                       int count, int textBytes, SkPoint offset, const SkRect* bounds);
    bool mergeRun(const SkFont& font, SkTextBlob::GlyphPositioning positioning,
                  uint32_t count, SkPoint offset);
    void updateDeferredBounds();

    static SkRect ConservativeRunBounds(const SkTextBlob::RunRecord&);
    static SkRect TightRunBounds(const SkTextBlob::RunRecord&);

    friend class SkTextBlobPriv;
    friend class SkTextBlobBuilderPriv;

    SkAutoTMalloc<uint8_t> fStorage;
    size_t                 fStorageSize;
    size_t                 fStorageUsed;

    SkRect                 fBounds;
    int                    fRunCount;
    bool                   fDeferredBounds;
    size_t                 fLastRun; // index into fStorage

    RunBuffer              fCurrentRunBuffer;
};

#endif // SkTextBlob_DEFINED
