#include "custom.h"

#include "fonts/IconFontAwesome6.h"
using namespace ImGui;

void c_custom::render_arrows_for_horizontal_bar( ImVec2 pos, float alpha, float width, float height ) {

    auto draw = GetWindowDrawList( );
    draw->AddRect( pos, pos + ImVec2( width, height ), ImColor( 0.f, 0.f, 0.f, alpha * GetStyle( ).Alpha ), 4 );
}

bool c_custom::expand_button( const char* label, bool selected, float rounding, ImDrawFlags flags ) {
    if (!ImGui::GetCurrentContext()) return false;

    if (!expand_button_values) {
        expand_button_values = new std::unordered_map<ImGuiID, float>();
    }

    ImGuiWindow* window = GetCurrentWindow( );
    if (!window) return false;

    ImGuiID id = window->GetID( label );

    ImVec2 label_size = CalcTextSize( label, 0, 1 );
    ImVec2 icon_size = GetIO( ).Fonts->Fonts[1]->CalcTextSizeA( GetIO( ).Fonts->Fonts[1]->FontSize - 2, FLT_MAX, 0, ICON_FA_FORWARD );
    ImVec2 pos = window->DC.CursorPos;

    auto draw = window->DrawList;

    ImRect rect( pos, pos + ImVec2( GetWindowWidth( ), 45 ) );
    ItemAdd( rect, id );
    ItemSize( rect, GetStyle( ).FramePadding.y );

    bool hovered, held;
    bool pressed = ButtonBehavior( rect, id, &hovered, &held, NULL );

    static std::unordered_map < ImGuiID, float > values;
    auto value = expand_button_values->find(id);
    if (value == expand_button_values->end()) {
        expand_button_values->insert({id, 0.f});
        value = expand_button_values->find(id);
    }

    value->second = ImLerp( value->second, ( selected ? 1.f : 0.f ), 0.035f );

    draw->AddRectFilled( rect.Min, rect.Max, ImColor( 1.f, 1.f, 1.f, 0.025f * value->second ), rounding, flags );
    draw->AddText( ImVec2( rect.Min.x + 50 / 2 - icon_size.x / 2, rect.GetCenter( ).y - icon_size.y / 2 ), GetColorU32( ImGuiCol_Text, ( value->second > 0.3f ? value->second : 0.3f ) ), ICON_FA_FORWARD );
    draw->AddText( ImVec2( rect.Min.x + 50, rect.GetCenter( ).y - label_size.y / 2 ), GetColorU32( ImGuiCol_Text, ( value->second > 0.3f ? value->second : 0.3f ) ), label );

    return pressed;
}

bool c_custom::tab( const char* icon, const char* label, bool selected ) {
    if (!ImGui::GetCurrentContext()) return false;

    if (!tab_values) {
        tab_values = new std::unordered_map<ImGuiID, float>();
    }

    ImGuiWindow* window = GetCurrentWindow( );
    ImGuiID id = window->GetID( label );

    ImVec2 label_size = CalcTextSize( label, 0, 1 );
    ImVec2 icon_size = GetIO( ).Fonts->Fonts[1]->CalcTextSizeA( GetIO( ).Fonts->Fonts[1]->FontSize, FLT_MAX, 0, icon );
    ImVec2 pos = window->DC.CursorPos;

    auto draw = window->DrawList;

    ImRect rect( pos, pos + ImVec2( GetWindowWidth( ), 40 ) );
    ItemAdd( rect, id );
    ItemSize( rect, GetStyle( ).FramePadding.y );

    bool hovered, held;
    bool pressed = ButtonBehavior( rect, id, &hovered, &held, NULL );

    static std::unordered_map < ImGuiID, float > values;
    auto value = values.find( id );
    if ( value == values.end( ) ) {

        values.insert( { id, 0.f } );
        value = values.find( id );
    }

    value->second = ImLerp( value->second, ( selected || hovered ? 1.f : 0.f ), 0.035f );

    draw->AddText( ImVec2( rect.Min.x + 50 / 2 - icon_size.x / 2, rect.GetCenter( ).y - icon_size.y / 2 ), GetColorU32( ImGuiCol_Text, ( value->second > 0.3f ? value->second : 0.3f ) ), icon );
    draw->AddText( ImVec2( rect.Min.x + 50, rect.GetCenter( ).y - label_size.y / 2 ), GetColorU32( ImGuiCol_Text, ( value->second > 0.3f ? value->second : 0.3f ) ), label );

    return pressed;
}

