/* -*- C++ -*-
 *
 *  ScriptParser.cpp - Define block parser of ONScripter
 *
 *  Copyright (c) 2001-2011 Ogapee. All rights reserved.
 *  (original ONScripter, of which this is a fork).
 *
 *  ogapee@aqua.dti2.ne.jp
 *
 *  Copyright (c) 2007-2011 "Uncle" Mion Sonozaki
 *
 *  UncleMion@gmail.com
 *
 *  Copyright (c) 2023 Galladite
 *
 *  galladite@yandex.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, see <http://www.gnu.org/licenses/>
 *  or write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// Modified by Haeleth, Autumn 2006, to better support OS X/Linux packaging.

// Modified by Mion, March 2008, to update from
// Ogapee's 20080121 release source code.

// Modified by Mion, April 2009, to update from
// Ogapee's 20090331 release source code.

// Modified by Mion, November 2009, to update from
// Ogapee's 20091115 release source code.

#include "ScriptParser.h"
#include "Encoding.h"
#include "ShiftJISData.h"
#include <cstdio>
#include <cstdlib>

#ifdef MACOSX
#include "cocoa_bundle.h"
#include "cocoa_alertbox.h"
#endif
#ifdef WIN32
#include <windows.h>
#endif

#define VERSION_STR1 "ONScripter-EN"
#define VERSION_STR2 "Copyright (C) 2001-2010 Studio O.G.A., 2007-2010 \"Uncle\" Mion Sonozaki, 2023-2025 Galladite. All Rights Reserved."

#define DEFAULT_TEXT_SPEED_LOW    40
#define DEFAULT_TEXT_SPEED_MIDDLE 20
#define DEFAULT_TEXT_SPEED_HIGHT  10

#define MAX_PAGE_LIST 16

const char* DEFAULT_START_KINSOKU = (const char*)shiftjis_data::kinsoku_defaults::start_kinsoku;
const char* DEFAULT_END_KINSOKU = (const char*)shiftjis_data::kinsoku_defaults::end_kinsoku;

typedef int (ScriptParser::*FuncList)();
static struct FuncLUT{
    char command[30];
    FuncList method;
} func_lut[] = {
    {"zenkakko", &ScriptParser::zenkakkoCommand},
    {"windoweffect", &ScriptParser::effectCommand},
    {"windowchip", &ScriptParser::windowchipCommand},
    {"windowback", &ScriptParser::windowbackCommand},
    {"versionstr", &ScriptParser::versionstrCommand},
    {"usewheel", &ScriptParser::usewheelCommand},
    {"useescspc", &ScriptParser::useescspcCommand},
    {"underline", &ScriptParser::underlineCommand},
    {"transmode", &ScriptParser::transmodeCommand},
    {"time", &ScriptParser::timeCommand},
    {"textgosub", &ScriptParser::textgosubCommand},
    {"tan", &ScriptParser::tanCommand},
    {"sub", &ScriptParser::subCommand},
    {"stralias", &ScriptParser::straliasCommand},
    {"spi", &ScriptParser::soundpressplginCommand},
    {"soundpressplgin", &ScriptParser::soundpressplginCommand},
    {"skip",     &ScriptParser::skipCommand},
    {"sin", &ScriptParser::sinCommand},
    {"shadedistance",     &ScriptParser::shadedistanceCommand},
    {"setlayer", &ScriptParser::setlayerCommand},
    {"setkinsoku",   &ScriptParser::setkinsokuCommand},
    {"selectvoice",     &ScriptParser::selectvoiceCommand},
    {"selectcolor",     &ScriptParser::selectcolorCommand},
    {"savenumber",     &ScriptParser::savenumberCommand},
    {"savename",     &ScriptParser::savenameCommand},
    {"savedir",     &ScriptParser::savedirCommand},
    {"sar",    &ScriptParser::nsaCommand},
    {"rubyon2",    &ScriptParser::rubyonCommand},
    {"rubyon",    &ScriptParser::rubyonCommand},
    {"rubyoff",    &ScriptParser::rubyoffCommand},
    {"roff",    &ScriptParser::roffCommand},
    {"rmenu",    &ScriptParser::rmenuCommand},
    {"rgosub",   &ScriptParser::rgosubCommand},
    {"return",   &ScriptParser::returnCommand},
    {"pretextgosub", &ScriptParser::pretextgosubCommand},
    {"pagetag", &ScriptParser::pagetagCommand},
    {"numalias", &ScriptParser::numaliasCommand},
    {"nsadir",    &ScriptParser::nsadirCommand},
    {"nsa",    &ScriptParser::nsaCommand},
    {"notif",    &ScriptParser::ifCommand},
    {"next",    &ScriptParser::nextCommand},
    {"ns3",    &ScriptParser::nsaCommand},
    {"ns2",    &ScriptParser::nsaCommand},
    {"mul",      &ScriptParser::mulCommand},
    {"movl",      &ScriptParser::movCommand},
    {"mov10",      &ScriptParser::movCommand},
    {"mov9",      &ScriptParser::movCommand},
    {"mov8",      &ScriptParser::movCommand},
    {"mov7",      &ScriptParser::movCommand},
    {"mov6",      &ScriptParser::movCommand},
    {"mov5",      &ScriptParser::movCommand},
    {"mov4",      &ScriptParser::movCommand},
    {"mov3",      &ScriptParser::movCommand},
    {"mov",      &ScriptParser::movCommand},
    {"mode_wave_demo", &ScriptParser::mode_wave_demoCommand},
    {"mode_saya", &ScriptParser::mode_sayaCommand},
    {"mode_ext", &ScriptParser::mode_extCommand},
    {"mod",      &ScriptParser::modCommand},
    {"mid",      &ScriptParser::midCommand},
    {"menusetwindow",      &ScriptParser::menusetwindowCommand},
    {"menuselectvoice",      &ScriptParser::menuselectvoiceCommand},
    {"menuselectcolor",      &ScriptParser::menuselectcolorCommand},
    {"maxkaisoupage",      &ScriptParser::maxkaisoupageCommand},
    {"luasub",      &ScriptParser::luasubCommand},
    {"luacall",      &ScriptParser::luacallCommand},
    {"lookbacksp",      &ScriptParser::lookbackspCommand},
    {"lookbackcolor",      &ScriptParser::lookbackcolorCommand},
    //{"lookbackbutton",      &ScriptParser::lookbackbuttonCommand},
    {"log",      &ScriptParser::logCommand},
    {"loadgosub",      &ScriptParser::loadgosubCommand},
    {"linepage2",    &ScriptParser::linepageCommand},
    {"linepage",    &ScriptParser::linepageCommand},
    {"len",      &ScriptParser::lenCommand},
    {"labellog",      &ScriptParser::labellogCommand},
    {"labelexist",      &ScriptParser::labelexistCommand},
    {"kidokuskip", &ScriptParser::kidokuskipCommand},
    {"kidokumode", &ScriptParser::kidokumodeCommand},
    {"itoa2", &ScriptParser::itoaCommand},
    {"itoa", &ScriptParser::itoaCommand},
    {"intlimit", &ScriptParser::intlimitCommand},
    {"inc",      &ScriptParser::incCommand},
    {"if",       &ScriptParser::ifCommand},
    {"humanz",       &ScriptParser::humanzCommand},
    {"humanpos",       &ScriptParser::humanposCommand},
    {"goto",     &ScriptParser::gotoCommand},
    {"gosub",    &ScriptParser::gosubCommand},
    {"globalon",    &ScriptParser::globalonCommand},
    {"getparam",    &ScriptParser::getparamCommand},
    //{"game",    &ScriptParser::gameCommand},
    {"for",   &ScriptParser::forCommand},
    {"filelog",   &ScriptParser::filelogCommand},
    {"errorsave",   &ScriptParser::errorsaveCommand},
    {"english",   &ScriptParser::englishCommand},
    {"effectcut",   &ScriptParser::effectcutCommand},
    {"effectblank",   &ScriptParser::effectblankCommand},
    {"effect",   &ScriptParser::effectCommand},
    {"dsound",   &ScriptParser::dsoundCommand},
    {"div",   &ScriptParser::divCommand},
    {"dim",   &ScriptParser::dimCommand},
    {"defvoicevol",   &ScriptParser::defvoicevolCommand},
    {"defsub",   &ScriptParser::defsubCommand},
    {"defsevol",   &ScriptParser::defsevolCommand},
    {"defmp3vol",   &ScriptParser::defmp3volCommand},
    {"defbgmvol",   &ScriptParser::defmp3volCommand},
    {"defaultspeed", &ScriptParser::defaultspeedCommand},
    {"defaultfont", &ScriptParser::defaultfontCommand},
    {"dec",   &ScriptParser::decCommand},
    {"dec",   &ScriptParser::decCommand},
    {"date",   &ScriptParser::dateCommand},
    {"csvwrite",   &ScriptParser::csvwriteCommand},
    {"csvread",   &ScriptParser::csvreadCommand},
    {"csvopen",   &ScriptParser::csvopenCommand},
    {"csveof",   &ScriptParser::csveofCommand},
    {"csvdelete",   &ScriptParser::csvdeleteCommand},
    {"csvclose",   &ScriptParser::csvcloseCommand},
    {"cos", &ScriptParser::cosCommand},
    {"cmp",      &ScriptParser::cmpCommand},
    {"clickvoice",   &ScriptParser::clickvoiceCommand},
    {"clickstr",   &ScriptParser::clickstrCommand},
    {"clickskippage", &ScriptParser::clickskippageCommand},
    {"btnnowindowerase",   &ScriptParser::btnnowindoweraseCommand},
    {"break",   &ScriptParser::breakCommand},
    {"autosaveoff", &ScriptParser::autosaveoffCommand},
    {"automode", &ScriptParser::mode_extCommand},
    {"atoi",      &ScriptParser::atoiCommand},
    {"arc",      &ScriptParser::arcCommand},
    {"addnsadir",    &ScriptParser::addnsadirCommand},
    {"addkinsoku",   &ScriptParser::addkinsokuCommand},
    {"add",      &ScriptParser::addCommand},
    {"", NULL}
};

static struct FuncHash{
    int start;
    int end;
} func_hash['z'-'a'+1];

ScriptParser::ScriptParser()
//Using an initialization list to make sure pointers start out NULL
:
#ifdef MACOSX
  bundle_res_path(NULL), bundle_app_path(NULL), bundle_app_name(NULL),
#endif
  cmdline_game_id(NULL), last_nest_info(NULL), version_str(NULL),
  savedir(NULL), last_effect_link(NULL),
#ifndef NO_LAYER_EFFECTS
  layer_info(NULL),
#endif
  save_menu_name(NULL), load_menu_name(NULL), save_item_name(NULL),
  save_data_buf(NULL), file_io_buf(NULL), default_env_font(NULL),
  page_list(NULL), start_page(NULL), current_page(NULL),
  start_kinsoku(NULL), end_kinsoku(NULL), current_font(NULL),
  textgosub_label(NULL), pretextgosub_label(NULL),
  loadgosub_label(NULL), rgosub_label(NULL), key_table(NULL)
{
    resetDefineFlags();

    debug_level = 0;
    srand( time(NULL) );
    rand();

#ifdef MACOSX
    is_bundled = false;
#endif
    nsa_offset = 0;
    force_button_shortcut_flag = false;
    
    file_io_buf_ptr = 0;
    file_io_buf_len = 0;
    save_data_len = 0;

    CSVInfo.fp = NULL;
    CSVInfo.mode = csvinfo::NONE;
    CSVInfo.contents = NULL;
    CSVInfo.contents_ptr = NULL;

    /* ---------------------------------------- */
    /* Sound related variables */
    int i;
    for ( i=0 ; i<     CLICKVOICE_NUM ; i++ )
             clickvoice_file_name[i] = NULL;
    for ( i=0 ; i<    SELECTVOICE_NUM ; i++ )
            selectvoice_file_name[i] = NULL;
    for ( i=0 ; i<MENUSELECTVOICE_NUM ; i++ )
        menuselectvoice_file_name[i] = NULL;

    //Default kinsoku
    num_start_kinsoku = num_end_kinsoku = 0;

    if(script_h.enc.getEncoding() == Encoding::CODE_CP932)
        setKinsoku(DEFAULT_START_KINSOKU, DEFAULT_END_KINSOKU, false, Encoding::CODE_CP932);
    else
        setKinsoku(DEFAULT_START_KINSOKU, DEFAULT_END_KINSOKU, false, Encoding::CODE_UTF8);

    /*
    setKinsoku(DEFAULT_START_KINSOKU, DEFAULT_END_KINSOKU, false,
        script_h.enc.getEncoding() == Encoding::CODE_CP932 ? Encoding::CODE_CP932
                                                           : Encoding::CODE_UTF8);
    */

    //onscripter script syntax options (for running some older nscr games)
    allow_color_type_only = false;
    set_tag_page_origin_to_1 = false;
    answer_dialog_with_yes_ok = false;

