#include "es_ui.h"

#define NUM_CHARS 2048
#define NUM_VERTICES NUM_CHARS*4
#define NUM_INDICES NUM_CHARS*6
#define FONT_FILE "data/fonts/JetBrainsMono/ttf/JetBrainsMono-Light.ttf"
#define FONT_SIZE 18

SDL_bool _load_font(EsUI* ui) {
    // TODO (10 Dec 2020 sam): Figure out the correct texture size based on whatever
    ui->tex_width = 512;
    ui->tex_height = 512;
    ui->texture = (Uint8*) SDL_malloc(ui->tex_width*ui->tex_height*4 * sizeof(Uint8));
    Uint8* temp_texture = (Uint8*) SDL_malloc(ui->tex_width*ui->tex_height * sizeof(Uint8));
    SDL_RWops* font_file = SDL_RWFromFile(FONT_FILE, "rb");
    if (font_file == NULL) {
        warehouse_error_popup("Font load error", "Could not find font file");
        return SDL_FALSE;
    }
    Sint64 file_size = SDL_RWseek(font_file, 0, RW_SEEK_END);
    if (file_size < 0) {
        SDL_RWclose(font_file);
        warehouse_error_popup("Font load error", "Could not read font file");
        return SDL_FALSE;
    }
    Sint64 result = SDL_RWseek(font_file, 0, RW_SEEK_SET);
    if (result < 0) {
        SDL_RWclose(font_file);
        warehouse_error_popup("Font load error", "Could seek read font file");
        return SDL_FALSE;
    }
    Uint8* font_raw = (Uint8*) SDL_malloc(file_size * sizeof(Uint8));
    size_t read_size = SDL_RWread(font_file, font_raw, sizeof(Uint8), file_size);
    if ((Sint64)(read_size*sizeof(Uint8)) != file_size) {
        SDL_free(font_raw);
        SDL_RWclose(font_file);
        warehouse_error_popup("Font load error", "Could not read full font file");
        return SDL_FALSE;
    }
    SDL_RWclose(font_file);
    result = stbtt_BakeFontBitmap(font_raw, 0, FONT_SIZE, ui->texture, ui->tex_width, ui->tex_height, 32, 95, ui->glyphs);
    SDL_memset(ui->texture, 0, ui->tex_width*ui->tex_height*4*sizeof(Uint8));
    for (Uint32 i=0; i<ui->tex_width*ui->tex_height; i++)
        ui->texture[i*4] = temp_texture[i];
    if (result == 0) {
        SDL_free(font_raw);
        warehouse_error_popup("Font load error", "Could not load font glyphs");
        return SDL_FALSE;
    }
    SDL_free(font_raw);
    SDL_free(temp_texture);
    return SDL_TRUE;
}

SDL_bool ui_init(EsUI* ui) {
    SDL_bool sdl_result;
    ui->num_vertices = 0;
    ui->vertices_size = NUM_VERTICES;
    ui->vertices = (EsVertex*) SDL_malloc(ui->vertices_size * sizeof(EsVertex));
    ui->num_indices = 0;
    ui->indices_size = NUM_INDICES;
    ui->indices = (Uint32*) SDL_malloc(ui->indices_size * sizeof(Uint32));
    sdl_result = _load_font(ui);
    if (!sdl_result) return SDL_FALSE;
    return SDL_TRUE;
}

SDL_bool ui_flush(EsUI* ui) {
    if (ui->num_vertices > 0) {
        ui->num_vertices = 0;
        SDL_memset(ui->vertices, 0, ui->vertices_size * sizeof(EsVertex));
    }
    if (ui->num_indices > 0) {
        ui->num_indices = 0;
        SDL_memset(ui->indices, 0, ui->vertices_size * sizeof(Uint32));
    }
    return SDL_TRUE;
}