bool c_custom::sub_tab( const char* label, bool selected ) {

    ImGuiWindow* window = GetCurrentWindow( );
    ImGuiID id = window->GetID( label );

    ImVec2 label_size = CalcTextSize( label, 0, 1 );
    ImVec2 pos = window->DC.CursorPos;

    auto draw = window->DrawList;

    ImRect rect( pos, pos + label_size );
    ItemAdd( rect, id );
    ItemSize( ImVec4( rect.Min.x, rect.Min.y, rect.Max.x + 15, rect.Max.y ), GetStyle( ).FramePadding.y );

    bool hovered, held;
    bool pressed = ButtonBehavior( rect, id, &hovered, &held, NULL );

    static std::unordered_map < ImGuiID, float > values;
    auto value = values.find( id );
    if ( value == values.end( ) ) {

        values.insert( { id, 0.f } );
        value = values.find( id );
    }

    value->second = ImLerp( value->second, ( selected ? 1.f : 0.f ), 0.035f );

    draw->AddText( rect.Min, GetColorU32( ImGuiCol_Text, ( value->second > 0.3f ? value->second : 0.3f ) ), label );
    draw->AddCircleFilled( ImVec2( rect.GetCenter( ).x, rect.Max.y + 10 ), 3 * value->second, get_accent_color( value->second * GetStyle( ).Alpha ) );

    return pressed;
}

struct tab_area_structure {

    bool hovered, should_expand;
    float width;
};

void c_custom::tab_area( const char* str_id, ImVec2 size, std::function < void ( ) > content ) {
    if (!ImGui::GetCurrentContext()) return;

    if (!area_values) {
        area_values = new std::unordered_map<ImGuiID, float>();
    }

    static std::unordered_map < std::string, tab_area_structure > values;
    auto value = values.find( std::string( str_id ) );
    if ( value == values.end( ) ) {

        values.insert( { std::string( str_id ), { false, false, size.x } } );
        value = values.find( std::string( str_id ) );
    }

    value->second.width = ImLerp( value->second.width, ( value->second.should_expand && value->second.hovered ) ? size.x + 85 : size.x, 0.08f );

    if ( ( value->second.should_expand && value->second.hovered ) )
        GetForegroundDrawList( )->AddRectFilled( GetWindowPos( ) + ImVec2( value->second.width, 0 ), GetWindowPos( ) + GetWindowSize( ), ImColor( 0.f, 0.f, 0.f, 0.2f ), GetStyle( ).WindowRounding );

    PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0, 0 ) );

    BeginChild( std::string( str_id ).append( ".child" ).c_str( ), ImVec2( value->second.width, size.y ) );

    value->second.hovered = IsMouseHoveringRect( GetCurrentWindow( )->Pos, GetCurrentWindow( )->Pos + ImVec2( value->second.width, size.y ) ); // could use IsWindowHovered( ); , but it bugs

    GetWindowDrawList( )->AddRectFilled( GetCurrentWindow( )->Pos, GetCurrentWindow( )->Pos + GetCurrentWindow( )->Size, ImColor( 28, 30, 36 ), GetStyle( ).WindowRounding, ImDrawFlags_RoundCornersTopLeft );

    if ( expand_button( "Expand", value->second.should_expand, GetStyle( ).WindowRounding, ImDrawFlags_RoundCornersTopLeft ) )
        value->second.should_expand = !value->second.should_expand;

    content( );

    EndChild( );

    PopStyleVar( );

}

void c_custom::begin_child( const char* name, ImVec2 size ) {

    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow( );

    ImVec2 pos = window->DC.CursorPos;

    BeginChild( std::string( name ).append( ".main" ).c_str( ), size, false, ImGuiWindowFlags_NoScrollbar );

    GetWindowDrawList( )->AddRectFilled( pos, pos + size, IM_COL32(28, 30, 36, (int)(custom.m_anim * 255)), 4 );
    GetWindowDrawList( )->AddText( pos + ImVec2( 10, 8 ), custom.get_accent_color( custom.m_anim ), name, FindRenderedTextEnd( name ) );

    SetCursorPosY( 30 );
    BeginChild( name, { size.x, size.y - 30 } );
    SetCursorPosX( 10 );

    BeginGroup( );

    PushStyleVar( ImGuiStyleVar_Alpha, custom.m_anim );
    PushStyleVar( ImGuiStyleVar_ItemSpacing, { 8, 8 } );
}

void c_custom::end_child( ) {

    PopStyleVar( 2 );
    EndGroup( );
    EndChild( );
    EndChild( );
}

void c_custom::topbar_setup(std::vector<const char*> a, int& b) {
    SetCursorPos(ImVec2(70, 13));
    BeginGroup();

    for (int i = 0; i < a.size(); ++i) {
        if (custom.sub_tab(a.at(i), b == i) && b != i)
            b = i, custom.m_anim = 0.f;

        if (i != a.size() - 1)
            SameLine();
    }

    EndGroup();
}