#ifndef NO_LAYER_EFFECTS
    use_layers = true;
#endif

#ifdef RCA_SCALE
    scr_stretch_x = scr_stretch_y = 1.0;
#endif
    preferred_width = 0;

    errorsave = false;

    //initialize cmd function table hash
    for (i='z'-'a' ; i>=0 ; i--){
        func_hash[i].start = -1;
        func_hash[i].end = -2;
    }
    int idx = 0;
    while (func_lut[idx].method){
        int j = func_lut[idx].command[0]-'a';
        if (func_hash[j].start == -1) func_hash[j].start = idx;
        func_hash[j].end = idx;
        idx++;
    }
}

ScriptParser::~ScriptParser()
{
    reset();

    if (version_str) delete[] version_str;
    if (save_menu_name) delete[] save_menu_name;
    if (load_menu_name) delete[] load_menu_name;
    if (save_item_name) delete[] save_item_name;

    if (file_io_buf) delete[] file_io_buf;
    if (save_data_buf) delete[] save_data_buf;

    if (start_kinsoku) delete[] start_kinsoku;
    if (end_kinsoku) delete[] end_kinsoku;

#ifdef MACOSX
    if (bundle_res_path) delete[] bundle_res_path;
    if (bundle_app_path) delete[] bundle_app_path;
    if (bundle_app_name) delete[] bundle_app_name;
#endif
    if (cmdline_game_id) delete[] cmdline_game_id;
    if (savedir) delete[] savedir;
}

