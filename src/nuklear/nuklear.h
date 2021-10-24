#pragma once

// optional flags
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT

#include "impl/nuklear.h"
#include "impl/nuklear_sdl_gl3.h"

#include <stb/stb_image.h>
// icon_load() function from nuklear extended demo
static inline struct nk_image
nk_image_load(const char *filename)
{
    int x,y,n;
    GLuint tex;

    unsigned char *data = stbi_load(filename, &x, &y, &n, 0);
    //if (!data) die("[SDL]: failed to load image: %s", filename);
    if (!data) {
		exit(1);
		fprintf(stderr, "couldn't load image %s\n", filename);
		return nk_image_id(0);
	}

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    return nk_image_id((int)tex);
}
