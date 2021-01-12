#include "SDL.h"

#define SHADER_DIRECTORY "src/glsl"
#define SHADER_TEMP_DIRECTORY "shaders"

typedef struct {
    char* filepath;
    char* contents;
} FileContents;

typedef struct {
    char* prefix;
    FileContents vertex;
    FileContents fragment;
} ShaderCode;

typedef struct {
    FileContents vertex_header;
    FileContents vertex_sm_header;
    FileContents vertex_footer;
    FileContents vertex_main;
    FileContents vertex_sm_main;
    FileContents fragment_header;
    FileContents fragment_sm_header;
    FileContents fragment_footer;
    FileContents fragment_main;
    FileContents fragment_sm_main;
} CommonCode;

int _read_file(FileContents* file) {
    Sint64 file_size;
    size_t read_size;
    Sint64 result;
    char filepath[128];
    SDL_snprintf(filepath, 128, "%s/%s", SHADER_DIRECTORY, file->filepath);
    SDL_RWops* f;
    f = SDL_RWFromFile(filepath, "r");
    if (f == NULL)
        return -1;
    file_size = SDL_RWseek(f, 0, RW_SEEK_END);
    if (file_size < 0)
        return -2;
    result = SDL_RWseek(f, 0, RW_SEEK_SET);
    if (result < 0)
        return -3;
    file->contents = (char*) SDL_malloc(file_size * sizeof(char));
    read_size = SDL_RWread(f, file->contents, sizeof(char), file_size);
    if ((Sint64) read_size != file_size) {
        SDL_free(file->contents);
        free(file->contents);
        return -4;
    }
    SDL_RWclose(f);
    return 0;
}

int _error_message_and_return(int code) {
    SDL_Log("Error in shader compilation %i", code);
    return code;
}

int _load_shader_code(ShaderCode* shader) {
    int result;
    result = _read_file(&shader->vertex);
    if (result != 0)  return _error_message_and_return(-1);
    result = _read_file(&shader->fragment);
    if (result != 0)  return _error_message_and_return(-1);
    return 0;
}

int _compile_shaders(ShaderCode* shader, CommonCode* common) {
    SDL_RWops* glsl_file;
    char filename[128];
    SDL_snprintf(filename, 128, "%s/%s_vertex.glsl", SHADER_TEMP_DIRECTORY, shader->prefix);
    glsl_file = SDL_RWFromFile(filename, "w");
    SDL_RWwrite(glsl_file, common->vertex_header.contents, 1, SDL_strlen(common->vertex_header.contents));
    SDL_RWwrite(glsl_file, shader->vertex.contents, 1, SDL_strlen(shader->vertex.contents));
    SDL_RWwrite(glsl_file, common->vertex_main.contents, 1, SDL_strlen(common->vertex_main.contents));
    SDL_RWwrite(glsl_file, common->vertex_footer.contents, 1, SDL_strlen(common->vertex_footer.contents));
    SDL_RWclose(glsl_file);
    SDL_snprintf(filename, 128, "%s/%s_sm_vertex.glsl", SHADER_TEMP_DIRECTORY, shader->prefix);
    glsl_file = SDL_RWFromFile(filename, "w");
    SDL_RWwrite(glsl_file, common->vertex_header.contents, 1, SDL_strlen(common->vertex_header.contents));
    SDL_RWwrite(glsl_file, common->vertex_sm_header.contents, 1, SDL_strlen(common->vertex_sm_header.contents));
    SDL_RWwrite(glsl_file, shader->vertex.contents, 1, SDL_strlen(shader->vertex.contents));
    SDL_RWwrite(glsl_file, common->vertex_main.contents, 1, SDL_strlen(common->vertex_main.contents));
    SDL_RWwrite(glsl_file, common->vertex_sm_main.contents, 1, SDL_strlen(common->vertex_sm_main.contents));
    SDL_RWwrite(glsl_file, common->vertex_footer.contents, 1, SDL_strlen(common->vertex_footer.contents));
    SDL_RWclose(glsl_file);
    SDL_snprintf(filename, 128, "%s/%s_fragment.glsl", SHADER_TEMP_DIRECTORY, shader->prefix);
    glsl_file = SDL_RWFromFile(filename, "w");
    SDL_RWwrite(glsl_file, common->fragment_header.contents, 1, SDL_strlen(common->fragment_header.contents));
    SDL_RWwrite(glsl_file, shader->fragment.contents, 1, SDL_strlen(shader->fragment.contents));
    SDL_RWwrite(glsl_file, common->fragment_main.contents, 1, SDL_strlen(common->fragment_main.contents));
    SDL_RWwrite(glsl_file, common->fragment_footer.contents, 1, SDL_strlen(common->fragment_footer.contents));
    SDL_RWclose(glsl_file);
    SDL_snprintf(filename, 128, "%s/%s_sm_fragment.glsl", SHADER_TEMP_DIRECTORY, shader->prefix);
    glsl_file = SDL_RWFromFile(filename, "w");
    SDL_RWwrite(glsl_file, common->fragment_header.contents, 1, SDL_strlen(common->fragment_header.contents));
    SDL_RWwrite(glsl_file, common->fragment_sm_header.contents, 1, SDL_strlen(common->fragment_sm_header.contents));
    SDL_RWwrite(glsl_file, shader->fragment.contents, 1, SDL_strlen(shader->fragment.contents));
    SDL_RWwrite(glsl_file, common->fragment_main.contents, 1, SDL_strlen(common->fragment_main.contents));
    SDL_RWwrite(glsl_file, common->fragment_sm_main.contents, 1, SDL_strlen(common->fragment_sm_main.contents));
    SDL_RWwrite(glsl_file, common->fragment_footer.contents, 1, SDL_strlen(common->fragment_footer.contents));
    SDL_RWclose(glsl_file);
    return 0;
}