void ScriptParser::reset()
{
    resetDefineFlags();

    int i;
    for (i='z'-'a' ; i>=0 ; i--){
        UserFuncHash &ufh = user_func_hash[i];
        UserFuncLUT *func = ufh.root.next;
        while(func){
            UserFuncLUT *tmp = func;
            func = func->next;
            delete tmp;
        }
        ufh.root.next = NULL;
        ufh.last = &ufh.root;
    }

    // reset misc variables
    nsa_path = DirPaths();

    if (CSVInfo.fp != NULL) fclose(CSVInfo.fp);
    CSVInfo.fp = NULL;
    CSVInfo.mode = csvinfo::NONE;
    if (CSVInfo.contents != NULL) free(CSVInfo.contents);
    CSVInfo.contents = NULL;
    CSVInfo.contents_ptr = NULL;

    if (version_str) delete[] version_str;
    version_str = new char[strlen(VERSION_STR1)+
                           strlen("\n")+
                           strlen(VERSION_STR2)+
                           strlen("\n")+
                           +1];
    sprintf( version_str, "%s\n%s\n", VERSION_STR1, VERSION_STR2 );

    /* Text related variables */
    sentence_font.reset();
    menu_font.reset();
    ruby_font.reset();
    current_font = &sentence_font;

    if (page_list){
        delete[] page_list;
        page_list = NULL;
    }
    //current_page & start_page point to page_list elements
    current_page = start_page = NULL;
    
    textgosub_label = NULL;
    pretextgosub_label = NULL;
    loadgosub_label = NULL;
    rgosub_label = NULL;

    /* ---------------------------------------- */
    /* Sound related variables */
    for ( i=0 ; i<     CLICKVOICE_NUM ; i++ )
        setStr(&clickvoice_file_name[i], NULL);
    for ( i=0 ; i<    SELECTVOICE_NUM ; i++ )
        setStr(&selectvoice_file_name[i], NULL);
    for ( i=0 ; i<MENUSELECTVOICE_NUM ; i++ )
        setStr(&menuselectvoice_file_name[i], NULL);

    /* Menu related variables */
    setDefaultMenuLabels();
    deleteRMenuLink();

    /* Effect related variables */
    EffectLink *link = root_effect_link.next;
    while(link){
        EffectLink *tmp = link;
        link = link->next;
        delete tmp;
    }
    last_effect_link = &root_effect_link;
    last_effect_link->next = NULL;

#ifndef NO_LAYER_EFFECTS
    deleteLayerInfo();
#endif

    readLog( script_h.log_info[ScriptHandler::LABEL_LOG] );
}

void ScriptParser::resetDefineFlags()
{
    globalon_flag = false;
    labellog_flag = false;
    filelog_flag = false;
    kidokuskip_flag = false;
    clickskippage_flag = false;
    autosaveoff_flag = false;

    rmode_flag = true;
    windowback_flag = false;
    btnnowindowerase_flag = false;
    usewheel_flag = false;
    useescspc_flag = false;
    mode_wave_demo_flag = false;
    mode_saya_flag = false;
    //NScr 2.82+ enables mode_ext (automode) by default, let's do so too
    mode_ext_flag = true;
    rubyon_flag = rubyon2_flag = false;
    pagetag_flag = false;
    windowchip_sprite_no = -1;
    string_buffer_offset = 0;

    break_flag = false;
    trans_mode = AnimationInfo::TRANS_TOPLEFT;

    z_order = 499;

    /* ---------------------------------------- */
    /* Lookback related variables */
    lookback_sp[0] = lookback_sp[1] = -1;
    lookback_color[0] = 0xff;
    lookback_color[1] = 0xff;
    lookback_color[2] = 0x00;

    /* ---------------------------------------- */
    /* Save/Load related variables */
    num_save_file = 9;

    /* ---------------------------------------- */
    /* Text related variables */
    shade_distance[0] = 1;
    shade_distance[1] = 1;
    
    default_text_speed[0] = DEFAULT_TEXT_SPEED_LOW;
    default_text_speed[1] = DEFAULT_TEXT_SPEED_MIDDLE;
    default_text_speed[2] = DEFAULT_TEXT_SPEED_HIGHT;
    max_page_list = MAX_PAGE_LIST+1;
    num_chars_in_sentence = 0;

    clickstr_line = 0;
    clickstr_state = CLICK_NONE;
    linepage_mode = 0;
    english_mode = false;
    
    /* ---------------------------------------- */
    /* Menu related variables */
    menu_font.font_size_xy[0] = DEFAULT_FONT_SIZE;
    menu_font.font_size_xy[1] = DEFAULT_FONT_SIZE;
    menu_font.top_xy[0] = 0;
    menu_font.top_xy[1] = 16;
    menu_font.num_xy[0] = 32;
    menu_font.num_xy[1] = 23;
    menu_font.pitch_xy[0] = menu_font.font_size_xy[0];
    menu_font.pitch_xy[1] = 2 + menu_font.font_size_xy[1];
    menu_font.window_color[0] = menu_font.window_color[1] = menu_font.window_color[2] = 0xcc;

    /* ---------------------------------------- */
    /* Effect related variables */
    effect_blank = 10;
    effect_cut_flag = false;

    window_effect.effect = 1;
    window_effect.duration = 0;
    root_effect_link.no = 0;
    root_effect_link.effect = 0;
    root_effect_link.duration = 0;

    current_mode = DEFINE_MODE;
}

