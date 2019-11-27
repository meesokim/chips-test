/*
    UI implementation for spc1000.c, this must live in its own C++ file.
*/
extern "C" {
#include "common.h"
}
#include "imgui.h"
#include "chips/z80.h"
#include "chips/beeper.h"
#include "chips/ay38910.h"
#include "chips/mc6847.h"
#include "chips/kbd.h"
#include "chips/clk.h"
#include "chips/mem.h"
#include "systems/spc1000.h"
#define CHIPS_IMPL
#define UI_DASM_USE_Z80
#define UI_DBG_USE_Z80
#include "ui.h"
#include "util/z80dasm.h"
#include "ui/ui_util.h"
#include "ui/ui_chip.h"
#include "ui/ui_memedit.h"
#include "ui/ui_memmap.h"
#include "ui/ui_dasm.h"
#include "ui/ui_dbg.h"
#include "ui/ui_kbd.h"
#include "ui/ui_z80.h"
#include "ui/ui_ay38910.h"
#include "ui/ui_audio.h"
#define DUMP_NUM_ITEMS (13)
extern "C" {
    typedef struct { const char* name; const uint8_t* ptr; int size; } dump_item;
    extern dump_item dump_items[];
}
#include "ui/ui_spc1000.h"
#ifdef __clang__
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#endif

extern "C" {

extern spc1000_desc_t spc1000_desc(spc1000_type_t type, spc1000_joystick_type_t joy_type);

static double exec_time;
static ui_spc1000_t ui_spc1000;

/* reboot callback */
static void boot_cb(spc1000_t* sys, spc1000_type_t type) {
    spc1000_desc_t desc = spc1000_desc(type, sys->joystick_type);
    spc1000_init(sys, &desc);
}

void keybutton(const char *key, int space)
{
    if (ImGui::Button(key))
    {
        keybuf_put(key);
    }
    ImGui::SameLine();
    ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x + space, ImGui::GetCursorPos().y));    
}

void spc1000ui_draw(void) {
    ui_spc1000_draw(&ui_spc1000, exec_time);
    bool g_bMenuOpen = false;
    ImGui::SetNextWindowPos(ImVec2(0,(float)sapp_height()-60));
    ImGui::Begin("B", &g_bMenuOpen, ImVec2(0,(float)sapp_height()), 0.f, ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_AlwaysUseWindowPadding);
    ImGuiStyle& style = ImGui::GetStyle();    
    style.WindowBorderSize = 0.0f;  
    ImGui::SetWindowFocus("top"); 
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::SetWindowFontScale(2.0);
    keybutton("1",10);
    keybutton("2",10);
    keybutton("3",10);
    keybutton("S",20);
    keybutton("Y",10);
    keybutton("LOAD\n",100);
    keybutton("RUN\n",100);
#if 0    
    const float ItemSpacing = style.ItemSpacing.x;
    static float HostButtonWidth = 00.0f; //The 100.0f is just a guess size for the first frame.
    float pos = HostButtonWidth + ItemSpacing;
    ImGui::SetCursorPos(ImVec2(sapp_window_width() - pos,  ImGui::GetCursorPos().y));
    keybutton("/");
    HostButtonWidth = ImGui::GetItemRectSize().x;
    static float ClientButtonWidth = 100.0f;
    pos += ClientButtonWidth + ItemSpacing;
    ImGui::SameLine(sapp_window_width() - pos);
    keybutton(",");
    static float LocalButtonWidth = 100.0f;
    pos += LocalButtonWidth + ItemSpacing;
    ImGui::SameLine(sapp_window_width() - pos);
    keybutton("Z");
#else
    keybutton(",",100);
    keybutton("Z",100);    
    keybutton("/",50);
#endif    
    ImGui::End();
}

void spc1000ui_init(spc1000_t* spc1000) {
    ui_init(spc1000ui_draw);
    ui_spc1000_desc_t desc = {0};
    sapp_show_keyboard(true);
    desc.spc1000 = spc1000;
    desc.boot_cb = boot_cb;
    desc.create_texture_cb = gfx_create_texture;
    desc.update_texture_cb = gfx_update_texture;
    desc.destroy_texture_cb = gfx_destroy_texture;
    desc.dbg_keys.break_keycode = SAPP_KEYCODE_F5;
    desc.dbg_keys.break_name = "F5";
    desc.dbg_keys.continue_keycode = SAPP_KEYCODE_F5;
    desc.dbg_keys.continue_name = "F5";
    desc.dbg_keys.step_over_keycode = SAPP_KEYCODE_F6;
    desc.dbg_keys.step_over_name = "F6";
    desc.dbg_keys.step_into_keycode = SAPP_KEYCODE_F7;
    desc.dbg_keys.step_into_name = "F7";
    desc.dbg_keys.toggle_breakpoint_keycode = SAPP_KEYCODE_F9;
    desc.dbg_keys.toggle_breakpoint_name = "F9";
    ui_spc1000_init(&ui_spc1000, &desc);
}

void spc1000ui_discard(void) {
    ui_spc1000_discard(&ui_spc1000);
}

void spc1000ui_exec(spc1000_t* spc1000, uint32_t frame_time_us) {
    if (ui_spc1000_before_exec(&ui_spc1000)) {
        uint64_t start = stm_now();
        spc1000_exec(spc1000, frame_time_us);
        exec_time = stm_ms(stm_since(start));
        ui_spc1000_after_exec(&ui_spc1000);
    }
    else {
        exec_time = 0.0;
    }
}

} /* extern "C" */
