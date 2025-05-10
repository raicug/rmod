#include "globals/settings.h"

void __fastcall raicu::hooks::handles::html_panel_load_url(void *html_panel, const char *url) {
    if (raicu::globals::settings::loading_screen::enabled && !strstr(url, "assets://garrysmod/html/menu.html"))
        url = settings::loading_screen::url;

    originals::html_panel_load_url(html_panel, url);
}