int ScriptParser::open()
{
    script_h.cBR = new DirectReader( archive_path, key_table );
    script_h.cBR->open();

    script_h.game_identifier = cmdline_game_id;
    cmdline_game_id = NULL;

    if ( script_h.readScript( archive_path ) ) return -1;

    /* All the code used for setting the screen side and such have
     * been moved to a new function, open_screen (beneath). This is
     * because for variable screen resolutions, we need access to the
     * save path before setting the resolution.
     * -Galladite 2023-10-20
     */

    return 0;
}

int ScriptParser::open_screen()
{
    if (script_h.screen_width == -1 || script_h.screen_height == -1) {
        script_width=800;
        script_height=800;

        int x=640, y=480;

        if ( loadFileIOBuf( "screen.dat" ) == 0 ) {
            // We need to be able to read at least 8 bytes
            if ( !( file_io_buf_ptr+7 >= file_io_buf_len ) ) {
                x =
                    (unsigned int)file_io_buf[file_io_buf_ptr+3] << 24 |
                    (unsigned int)file_io_buf[file_io_buf_ptr+2] << 16 |
                    (unsigned int)file_io_buf[file_io_buf_ptr+1] << 8 |
                    (unsigned int)file_io_buf[file_io_buf_ptr];
                file_io_buf_ptr += 4;

                y =
                    (unsigned int)file_io_buf[file_io_buf_ptr+3] << 24 |
                    (unsigned int)file_io_buf[file_io_buf_ptr+2] << 16 |
                    (unsigned int)file_io_buf[file_io_buf_ptr+1] << 8 |
                    (unsigned int)file_io_buf[file_io_buf_ptr];
                file_io_buf_ptr += 4;
            }
        }

        // If we didn't read these properly, we should use the
        // default 640x480 set above
        //
        // This also needs to be set correctly for it to be able
        // to be read by getres
        script_h.screen_width = x;
        script_h.screen_height = y;
    }

    script_width = script_h.screen_width;
    script_height = script_h.screen_height;

#ifndef PDA
    screen_ratio1 = 1;
    screen_ratio2 = 1;
    if (preferred_width > 0) {
        screen_ratio1 = preferred_width;
        screen_ratio2 = script_width;
    }

    screen_width  = script_width * screen_ratio1 / screen_ratio2;
    screen_height = script_height * screen_ratio1 / screen_ratio2;
#else
    if ((double)script_width/script_height != (4.0/3)) {
        printf("Kill process here\n");
    }
    screen_ratio1 = 1; // PDA build is almost certainly broken :(
    screen_ratio2 = 1; // To be fixed, valid values for these 2 variables would have to be calculated, however if the ratio isn't 4:3 it's not going to work.
    screen_width = 320;
    screen_height = 240;
#endif

    underline_value = script_height - 1;
    for (int i=0; i<3; i++)
        humanpos[i] = (script_width/4) * (i+1);
    if (debug_level > 0)
        printf("humanpos: %d,%d,%d; underline: %d\n", humanpos[0], humanpos[1],
               humanpos[2], underline_value);

    return 0;
}

#ifdef MACOSX
void ScriptParser::checkBundled()
{
    // check whether this onscripter is bundled, and if so find the
    // resources and app directories
    
    ONSCocoa::getBundleInfo(&bundle_res_path, &bundle_app_path, &bundle_app_name);
    is_bundled = true; // always bundled on OS X
}
#endif

unsigned char ScriptParser::convHexToDec( char ch )
{
    if      ( '0' <= ch && ch <= '9' ) return ch - '0';
    else if ( 'a' <= ch && ch <= 'f' ) return ch - 'a' + 10;
    else if ( 'A' <= ch && ch <= 'F' ) return ch - 'A' + 10;
    else errorAndExit("convHexToDec: not valid character for color.");

    return 0;
}

void ScriptParser::setColor( uchar3 &dstcolor, uchar3 srccolor )
{
    for (int i=0; i<3; i++)
        dstcolor[i] = srccolor[i];
}

void ScriptParser::readColor( uchar3 *color, const char *buf ){
    if ( buf[0] != '#' ) errorAndExit("readColor: no preceding #.");
    (*color)[0] = convHexToDec( buf[1] ) << 4 | convHexToDec( buf[2] );
    (*color)[1] = convHexToDec( buf[3] ) << 4 | convHexToDec( buf[4] );
    (*color)[2] = convHexToDec( buf[5] ) << 4 | convHexToDec( buf[6] );
}

void ScriptParser::add_debug_level()
{
    debug_level++;
}

void ScriptParser::errorAndCont( const char *str, const char *reason, const char *title, bool is_simple )
{
    if (title == NULL)
        title = "Parse Issue";
    script_h.processError(str, title, reason, true, is_simple);
}

void ScriptParser::errorAndExit( const char *str, const char *reason, const char *title, bool is_simple )
{
    if (title == NULL)
        title = "Parse Error";
    script_h.processError(str, title, reason, false, is_simple);
}

