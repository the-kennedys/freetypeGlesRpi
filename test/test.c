
/* Most of this code is an opengl es 2 version of Freetype GL, with
   the following license. 
 ============================================================================
 * Freetype GL - A C OpenGL Freetype engine
 * Platform:    Any
 * WWW:         http://code.google.com/p/freetype-gl/
 * ----------------------------------------------------------------------------
 * Copyright 2011,2012 Nicolas P. Rougier. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY NICOLAS P. ROUGIER ''AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL NICOLAS P. ROUGIER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of Nicolas P. Rougier. 
 */

/* This is a demonstration of the font atlas and font implementations */


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
#include <math.h>

bool *keys;
int *mouse;
struct joystick_t *joy1;

texture_font_t *font1, *font2;
texture_atlas_t *atlas;

char * vert = 
  "uniform mat4		u_mvp;\n"
  "attribute vec3		a_position;\n"
  "attribute vec4		a_color;\n"
  "attribute vec2		a_st;\n"
  "varying vec2		        v_frag_uv;\n"
  "varying vec4		        v_color;\n"
  "void main(void) {\n"
  "       v_frag_uv = a_st;\n"
  "       gl_Position = u_mvp * vec4(a_position,1);\n"
  "       v_color = a_color;\n"
  "}\n";


char * frag = 
  "precision mediump float;\n"
  "uniform sampler2D		texture_uniform;\n"
  "varying vec2 v_frag_uv;\n"
  "varying vec4		        v_color;\n"
  "void main()\n"
  "{\n"
  "    gl_FragColor = vec4(v_color.xyz, v_color.a * texture2D(texture_uniform, v_frag_uv).a);\n"
  "}\n";

int programHandle;
int vertexHandle, texHandle, samplerHandle, colorHandle, mvpHandle;
float pitch, roll, yaw;

// --------------------------------------------------------------- add_text ---
void add_text( vector_t * vVector, texture_font_t * font,
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

            // data is x,y,z,s,t,r,g,b,a
            GLfloat vertices[] = {
              x0,y0,0,
              s0,t0,
              r, g, b, a,
              x0,y1,0,
              s0,t1,
              r, g, b, a,
              x1,y1,0,
              s1,t1,
              r, g, b, a,
              x0,y0,0,
              s0,t0,
              r, g, b, a,
              x1,y1,0,
              s1,t1,
              r, g, b, a,
              x1,y0,0,
              s1,t0,
              r, g, b, a
            };

            vector_push_back_data( vVector, vertices, 9*6);

            pen->x += glyph->advance_x;
        }
    }
}



void render() {
    float sy, cy, sp, cp, a, b;
    wchar_t disp[100];

    vector_t * vVector = vector_new(sizeof(GLfloat));

    vec2 pen = {-400,150};
    vec4 color = {.2,0.2,0.2,1};
    
    add_text( vVector, font1,
              L"freetypeGlesRpi", &color, &pen );

    pen.x = -390;
    pen.y = 140;
    vec4 transColor = {1,0.3,0.3,0.6};

    add_text( vVector, font1,
              L"freetypeGlesRpi", &transColor, &pen );


    pen.x = -190;
    pen.y = 0;

    add_text( vVector, font2,
              L"Roller Racing Demo", &color, &pen );

    
    swprintf(disp, 60, L"Pitch %.1f, Yaw %.1f", pitch, yaw);
    pen.x = -100;
    pen.y = -100;

    add_text( vVector, font2,
              disp, &color, &pen );

   // Clear the color buffer
   glClear ( GL_COLOR_BUFFER_BIT );

   // Use the program object
   glUseProgram ( programHandle );
   /*
     a 0 0 0     c -s 0 0  ac -as 0 0  cp 0 -sp 0    a*cy*cp -a*sp -sp*a*cy 0
     0 b 0 0     s c 0 0   bs  bc 0 0  0  1   0 0    b*sy*cp b*cy -sp*b*cy 0
     0 0 1 0     0 0 1 0   0    0 1 0  sp 0  cp 0    sp         0   cp     0
     0 0 0 1     0 0 0 1   0    0 0 1  0  0   0 1    0          0   0      1
    */
    // Pitch, roll and yaw
    yaw = yaw + 0.01;
    pitch = pitch + 0.01;
    sy = sin(yaw);
    cy = cos(yaw);
    sp = sin(pitch);
    cp = cos(pitch);
    a = 1.0f/getDisplayWidth();
    b = 1.0f/getDisplayHeight();
    // Set scaling so model coords are screen coords
    GLfloat mvp[] = {
      a*cy*cp, -a*sy, -sp*a*cy, 0,
      b*sy*cp, b*cy, -sp*b*cy, 0,
      sp, 0, cp, 0,
      0, 0, 0, 1.0
    };

    glUniformMatrix4fv(mvpHandle, 1, GL_FALSE, (GLfloat *) mvp);

   // Load the vertex data
   glVertexAttribPointer ( vertexHandle, 3, GL_FLOAT, GL_FALSE, 9*sizeof(GLfloat), vVector->items );
   glEnableVertexAttribArray ( vertexHandle );
   glVertexAttribPointer ( texHandle, 2, GL_FLOAT, GL_FALSE, 9*sizeof(GLfloat), (GLfloat*)vVector->items+3 );
   glEnableVertexAttribArray ( texHandle );
   glVertexAttribPointer ( colorHandle, 4, GL_FLOAT, GL_FALSE, 9*sizeof(GLfloat), (GLfloat*)vVector->items+5 );
   glEnableVertexAttribArray ( colorHandle );

   glActiveTexture( GL_TEXTURE0 );
   glBindTexture( GL_TEXTURE_2D, atlas->id );

   glUniform1i ( samplerHandle, 0);

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);

   glDrawArrays ( GL_TRIANGLES, 0, vVector->size/9 );

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
    glClearColor(0.8, 0.8, 0.8, 1);


  /* Texture atlas to store individual glyphs */
  atlas = texture_atlas_new( 1024, 1024, 1 );

  font1 = texture_font_new( atlas, "./fonts/custom.ttf", 50 );
  font2 = texture_font_new( atlas, "./fonts/ObelixPro.ttf", 70 );

  /* Cache some glyphs to speed things up */
  texture_font_load_glyphs( font1, L" !\"#$%&'()*+,-./0123456789:;<=>?"
			    L"@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
			    L"`abcdefghijklmnopqrstuvwxyz{|}~");

  texture_font_load_glyphs( font2, L" !\"#$%&'()*+,-./0123456789:;<=>?"
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
    vertexHandle = glGetAttribLocation ( programHandle, "a_position" );
    texHandle = glGetAttribLocation ( programHandle, "a_st" );
    colorHandle = glGetAttribLocation ( programHandle, "a_color" );
    samplerHandle = glGetUniformLocation( programHandle, "texture_uniform" );

    mvpHandle = glGetUniformLocation(programHandle, "u_mvp");
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


