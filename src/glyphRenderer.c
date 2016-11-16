/*
 * Copyright (c) 2016 Jonathan Glines
 * Jonathan Glines <jonathan@glines.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <assert.h>
#include <math.h>

#include "fonts.h"

#include "glyphRenderer.h"

/* Private methods */
void st_GlyphRenderer_updateCellSize(
    st_GlyphRenderer *self);

void st_GlyphRenderer_calculateFacePixelBBox(
    const FT_Face face,
    int *width, int *height);

struct st_GlyphRenderer_Internal {
  /* TODO: Support multiple font faces in this structure */
  /* TODO: Somewhere in the FreeType documentation they suggest disposing of
   * FT_Face objects whenever possible. It might be best not to keep these
   * objects around, but I don't know. */
  FT_Face face;
  int cellSize[2];
};

void st_GlyphRenderer_init(
    st_GlyphRenderer *self)
{
  /* FIXME: Get rid of this path */
#define FONT_FACE_PATH "/nix/store/fvwp39z54ka2s7h3gawhfmayrqjnd05a-dejavu-fonts-2.37/share/fonts/truetype/DejaVuSansMono.ttf"

  FT_Error error;
  FT_Library ft;
  const char *fontPath = FONT_FACE_PATH;

  ft = st_Fonts_getFreeTypeInstance();

  /* Allocate internal data structures */
  self->internal = (struct st_GlyphRenderer_Internal*)malloc(
      sizeof(struct st_GlyphRenderer_Internal));

  /* TODO: Support configuration of the font face to load. This might require
   * an st_GlyphRenderer_addFace() method. */
  error = FT_New_Face(
      ft,  /* library */
      fontPath,  /* filepathname */
      0,  /* face_index */
      &self->internal->face  /* aface */
      );
  if (error == FT_Err_Unknown_File_Format) {
    fprintf(stderr, "Freetype encountered an unknown file format: %s\n",
        fontPath);
    /* TODO: Print the specific error message from Freetype */
  } else if (error != FT_Err_Ok) {
    fprintf(stderr, "Freetype encountered an error reading file: %s\n",
        fontPath);
    /* TODO: Print the specific error message from Freetype */
  }
  /* TODO: Fail gracefully? */
  /* TODO: Support setting different font sizes */
  /* TODO: Support differentiation between bitmap and vector fonts here */
  error = FT_Set_Char_Size(
      self->internal->face,  /* face */
      0,  /* char_width */
      16*64,  /* char_height */
      300,  /* horz_resolution */
      300  /* vert_resolution */
      );
  if (error != FT_Err_Ok) {
    fprintf(stderr,
        "Freetype failed to set dimensions for font: %s\n",
        fontPath);
    /* TODO: Fail gracefully? */
  }
  st_GlyphRenderer_updateCellSize(self);
}

void st_GlyphRenderer_updateCellSize(
    st_GlyphRenderer *self)
{
  /* Calculate the cell size for the current base font */
  st_GlyphRenderer_calculateFacePixelBBox(
      self->internal->face,  /* face */
      &self->internal->cellSize[0],  /* width */
      &self->internal->cellSize[1]  /* height */
      );
}

/** Each FreeType font face defines a "bounding box" that encloses all glyphs.
 * The atlas does not use this information, since it packs glyphs as tightly as
 * possible, but we do need to compute bounding box information for the
 * terminal. */
void st_GlyphRenderer_calculateFacePixelBBox(
    const FT_Face face,
    int *width, int *height)
{
  double x_pixels_per_unit, y_pixels_per_unit, units_per_em;
  /* Compute a scaling factor to convert from font units to pixels, as
   * described in
   * <https://www.freetype.org/freetype2/docs/tutorial/step2.html> */
  units_per_em = (double)face->units_per_EM;
  x_pixels_per_unit = face->size->metrics.x_ppem / units_per_em;
  y_pixels_per_unit = face->size->metrics.y_ppem / units_per_em;
  *width = (int)ceil(
      (double)(face->bbox.xMax - face->bbox.xMin) * x_pixels_per_unit);
  *height = (int)ceil(
      (double)(face->bbox.yMax - face->bbox.yMin) * y_pixels_per_unit);
}

