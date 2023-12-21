//ftt
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftpfr.h>
#include <freetype/ftadvanc.h>

#include <crft.h>

static FT_Library library;

CRBOOL _init_crft_()
{
	FT_Error error = FT_Init_FreeType(&library);
	return CRTRUE;
}