int ScriptParser::parseLine()
{
    const char *cmd = script_h.getStringBuffer();

    if ( (debug_level > 0) && (*cmd != ':') && (*cmd != 0x0a) ) {
        printf("ScriptParser::Parseline %s\n", cmd );
        fflush(stdout);
    }

    script_h.current_cmd[0] = '\0';
    if ( script_h.isText() || script_h.isPretext() ) return RET_NOMATCH;
    script_h.current_cmd_type = ScriptHandler::CMD_NONE;

    if ( *cmd == ';' ) return RET_CONTINUE;
    else if ( *cmd == '*' ) return RET_CONTINUE;
    else if ( *cmd == ':' ) return RET_CONTINUE;

    if (*cmd != '_'){
        snprintf(script_h.current_cmd, 64, "%s", cmd);
        //Check against user-defined cmds
        if (cmd[0] >= 'a' && cmd[0] <= 'z'){
            UserFuncHash &ufh = user_func_hash[cmd[0]-'a'];
            UserFuncLUT *uf = ufh.root.next;
            while(uf){
                if (!strcmp( uf->command, cmd )){
                    if (uf->lua_flag){
#ifdef USE_LUA
                        if (lua_handler.callFunction(false, cmd))
                            errorAndExit( lua_handler.error_str, NULL, "Lua Error" );
#endif
                    }
                    else{
                        gosubReal( cmd, script_h.getNext() );
                    }
                    return RET_CONTINUE;
                }
                uf = uf->next;
            }
        }
    }
    else{
        cmd++;
    }

    //Check against builtin cmds
    if (cmd[0] >= 'a' && cmd[0] <= 'z'){
        FuncHash &fh = func_hash[cmd[0]-'a'];
        for (int i=fh.start ; i<=fh.end ; i++){
            if ( !strcmp( func_lut[i].command, cmd ) ){
                return (this->*func_lut[i].method)();
            }
        }
    }

    return RET_NOMATCH;
}

void ScriptParser::deleteRMenuLink()
{
    RMenuLink *link = root_rmenu_link.next;
    while(link){
        RMenuLink *tmp = link;
        link = link->next;
        delete tmp;
    }
    root_rmenu_link.next = NULL;

    rmenu_link_num   = 0;
    rmenu_link_width = 0;
}

int ScriptParser::getSystemCallNo( const char *buffer )
{
    if      ( !strcmp( buffer, "skip" ) )        return SYSTEM_SKIP;
    else if ( !strcmp( buffer, "reset" ) )       return SYSTEM_RESET;
    else if ( !strcmp( buffer, "save" ) )        return SYSTEM_SAVE;
    else if ( !strcmp( buffer, "load" ) )        return SYSTEM_LOAD;
    else if ( !strcmp( buffer, "lookback" ) )    return SYSTEM_LOOKBACK;
    else if ( !strcmp( buffer, "windowerase" ) ) return SYSTEM_WINDOWERASE;
    else if ( !strcmp( buffer, "rmenu" ) )       return SYSTEM_MENU;
    else if ( !strcmp( buffer, "automode" ) )    return SYSTEM_AUTOMODE;
    else if ( !strcmp( buffer, "end" ) )         return SYSTEM_END;
    else{
        printf("Unsupported system call %s\n", buffer );
        return -1;
    }
}

void ScriptParser::setArchivePath(const char *path)
{
    archive_path = DirPaths(path);
    //printf("archive_path: %s\n", archive_path.get_all_paths());
}

void ScriptParser::setSavePath(const char *path)
{
    if ( (path == NULL) || (*path == '\0') ||
         (path[strlen(path)-1] == DELIMITER) ) {
        setStr( &script_h.save_path, path );
    } else {
        if (script_h.save_path) delete[] script_h.save_path;
        script_h.save_path = new char[ strlen(path) + 2 ];
        sprintf( script_h.save_path, "%s%c", path, DELIMITER );
    }
    if (debug_level > 0) {
        printf("setting save path to '%s'\n", script_h.save_path);
        if (debug_level > 1) {
            //dump the byte values (for debugging cmd-line codepage settings)
            printf("save_path:");
            for (unsigned int i=0; i<strlen(script_h.save_path); i++)
                printf(" %02x", (unsigned char) script_h.save_path[i]);
            printf("\n");
        }
    }
}

void ScriptParser::setNsaOffset(const char *off)
{
    int offset = atoi(off);
    if (offset > 0)
        nsa_offset = offset;
}

void ScriptParser::saveGlovalData(bool no_error)
{
    if ( !globalon_flag ) return;

    file_io_buf_ptr = 0;
    writeVariables( script_h.global_variable_border, VARIABLE_RANGE, false );
    allocFileIOBuf();
    writeVariables( script_h.global_variable_border, VARIABLE_RANGE, true );

    if (saveFileIOBuf( "gloval.sav" ) && !no_error)
        errorAndExit( "can't open 'gloval.sav' for writing", NULL, "I/O Error", true );
}

void ScriptParser::allocFileIOBuf()
{
    if (file_io_buf_ptr > file_io_buf_len){
        file_io_buf_len = file_io_buf_ptr;
        if (file_io_buf) delete[] file_io_buf;
        file_io_buf = new unsigned char[file_io_buf_len];

        if (save_data_buf){
            memcpy(file_io_buf, save_data_buf, save_data_len);
            delete[] save_data_buf;
        }
        save_data_buf = new unsigned char[file_io_buf_len];
        memcpy(save_data_buf, file_io_buf, save_data_len);
    }
    file_io_buf_ptr = 0;
}

int ScriptParser::saveFileIOBuf( const char *filename, int offset, const char *savestr )
{
    FILE *fp;
    int retval = 0;
    size_t ret = 0;
    bool usesavedir = true;
    // all files except envdata go in savedir
    if (!strcmp( filename, "envdata" ))
        usesavedir = false;

    //Mion: create a temporary file, to avoid overwriting valid files
    // (if an error occurs)
    const char *root = script_h.save_path;
    if (usesavedir && script_h.savedir)
        root = script_h.savedir;

    char *fullname = new char[strlen(root)+strlen(filename)+1];
    sprintf( fullname, "%s%s", root, filename );
    char *tmp = new char[strlen(fullname) + 9];
    sprintf(tmp, "%s.tmpfile", fullname);

    if ( (fp = ::fopen( tmp, "wb" )) == NULL ) {
        retval = -1;
        goto save_io_cleanup;
    }

    ret = fwrite(file_io_buf+offset, 1, file_io_buf_ptr-offset, fp);

    if (savestr){
        size_t savelen = strlen(savestr);
        if ( (fputc('"', fp) == EOF) ||
             (fwrite(savestr, 1, savelen, fp) != savelen) ||
             (fputs("\"*", fp) == EOF) ) {
            snprintf(script_h.errbuf, MAX_ERRBUF_LEN,
                     "error writing to '%s'", filename);
            errorAndCont( script_h.errbuf, NULL, "I/O Issue" );
        }
    }

    fclose(fp);

    if (ret != file_io_buf_ptr-offset) {
        retval = -2;
        goto save_io_cleanup;
    }

    //now rename the tmp file and see if errors occur
    //(using "ret =" to avoid compiler warnings about unused return values)
    ret = remove(fullname); //ignore errors (like if fullname doesn't exist)
    if (rename(tmp, fullname)) {
        retval = -1;
        goto save_io_cleanup;
    }

  save_io_cleanup:
    delete[] fullname;
    delete[] tmp;

    return retval;
}

