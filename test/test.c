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
texture_atlas_t *atlas;

char * vert = 
  "attribute vec3		vposition;\n"
  "attribute vec2		uv_attrib;\n"
  "varying vec2		        v_frag_uv;\n"
  "void main(void) {\n"
  "       v_frag_uv = uv_attrib;\n"
  "       gl_Position = vec4(vposition,1);\n"
  "}\n";


char * frag = 
  "precision mediump float;\n"
  "uniform sampler2D		texture_uniform;\n"
  "varying vec2 v_frag_uv;\n"
  "void main()\n"
  "{\n"
  "    gl_FragColor = texture2D(texture_uniform, v_frag_uv);\n"
  "}\n";

int programHandle;
int vertexHandle, texHandle, samplerHandle;


// --------------------------------------------------------------- add_text ---
void add_text( vector_t * vVector, vector_t * tVector, texture_font_t * font,
               wchar_t * text, vec4 * color, vec2 * pen )
{
    size_t i;
    float r = color->red, g = color->green, b = color->blue, a = color->alpha;
    for( i=0; i<wcslen(text); ++i )
    {
        texture_glyph_t *glyph = texture_font_get_glyph( font, text[i] );
        if( glyph != NULL )
        {
            int kerning = 0;
            if( i > 0)
            {
                kerning = texture_glyph_get_kerning( glyph, text[i-1] );
            }
            pen->x += kerning;
            int x0  = (int)( pen->x + glyph->offset_x );
            int y0  = (int)( pen->y + glyph->offset_y );
            int x1  = (int)( x0 + glyph->width );
            int y1  = (int)( y0 - glyph->height );
            float s0 = glyph->s0;
            float t0 = glyph->t0;
            float s1 = glyph->s1;
            float t1 = glyph->t1;
            float scale = 0.01;
            GLfloat vertices[] = {
              x0*scale,y0*scale,0,
              x0*scale,y1*scale,0,
              x1*scale,y1*scale,0,
              x0*scale,y0*scale,0,
              x1*scale,y1*scale,0,
              x1*scale,y0*scale,0};
            GLfloat textures[] = {
              s0,t0,
              s0,t1,
              s1,t1,
              s0,t0,
              s1,t1,
              s1,t0};
            vector_push_back_data( vVector, vertices, 18);
            vector_push_back_data( tVector, textures, 12);
            pen->x += glyph->advance_x;
        }
    }
}



void render() {


    texture_glyph_t *tgp = texture_font_get_glyph( font, 'A' );
    vector_t * vVector = vector_new(sizeof(GLfloat));
    vector_t * tVector = vector_new(sizeof(GLfloat));
    GLfloat vVertices[] = { -0.1f, -0.1f, 0.0f, 
                            0.1f, 0.1f, 0.0f,
                            -0.1f, 0.1f, 0.0f,
                            -0.1f, -0.1f, 0.0f,
                            0.1f, -0.1f, 0.0f,
                            0.1f, 0.1f, 0.0f};
    GLfloat tVertices[] = { tgp->s0,tgp->t1,
                            tgp->s1,tgp->t0,
                            tgp->s0,tgp->t0,
                            tgp->s0,tgp->t1,
                            tgp->s1,tgp->t1,
                            tgp->s1,tgp->t0 };
    vec2 pen = {-2,-3};
    vec4 color = {1,0,0,1};
    
    add_text( vVector, tVector, font,
              L"Roller Racing", &color, &pen );
   // Clear the color buffer
   glClear ( GL_COLOR_BUFFER_BIT );

   // Use the program object
   glUseProgram ( programHandle );

   // Load the vertex data
   glVertexAttribPointer ( vertexHandle, 3, GL_FLOAT, GL_FALSE, 0, vVector->items );
   glEnableVertexAttribArray ( 0 );
   glVertexAttribPointer ( texHandle, 2, GL_FLOAT, GL_FALSE, 0, tVector->items );
   glEnableVertexAttribArray ( 1 );

   glActiveTexture( GL_TEXTURE0 );
   glBindTexture( GL_TEXTURE_2D, atlas->id );

   glUniform1i ( samplerHandle, 0);

   glDrawArrays ( GL_TRIANGLES, 0, 18 );

   swapBuffers();

}

int main(int argc, char **argv) {
  int vHandle, fHandle, length, compile_ok;
  /* Text to be printed */
  wchar_t *text = L"A Quick Brown Fox Jumps Over The Lazy Dog 0123456789";


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
    //glEnable(GL_DEPTH_TEST);
    glClearColor(0.5, 0.5, 0.5, 1);


  /* Texture atlas to store individual glyphs */
  atlas = texture_atlas_new( 512, 512, 3 );

  font = texture_font_new( atlas, "./fonts/Vera.ttf", 20 );

  /* Cache some glyphs to speed things up */
  texture_font_load_glyphs( font, L" !\"#$%&'()*+,-./0123456789:;<=>?"
			    L"@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
			    L"`abcdefghijklmnopqrstuvwxyz{|}~");




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

    
    // Bind vPosition to attribute 0
    vertexHandle = glGetAttribLocation ( programHandle, "vposition" );
    texHandle = glGetAttribLocation ( programHandle, "uv_attrib" );
    samplerHandle = glGetUniformLocation( programHandle, "texture_uniform" );
    printf("%d %d %d\n", vertexHandle, texHandle, samplerHandle);

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


