#include "texture-atlas.h"

#include "texture-font.h"

#include "support.h"
#include "input.h"
#include "keys.h"

#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>		// usleep
#include <string.h>
#include <stdio.h>

bool *keys;
int *mouse;
struct joystick_t *joy1;

texture_font_t *font;

char * vert = "attribute vec3		vert_attrib;\
attribute vec2		uv_attrib;\
\
uniform mat4		opm_uniform;\
\
varying vec2		v_frag_uv;\
\
void main(void) {\
\
	v_frag_uv = uv_attrib;\
    gl_Position = opm_uniform * vec4(vert_attrib,1);\
\
}\
";

char * frag = "uniform sampler2D		texture_uniform;\
uniform float cx,cy;\
varying vec2 v_frag_uv;\
 \
\
void main()\
{\
	float x=cx+v_frag_uv.x;\
	float y=cy+v_frag_uv.y;\
    gl_FragColor = texture2D(texture_uniform, vec2(x,y));\
}\
";

int programHandle;

void render() {


    texture_glyph_t *tgp = texture_font_get_glyph( font, 'A' );

}

int main(int argc, char **argv) {
  int vHandle, fHandle, length, compile_ok;
  /* Text to be printed */
  wchar_t *text = L"A Quick Brown Fox Jumps Over The Lazy Dog 0123456789";

  /* Texture atlas to store individual glyphs */
  texture_atlas_t *atlas = texture_atlas_new( 512, 512, 1 );

  font = texture_font_new( atlas, "./fonts/Vera.ttf", 16 );

  /* Cache some glyphs to speed things up */
  texture_font_load_glyphs( font, L" !\"#$%&'()*+,-./0123456789:;<=>?"
			    L"@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
			    L"`abcdefghijklmnopqrstuvwxyz{|}~");



    // creates a window and GLES context
    if (makeContext() != 0)
        exit(-1);

    // all the shaders have at least texture unit 0 active so
    // activate it now and leave it active
    glActiveTexture(GL_TEXTURE0);


    glViewport(0, 0, getDisplayWidth(), getDisplayHeight());


    // we don't want to draw the back of triangles
    // the blending is set up for glprint but disabled
    // while not in use
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0, 0.5, 1, 1);

    vHandle = glCreateShader(GL_VERTEX_SHADER);
    length = strlen(vert);
    glShaderSource(vHandle, 1, (const char **)&vert, &length);
    glCompileShader(vHandle);
    glGetShaderiv(vHandle, GL_COMPILE_STATUS, &compile_ok);
    if (compile_ok == GL_FALSE) {
        fprintf(stderr, "vert:");
        print_log(vHandle);
        glDeleteShader(vHandle);
        return 0;
    }

    fHandle = glCreateShader(GL_FRAGMENT_SHADER);
    length = strlen(frag);
    glShaderSource(fHandle, 1, (const char **)&frag, &length);
    glCompileShader(fHandle);
    glGetShaderiv(fHandle, GL_COMPILE_STATUS, &compile_ok);
    if (compile_ok == GL_FALSE) {
        fprintf(stderr, "frag:");
        print_log(fHandle);
        glDeleteShader(fHandle);
        return 0;
    }

    programHandle = glCreateProgram();

    glAttachShader(programHandle, vHandle);
    glAttachShader(programHandle, fHandle);

    glLinkProgram(programHandle);

    glGetProgramiv(programHandle, GL_LINK_STATUS, &compile_ok);
    if (!compile_ok) {
        printf("glLinkProgram:");
        print_log(programHandle);
        printf("\n");
    }



    texture_atlas_upload(atlas);


    // count each frame
    int num_frames = 0;

    // set to true to leave main loop
    bool quit = false;

    // get a pointer to the key down array
    keys = getKeys();
    mouse = getMouse();
    joy1=getJoystick(0);
    //setMouseRelative(true);

    while (!quit) {		// the main loop

        doEvents();	// update mouse and key arrays
        updateJoystick(joy1);

        if (keys[KEY_ESC])
            quit = true;	// exit if escape key pressed

        // if (keys[KEY_A]) camAng=camAng+1;
        // if (keys[KEY_S]) camAng=camAng-1;
        // if (keys[KEY_W]) lightAng=lightAng+1;
        // if (keys[KEY_Q]) lightAng=lightAng-1;

        render();	// the render loop

        usleep(16000);	// no need to run cpu/gpu full tilt

    }

    closeContext();		// tidy up
    releaseJoystick(joy1);




  return 0;
}