int ScriptParser::loadFileIOBuf( const char *filename )
{
    FILE *fp;
    bool usesavedir = true;
    if (!strcmp( filename, "envdata" ))
        usesavedir = false;
    if ( (fp = fopen( filename, "rb", true, usesavedir )) == NULL )
        return -1;
    
    fseek(fp, 0, SEEK_END);
    size_t len = ftell(fp);
    file_io_buf_ptr = len+1;
    allocFileIOBuf();

    fseek(fp, 0, SEEK_SET);
    size_t ret = fread(file_io_buf, 1, len, fp);
    fclose(fp);
    file_io_buf[len] = 0;

    if (ret != len) return -2;
    
    return 0;
}

void ScriptParser::writeChar(char c, bool output_flag)
{
    if (output_flag)
        file_io_buf[file_io_buf_ptr] = (unsigned char)c;
    file_io_buf_ptr++;
}

char ScriptParser::readChar()
{
    if (file_io_buf_ptr >= file_io_buf_len ) return 0;
    return (char)file_io_buf[file_io_buf_ptr++];
}

void ScriptParser::writeInt(int i, bool output_flag)
{
    if (output_flag){
        file_io_buf[file_io_buf_ptr++] = i & 0xff;
        file_io_buf[file_io_buf_ptr++] = (i >> 8) & 0xff;
        file_io_buf[file_io_buf_ptr++] = (i >> 16) & 0xff;
        file_io_buf[file_io_buf_ptr++] = (i >> 24) & 0xff;
    }
    else{
        file_io_buf_ptr += 4;
    }
}

int ScriptParser::readInt()
{
    if (file_io_buf_ptr+3 >= file_io_buf_len ) return 0;
    
    int i =
        (unsigned int)file_io_buf[file_io_buf_ptr+3] << 24 |
        (unsigned int)file_io_buf[file_io_buf_ptr+2] << 16 |
        (unsigned int)file_io_buf[file_io_buf_ptr+1] << 8 |
        (unsigned int)file_io_buf[file_io_buf_ptr];
    file_io_buf_ptr += 4;

    return i;
}

void ScriptParser::writeStr(char *s, bool output_flag)
{
    if ( s && s[0] ){
        if (output_flag)
            memcpy( file_io_buf + file_io_buf_ptr,
                    s,
                    strlen(s) );
        file_io_buf_ptr += strlen(s);
    }
    writeChar( 0, output_flag );
}

void ScriptParser::readStr(char **s)
{
    int counter = 0;

    while (file_io_buf_ptr+counter < file_io_buf_len){
        if (file_io_buf[file_io_buf_ptr+counter++] == 0) break;
    }
    
    if (*s) delete[] *s;
    *s = NULL;
    
    if (counter > 1){
        *s = new char[counter+1];
        memcpy(*s, file_io_buf + file_io_buf_ptr, counter);
        (*s)[counter] = 0;
    }
    file_io_buf_ptr += counter;
}

void ScriptParser::writeVariables( int from, int to, bool output_flag )
{
    for (int i=from ; i<to ; i++){
        writeInt( script_h.getVariableData(i).num, output_flag );
        writeStr( script_h.getVariableData(i).str, output_flag );
    }
}

void ScriptParser::readVariables( int from, int to )
{
    for (int i=from ; i<to ; i++){
        script_h.getVariableData(i).num = readInt();
        readStr( &script_h.getVariableData(i).str );
    }
}

void ScriptParser::writeArrayVariable( bool output_flag )
{
    ScriptHandler::ArrayVariable *av = script_h.getRootArrayVariable();

    while(av){
        int i, dim = 1;
        for ( i=0 ; i<av->num_dim ; i++ )
            dim *= av->dim[i];
        
        for ( i=0 ; i<dim ; i++ ){
            unsigned long ch = av->data[i];
            if (output_flag){
                file_io_buf[file_io_buf_ptr+3] = (unsigned char)((ch>>24) & 0xff);
                file_io_buf[file_io_buf_ptr+2] = (unsigned char)((ch>>16) & 0xff);
                file_io_buf[file_io_buf_ptr+1] = (unsigned char)((ch>>8)  & 0xff);
                file_io_buf[file_io_buf_ptr]   = (unsigned char)(ch & 0xff);
            }
            file_io_buf_ptr += 4;
        }
        av = av->next;
    }
}

void ScriptParser::readArrayVariable()
{
    ScriptHandler::ArrayVariable *av = script_h.getRootArrayVariable();

    while(av){
        int i, dim = 1;
        for ( i=0 ; i<av->num_dim ; i++ )
            dim *= av->dim[i];
        
        for ( i=0 ; i<dim ; i++ ){
            unsigned long ret;
            if (file_io_buf_ptr+3 >= file_io_buf_len ) return;
            ret = file_io_buf[file_io_buf_ptr+3];
            ret = ret << 8 | file_io_buf[file_io_buf_ptr+2];
            ret = ret << 8 | file_io_buf[file_io_buf_ptr+1];
            ret = ret << 8 | file_io_buf[file_io_buf_ptr];
            file_io_buf_ptr += 4;
            av->data[i] = ret;
        }
        av = av->next;
    }
}

void ScriptParser::writeLog( ScriptHandler::LogInfo &info )
{
    file_io_buf_ptr = 0;
    bool output_flag = false;
    for (int n=0 ; n<2 ; n++){
        int  i,j;
        char buf[10];

        sprintf( buf, "%d", info.num_logs );
        for ( i=0 ; i<(int)strlen( buf ) ; i++ ) writeChar( buf[i], output_flag );
        writeChar( 0x0a, output_flag );

        ScriptHandler::LogLink *cur = info.root_log.next;
        for ( i=0 ; i<info.num_logs ; i++ ){
            writeChar( '"', output_flag );
            for ( j=0 ; j<(int)strlen( cur->name ) ; j++ )
                writeChar( cur->name[j] ^ 0x84, output_flag );
            writeChar( '"', output_flag );
            cur = cur->next;
        }

        if (n==1) break;
        allocFileIOBuf();
        output_flag = true;
    }

    if (saveFileIOBuf( info.filename )){
        snprintf(script_h.errbuf, MAX_ERRBUF_LEN,
                 "can't write to '%s'", info.filename);
        errorAndExit( script_h.errbuf, NULL, "I/O Error" );
    }
}

