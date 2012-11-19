#include "texture-atlas.h"
#include "texture-font.h"

int main(int argc, char **argv) {

  /* Text to be printed */
  wchar_t *text = L"A Quick Brown Fox Jumps Over The Lazy Dog 0123456789";

  /* Texture atlas to store individual glyphs */
  texture_atlas_t *atlas = texture_atlas_new( 512, 512, 1 );

  texture_font_t *font = texture_font_new( atlas, "./fonts/Vera.ttf", 16 );

  /* Cache some glyphs to speed things up */
  texture_font_load_glyphs( font, L" !\"#$%&'()*+,-./0123456789:;<=>?"
			    L"@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
			    L"`abcdefghijklmnopqrstuvwxyz{|}~");


  return 0;
}
