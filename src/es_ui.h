/*
 * es_ui has all of the ui and text rendering functinality. It creates the vertex and index
 * buffers required to render the text. It does not do any of the actual rendering.
 */

#ifndef ES_UI_DEFINED
#define ES_UI_DEFINED

#include "SDL.h"
#include "es_warehouse.h"

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include "stb_truetype.h"

typedef struct {
    Uint32 num_vertices;
    Uint32 vertices_size;
    EsVertex* vertices;
    Uint32 num_indices;
    Uint32 indices_size;
    Uint32* indices;
    Uint32 tex_width;
    Uint32 tex_height;
    Uint8* texture;
    stbtt_bakedchar glyphs[96];
} EsUI;

extern SDL_bool ui_init(EsUI* ui);
extern SDL_bool ui_render_text(EsUI* ui, const char* text, float x, float y);
extern SDL_bool ui_flush(EsUI* ui);


#endif