void ScriptParser::readLog( ScriptHandler::LogInfo &info )
{
    script_h.resetLog( info );
    
    if (loadFileIOBuf( info.filename ) == 0){
        int i, j, ch, count = 0;
        char buf[100];

        while( (ch = readChar()) != 0x0a ){
            count = count * 10 + ch - '0';
        }

        for ( i=0 ; i<count ; i++ ){
            readChar();
            j = 0; 
            while( (ch = readChar()) != '"' ) buf[j++] = ch ^ 0x84;
            buf[j] = '\0';

            script_h.findAndAddLog( info, buf, true );
        }
    }
}

void ScriptParser::deleteNestInfo()
{
    NestInfo *info = root_nest_info.next;
    while(info){
        NestInfo *tmp = info;
        info = info->next;
        delete tmp;
    }
    root_nest_info.next = NULL;
    last_nest_info = &root_nest_info;
}

#ifndef NO_LAYER_EFFECTS
void ScriptParser::deleteLayerInfo()
{
    while (layer_info) {
        LayerInfo *tmp = layer_info;
        layer_info = layer_info->next;
        delete tmp;
    }
}
#endif

void ScriptParser::setStr( char **dst, const char *src, int num, bool to_utf8 )
{
    if ( *dst ) delete[] *dst;
    *dst = NULL;
    
    if ( src ){
        if ( num >= 0 ){
            *dst = new char[ num + 1 ];
            memcpy( *dst, src, num );
            (*dst)[num] = '\0';
        }
        else{
            num = strlen(src);

            if ( to_utf8 && script_h.enc.getEncoding() == Encoding::CODE_UTF8 ) {
                char *tmp_buf = new char[ num*2 + 1 ];
                DirectReader::convertFromSJISToUTF8(tmp_buf, src);
                num = strlen(tmp_buf);
                *dst = new char[ num + 1 ];
                strcpy(*dst, tmp_buf);
                delete[] tmp_buf;
            }
            else {
                *dst = new char[ num + 1 ];
                strcpy( *dst, src );
            }
        }
    }
}

void ScriptParser::setCurrentLabel( const char *label )
{
    current_label_info = script_h.lookupLabel( label );
    current_line = script_h.getLineByAddress( current_label_info.start_address );
    script_h.setCurrent( current_label_info.start_address );
}

int ScriptParser::readEffect( EffectLink *effect )
{
    int num = 1;
    
    effect->effect = script_h.readInt();
    if ( script_h.getEndStatus() & ScriptHandler::END_COMMA ){
        num++;
        effect->duration = script_h.readInt();
        if ( script_h.getEndStatus() & ScriptHandler::END_COMMA ){
            num++;
            const char *buf = script_h.readStr();
            effect->anim.setImageName( buf );
        }
        else
            effect->anim.remove();
    }
    else if (effect->effect < 0 || effect->effect > 255){
        snprintf(script_h.errbuf, MAX_ERRBUF_LEN,
                 "effect %d out of range, changing to 0", effect->effect);
        errorAndCont( script_h.errbuf );
        effect->effect = 0; // to suppress error
    }

    //printf("readEffect %d: %d %d %s\n", num, effect->effect, effect->duration, effect->anim.image_name );
    return num;
}

ScriptParser::EffectLink *ScriptParser::parseEffect(bool init_flag)
{
    if (init_flag) tmp_effect.anim.remove();

    int num = readEffect(&tmp_effect);

    if (num > 1) return &tmp_effect;
    if (tmp_effect.effect == 0 || tmp_effect.effect == 1) return &tmp_effect;

    EffectLink *link = &root_effect_link;
    while(link){
        if (link->no == tmp_effect.effect) return link;
        link = link->next;
    }

    snprintf(script_h.errbuf, MAX_ERRBUF_LEN,
             "effect %d not found", tmp_effect.effect);
    errorAndExit( script_h.errbuf );

    return NULL;
}

FILE *ScriptParser::fopen( const char *path, const char *mode, const bool save, const bool usesavedir )
{
    const char* root;
    char *file_name;
    FILE *fp = NULL;

    if (usesavedir && script_h.savedir) {
        root = script_h.savedir;
        file_name = new char[strlen(root)+strlen(path)+1];
        sprintf( file_name, "%s%s", root, path );
        //printf("parser:fopen(\"%s\")\n", file_name);

        fp = ::fopen( file_name, mode );
    } else if (save) {
        root = script_h.save_path;
        file_name = new char[strlen(root)+strlen(path)+1];
        sprintf( file_name, "%s%s", root, path );
        //printf("parser:fopen(\"%s\")\n", file_name);

        fp = ::fopen( file_name, mode );
    } else {
        // search within archive_path dirs
        file_name = new char[archive_path.max_path_len()+strlen(path)+1];
        for (int n=0; n<(archive_path.get_num_paths()); n++) {
            root = archive_path.get_path(n);
            //printf("root: %s\n", root);
            sprintf( file_name, "%s%s", root, path );
            //printf("parser:fopen(\"%s\")\n", file_name);
            fp = ::fopen( file_name, mode );
            if (fp != NULL) break;
        }
    }

    delete[] file_name;
    return fp;
}

