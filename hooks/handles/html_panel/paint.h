int __fastcall raicu::hooks::handles::html_panel_paint(void* html_panel)
{
    static std::once_flag once;
    std::call_once(once, [&]()
    {
        globals::settings::menu_panel = html_panel;

        const char* script = xorstr(R"(
        UpdateNewsList(
            [
                {
                    "Title":"Main update",
                    "ShortName":"rmod-april-update",
                    "Tags":"Update",
                    "Date":"2025-05-09T21:57:00",
                    "Url":"https://raicu.vip/",
                    "SummaryHtml":"general update, every feature",
                    "HeaderImage":"https://s4.gifyu.com/images/bLJnJ.png"
                }
            ], false);
        )");

        const char* background_script = xorstr(R"(
        function UpdateBackground() {
            $('body').css({
                'background-image': scope.InGame ? 'none' : 'url("https://s4.gifyu.com/images/bLJuM.png")',
                'background-size': scope.InGame ? 'auto' : 'cover',
                'background-repeat': scope.InGame ? 'repeat' : 'no-repeat'
            });
        }
        setInterval(UpdateBackground, 1);
        )");

        utilities::run_javascript(html_panel, script);
        // utilities::run_javascript(html_panel, background_script);
    });

    return originals::html_panel_paint(html_panel);
}