int _copy_shaders(ShaderCode* shader) {
    int result;
    result = _load_shader_code(shader);
    if (result != 0)  return _error_message_and_return(-2);
    SDL_RWops* glsl_file;
    char filename[128];
    SDL_snprintf(filename, 128, "%s/%s_vertex.glsl", SHADER_TEMP_DIRECTORY, shader->prefix);
    glsl_file = SDL_RWFromFile(filename, "w");
    SDL_RWwrite(glsl_file, shader->vertex.contents, 1, SDL_strlen(shader->vertex.contents));
    SDL_RWclose(glsl_file);
    SDL_snprintf(filename, 128, "%s/%s_fragment.glsl", SHADER_TEMP_DIRECTORY, shader->prefix);
    glsl_file = SDL_RWFromFile(filename, "w");
    SDL_RWwrite(glsl_file, shader->fragment.contents, 1, SDL_strlen(shader->fragment.contents));
    SDL_RWclose(glsl_file);
    return 0;
}

int main(int argc, char** argv) {
    argc; argv;
    int result;
    CommonCode common;
    common.vertex_header.filepath = "vertex_header.glsl";
    common.vertex_sm_header.filepath = "vertex_sm_header.glsl";
    common.vertex_footer.filepath = "vertex_footer.glsl";
    common.vertex_main.filepath = "vertex_main.glsl";
    common.vertex_sm_main.filepath = "vertex_sm_main.glsl";
    result = _read_file(&common.vertex_header);
    if (result != 0)  return _error_message_and_return(-1);
    result = _read_file(&common.vertex_sm_header);
    if (result != 0)  return _error_message_and_return(-1);
    result = _read_file(&common.vertex_footer);
    if (result != 0)  return _error_message_and_return(-1);
    result = _read_file(&common.vertex_main);
    if (result != 0)  return _error_message_and_return(-1);
    result = _read_file(&common.vertex_sm_main);
    if (result != 0)  return _error_message_and_return(-1);
    common.fragment_header.filepath = "fragment_header.glsl";
    common.fragment_sm_header.filepath = "fragment_sm_header.glsl";
    common.fragment_footer.filepath = "fragment_footer.glsl";
    common.fragment_main.filepath = "fragment_main.glsl";
    common.fragment_sm_main.filepath = "fragment_sm_main.glsl";
    result = _read_file(&common.fragment_header);
    if (result != 0)  return _error_message_and_return(-1);
    result = _read_file(&common.fragment_sm_header);
    if (result != 0)  return _error_message_and_return(-1);
    result = _read_file(&common.fragment_footer);
    if (result != 0)  return _error_message_and_return(-1);
    result = _read_file(&common.fragment_main);
    if (result != 0)  return _error_message_and_return(-1);
    result = _read_file(&common.fragment_sm_main);
    if (result != 0)  return _error_message_and_return(-1);
    ShaderCode shaders[3];
    shaders[0].prefix = "tree";
    shaders[0].vertex.filepath = "tree_vertex.glsl";
    shaders[0].fragment.filepath = "tree_fragment.glsl";
    shaders[1].prefix = "grass";
    shaders[1].vertex.filepath = "grass_vertex.glsl";
    shaders[1].fragment.filepath = "grass_fragment.glsl";
    shaders[2].prefix = "base";
    shaders[2].vertex.filepath = "vertex.glsl";
    shaders[2].fragment.filepath = "fragment.glsl";
    for (Uint32 i=0; i<3; i++) {
        result = _load_shader_code(&shaders[i]);
        if (result != 0)  return _error_message_and_return(-2);
        result = _compile_shaders(&shaders[i], &common);
        if (result != 0)  return _error_message_and_return(-3);
    }
    ShaderCode skybox;
    skybox.prefix = "skybox";
    skybox.vertex.filepath = "skybox_vertex.glsl";
    skybox.fragment.filepath = "skybox_fragment.glsl";
    result = _copy_shaders(&skybox);
    if (result != 0)  return _error_message_and_return(-1);
    ShaderCode ui;
    ui.prefix = "ui";
    ui.vertex.filepath = "ui_vertex.glsl";
    ui.fragment.filepath = "ui_fragment.glsl";
    result = _copy_shaders(&ui);
    if (result != 0)  return _error_message_and_return(-1);
    SDL_Log("shader files generated. yay");
    return 0;
}