SDL_bool ui_render_text(EsUI* ui, const char* text, float x, float y) {
    size_t len = SDL_strlen(text);
    Uint32 first_vertex = 0;
    Uint32 first_index = 0;
    ui->vertices[first_vertex + 0].pos.x = -1.0;
    ui->vertices[first_vertex + 0].pos.y = -1.0;
    ui->vertices[first_vertex + 1].pos.x = -1.0;
    ui->vertices[first_vertex + 1].pos.y = 1.0;
    ui->vertices[first_vertex + 2].pos.x = 1.0;
    ui->vertices[first_vertex + 2].pos.y = 1.0;
    ui->vertices[first_vertex + 3].pos.x = 1.0;
    ui->vertices[first_vertex + 3].pos.y = -1.0;
    ui->vertices[first_vertex + 0].tex.x = 0.0;
    ui->vertices[first_vertex + 0].tex.y = 0.0;
    ui->vertices[first_vertex + 1].tex.x = 1.0;
    ui->vertices[first_vertex + 1].tex.y = 0.0;
    ui->vertices[first_vertex + 2].tex.x = 1.0;
    ui->vertices[first_vertex + 2].tex.y = 1.0;
    ui->vertices[first_vertex + 3].tex.x = 0.0;
    ui->vertices[first_vertex + 3].tex.y = 1.0;
    ui->indices[first_index + 0] = first_vertex + 0;
    ui->indices[first_index + 1] = first_vertex + 2;
    ui->indices[first_index + 2] = first_vertex + 1;
    ui->indices[first_index + 3] = first_vertex + 0;
    ui->indices[first_index + 4] = first_vertex + 3;
    ui->indices[first_index + 5] = first_vertex + 2;
    return SDL_TRUE;

    for (Uint32 i=0; i<len; i++) {
        stbtt_aligned_quad q;
        char c = text[i] - 32;
        stbtt_GetBakedQuad(ui->glyphs, 512, 512, c, &x, &y, &q, 1);
        if (ui->num_vertices + 4 > ui->vertices_size) {
            SDL_Log("UI vertex buffer full. Cannot add further chars");
            break;
        }
        if (ui->num_indices + 4 > ui->indices_size) {
            SDL_Log("UI index buffer full. Cannot add further chars");
            break;
        }
        Uint32 first_vertex = ui->num_vertices;
        ui->num_vertices += 4;
        ui->vertices[first_vertex + 0].pos.x = q.x0;
        ui->vertices[first_vertex + 0].pos.y = q.y0;
        ui->vertices[first_vertex + 1].pos.x = q.x1;
        ui->vertices[first_vertex + 1].pos.y = q.y0;
        ui->vertices[first_vertex + 2].pos.x = q.x1;
        ui->vertices[first_vertex + 2].pos.y = q.y1;
        ui->vertices[first_vertex + 3].pos.x = q.x0;
        ui->vertices[first_vertex + 3].pos.y = q.y1;
        ui->vertices[first_vertex + 0].tex.x = q.s0;
        ui->vertices[first_vertex + 0].tex.y = q.t0;
        ui->vertices[first_vertex + 1].tex.x = q.s1;
        ui->vertices[first_vertex + 1].tex.y = q.t0;
        ui->vertices[first_vertex + 2].tex.x = q.s1;
        ui->vertices[first_vertex + 2].tex.y = q.t1;
        ui->vertices[first_vertex + 3].tex.x = q.s0;
        ui->vertices[first_vertex + 3].tex.y = q.t1;
        ui->vertices[first_vertex + 0].pos.z = 1.0f;
        ui->vertices[first_vertex + 1].pos.z = 1.0f;
        ui->vertices[first_vertex + 2].pos.z = 1.0f;
        ui->vertices[first_vertex + 3].pos.z = 1.0f;
        Uint32 first_index = ui->num_indices;
        ui->num_indices += 6;
        ui->indices[first_index + 0] = first_vertex + 0;
        ui->indices[first_index + 1] = first_vertex + 2;
        ui->indices[first_index + 2] = first_vertex + 1;
        ui->indices[first_index + 3] = first_vertex + 0;
        ui->indices[first_index + 4] = first_vertex + 3;
        ui->indices[first_index + 5] = first_vertex + 2;
    }
    return SDL_TRUE;
}
