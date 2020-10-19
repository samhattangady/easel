#include "es_warehouse.h"

void warehouse_error_popup(const char* error_header, const char* error_text) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, error_text);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, error_header, error_text, NULL);
    return;
}