void ScriptParser::createKeyTable( const char *key_exe )
{
    if (!key_exe) return;
    
    FILE *fp = ::fopen(key_exe, "rb");
    if (fp == NULL){
        snprintf(script_h.errbuf, MAX_ERRBUF_LEN,
                 "createKeyTable: can't open EXE file '%s'", key_exe);
        errorAndCont(script_h.errbuf, NULL, "Init Issue", true);
        return;
    }

    key_table = new unsigned char[256];

    int i;
    for (i=0 ; i<256 ; i++) key_table[i] = i;

    unsigned char ring_buffer[256];
    int ring_start = 0, ring_last = 0;
    
    int ch, count;
    while((ch = fgetc(fp)) != EOF){
        i = ring_start;
        count = 0;
        while (i != ring_last &&
               ring_buffer[i] != ch ){
            count++;
            i = (i+1)%256;
        }
        if (i == ring_last && count == 255) break;
        if (i != ring_last)
            ring_start = (i+1)%256;
        ring_buffer[ring_last] = ch;
        ring_last = (ring_last+1)%256;
    }
    fclose(fp);

    if (ch == EOF)
        errorAndExit( "createKeyTable: can't find a key table.", NULL, "Init Issue", true );

    // Key table creation
    ring_buffer[ring_last] = ch;
    for (i=0 ; i<256 ; i++)
        key_table[ring_buffer[(ring_start+i)%256]] = i;
}

//Mion: for setting the default Save/Load Menu labels
//(depending on the current system menu language)
void ScriptParser::setDefaultMenuLabels()
{
    if (script_h.system_menu_script != ScriptHandler::JAPANESE_SCRIPT) {
        setStr( &save_menu_name, "[ Save ]" );
        setStr( &load_menu_name, "[ Load ]" );
        setStr( &save_item_name, "Slot " );
    }
    else {
        setStr( &save_menu_name, (const char*)shiftjis_data::menu_labels::save_menu_name );
        setStr( &load_menu_name, (const char*)shiftjis_data::menu_labels::load_menu_name );
        setStr( &save_item_name, (const char*)shiftjis_data::menu_labels::save_item_name );
    }
}

//Mion: for kinsoku
void ScriptParser::setKinsoku(const char *start_chrs, const char *end_chrs, bool add, int code)
{
    int num_start, num_end, i;
    const char *kchr;
    Kinsoku *tmp;

    // count chrs
    num_start = 0;
    kchr = start_chrs;
    while (*kchr != '\0') {
        kchr += script_h.enc.getBytes(*kchr, code);
        num_start++;
    }

    num_end = 0;
    kchr = end_chrs;
    while (*kchr != '\0') {
        kchr += script_h.enc.getBytes(*kchr, code);
        num_end++;
    }

    if (add) {
        if (start_kinsoku != NULL)
            tmp = start_kinsoku;
        else {
            tmp = new Kinsoku[1];
            num_start_kinsoku = 0;
        }
    } else {
        if (start_kinsoku != NULL)
            delete[] start_kinsoku;
        tmp = new Kinsoku[1];
        num_start_kinsoku = 0;
    }
    start_kinsoku = new Kinsoku[num_start_kinsoku + num_start];
    kchr = start_chrs;
    /*
    for (i=0; i<num_start_kinsoku+num_start; i++) {
        if (i < num_start_kinsoku)
            start_kinsoku[i].chr[0] = tmp[i].chr[0];
        else
            start_kinsoku[i].chr[0] = *kchr++;
        if IS_TWO_BYTE(start_kinsoku[i].chr[0]) {
            if (i < num_start_kinsoku)
                start_kinsoku[i].chr[1] = tmp[i].chr[1];
            else
                start_kinsoku[i].chr[1] = *kchr++;
        } else {
            start_kinsoku[i].chr[1] = '\0';
        }
    }
    */
    // TODO: test this -Galladite 2023-6-16
    for (int i=0; i<num_start; i++){
        start_kinsoku[num_start_kinsoku + i].unicode = script_h.enc.getUTF16(kchr, code);
        kchr += script_h.enc.getBytes(*kchr, code);
    }
    num_start_kinsoku += num_start;
    delete[] tmp;

    if (add) {
        if (end_kinsoku != NULL)
            tmp = end_kinsoku;
        else {
            tmp = new Kinsoku[1];
            num_end_kinsoku = 0;
        }
    } else {
        if (end_kinsoku != NULL)
            delete[] end_kinsoku;
        tmp = new Kinsoku[1];
        num_end_kinsoku = 0;
    }
    end_kinsoku = new Kinsoku[num_end_kinsoku + num_end];
    kchr = end_chrs;

    // From onani:
    /*
    for (int i=0; i<num_end; i++) {
        end_kinsoku[num_end_kinsoku + i].unicode = script_h.enc.getUTF16(kchr, code);
        kchr += script_h.enc.getBytes(*kchr, code);
    }
    */

    // TODO: test this -Galladite 2023-6-16
    for (i=0; i<num_end_kinsoku+num_end; i++) {
        if (i < num_end_kinsoku)
            end_kinsoku[i].unicode = tmp[i].unicode;
        else
            end_kinsoku[i].unicode = script_h.enc.getUTF16(kchr, code);
        kchr += script_h.enc.getBytes(*kchr, code);
    }

    // Old ver.
    /*
    for (i=0; i<num_end_kinsoku+num_end; i++) {
        if (i < num_end_kinsoku)
            end_kinsoku[i].chr[0] = tmp[i].chr[0];
        else
            end_kinsoku[i].chr[0] = *kchr++;
        if IS_TWO_BYTE(end_kinsoku[i].chr[0]) {
            if (i < num_end_kinsoku)
                end_kinsoku[i].chr[1] = tmp[i].chr[1];
            else
                end_kinsoku[i].chr[1] = *kchr++;
        } else {
            end_kinsoku[i].chr[1] = '\0';
        }
    }
    */

    num_end_kinsoku += num_end;
    delete[] tmp;
}

bool ScriptParser::isStartKinsoku(const char *str)
{
    /*
    for (int i=0; i<num_start_kinsoku; i++) {
        if ((start_kinsoku[i].chr[0] == *str) &&
            (start_kinsoku[i].chr[1] == *(str+1)))
            return true;
    }
    return false;
    */

    unsigned short unicode = script_h.enc.getUTF16(str);
    for (int i=0; i<num_start_kinsoku; i++)
        if (unicode == start_kinsoku[i].unicode) return true;
    return false;
}

bool ScriptParser::isEndKinsoku(const char *str)
{
    /*
    for (int i=0; i<num_end_kinsoku; i++) {
        if ((end_kinsoku[i].chr[0] == *str) &&
            (end_kinsoku[i].chr[1] == *(str+1)))
            return true;
    }
    return false;
    */

    unsigned short unicode = script_h.enc.getUTF16(str);
    for (int i=0; i<num_end_kinsoku; i++)
        if (unicode == end_kinsoku[i].unicode) return true;
    return false;
}