void st_GlyphRenderer_destroy(
    st_GlyphRenderer *self)
{
  /* Free internal data structures */
  FT_Done_Face(self->internal->face);
  free(self->internal);
}

void st_GlyphRenderer_getCellSize(
    const st_GlyphRenderer *self,
    int *width, int *height)
{
  *width = self->internal->cellSize[0];
  *height = self->internal->cellSize[1];
}

int st_GlyphRenderer_getGlyphDimensions(
    st_GlyphRenderer *self,
    uint32_t character,
    int *width, int *height)
{
  int glyph_index;
  FT_Error error;
  /* TODO: Look for a face that provides this glyph (once we add support for
   * multiple faces) */
  /* Check for a corresponding glyph provided by this face */
  glyph_index = FT_Get_Char_Index(self->internal->face, character);
  if (!glyph_index)
    return 1;  /* Error; could not find the glyph */
  /* Load the glyph */
  error = FT_Load_Glyph(
      self->internal->face,  /* face */
      glyph_index,  /* glyph_index */
      FT_LOAD_DEFAULT  /* load_flags */
      );
  if (error != FT_Err_Ok) {
    fprintf(stderr,
        "Freetype error loading the glyph for ASCII character '%c'\n",
        (char)character);
  }
  /* Calculate the pixel dimensions of this glyph, as we would render it */
  /* We convert the fixed-point 26.6 format (a.k.a. 1/64th pixel format) to an
   * integer value in pixels, rounding up */
#define TWENTY_SIX_SIX_TO_PIXELS(value) ( \
    ((value) >> 6) + ((value) & ((1 << 6) - 1) ? 1 : 0))
  *width = 
    TWENTY_SIX_SIX_TO_PIXELS(self->internal->face->glyph->metrics.width);
  *height = 
    TWENTY_SIX_SIX_TO_PIXELS(self->internal->face->glyph->metrics.height);
  return 0;
}

FT_Bitmap *st_GlyphRenderer_renderGlyph(
    st_GlyphRenderer *self,
    uint32_t character)
{
  int glyph_index;
  FT_Error error;

  /* Render the glyph for this character */
  glyph_index = FT_Get_Char_Index(self->internal->face, character);
  assert(glyph_index != 0);
  error = FT_Load_Glyph(
      self->internal->face,  /* face */
      glyph_index,  /* glyph_index */
      FT_LOAD_DEFAULT  /* load_flags */
      );
  assert(error == FT_Err_Ok);
  error = FT_Render_Glyph(
      self->internal->face->glyph,
      FT_RENDER_MODE_NORMAL);
  if (error != FT_Err_Ok) {
    fprintf(stderr,
        "Freetype encountered an error rendering the glyph for '0x%08x'\n",
        character);
    /* TODO: Fail gracefully */
    assert(0);
  }
  return &self->internal->face->glyph->bitmap;
}

void st_GlyphRenderer_getGlyphOffset(
    st_GlyphRenderer *self,
    uint32_t character,
    int *x, int *y)
{
  int glyph_index;
  FT_Error error;
  /* TODO: Look for a face that provides this glyph (once we add support for
   * multiple faces) */
  glyph_index = FT_Get_Char_Index(self->internal->face, character);
  assert(glyph_index);
  /* Load the glyph */
  error = FT_Load_Glyph(
      self->internal->face,  /* face */
      glyph_index,  /* glyph_index */
      FT_LOAD_DEFAULT  /* load_flags */
      );
  if (error != FT_Err_Ok) {
    fprintf(stderr,
        "Freetype error loading the glyph for ASCII character '%c'\n",
        (char)character);
  }
  /* TODO: Calculate the offset of this glyph */
  *x = 10;
  *y = 10;
}
