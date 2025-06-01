/* -*- C++ -*-
 * 
 *  ScriptParser.h - Define block parser of ONScripter-EN
 *
 *  Copyright (c) 2001-2009 Ogapee. All rights reserved.
 *  (original ONScripter, of which this is a fork).
 *
 *  ogapee@aqua.dti2.ne.jp
 *
 *  Copyright (c) 2007-2010 "Uncle" Mion Sonozaki
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

// Modified by Haeleth, Autumn 2006, to better support OS X/Linux packaging;
// and since then numerous other times (see SVN changelog for full details)

// Modified by Mion of Sonozaki Futago-tachi, March 2008, to update from
// Ogapee's 20080121 release source code.

#ifndef __SCRIPT_PARSER_H__
#define __SCRIPT_PARSER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <SDL_mixer.h>
#include "DirPaths.h"
#include "ScriptHandler.h"
#include "NsaReader.h"
#include "DirectReader.h"
#include "AnimationInfo.h"
#include "FontInfo.h"
#include "Layer.h"
#ifdef USE_LUA
#include "LUAHandler.h"
#endif

#if defined(USE_OGG_VORBIS)
#if defined(INTEGER_OGG_VORBIS)
#include <tremor/ivorbisfile.h>
#else
#include <vorbis/vorbisfile.h>
#endif
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define DEFAULT_FONT_SIZE 26

#define DEFAULT_LOOKBACK_NAME0 "uoncur.bmp"
#define DEFAULT_LOOKBACK_NAME1 "uoffcur.bmp"
#define DEFAULT_LOOKBACK_NAME2 "doncur.bmp"
#define DEFAULT_LOOKBACK_NAME3 "doffcur.bmp"

// Mion: kinsoku
#define DEFAULT_START_KINSOKU "」』）］｝、。，．・？！ヽヾゝゞ々ー"
#define DEFAULT_END_KINSOKU   "「『（［｛"

typedef unsigned char uchar3[3];

struct OVInfo{
    SDL_AudioCVT cvt;
    int cvt_len;
    int mult1;
    int mult2;
    unsigned char *buf;
    long decoded_length;
#if defined(USE_OGG_VORBIS)
    ogg_int64_t length;
    ogg_int64_t pos;
    OggVorbis_File ovf;
#endif
};

typedef struct {
    FILE *fp; // May be unneeded
    enum { NONE,
           R, RC,
           W, WC } mode;
    unsigned char *contents;
    unsigned char *contents_ptr;
} csvinfo;

class ScriptParser
{
public:
    struct MusicStruct{
        OVInfo *ovi;
        int volume;
        bool is_mute;
        Mix_Chunk **voice_sample; //Mion: for bgmdownmode
        MusicStruct()
        : ovi(NULL), volume(0), is_mute(false), voice_sample(NULL) {}
    };
    csvinfo CSVInfo;

    ScriptParser();
    virtual ~ScriptParser();

    void reset();
    void resetDefineFlags(); // for resetting (non-pointer) variables
    int open();
    int open_screen(); // Need access to save path before setting variable screen res -Galladite 2023-10-20
    int parseLine();
    void setCurrentLabel( const char *label );
    void gosubReal( const char *label, char *next_script, bool textgosub_flag=false, int rgosub_state=CLICK_NONE, bool rgosub_1byte=false );

    FILE *fopen(const char *path, const char *mode, const bool save = false, const bool usesavedir = false);
    void saveGlovalData(bool no_error=false);
    void setArchivePath(const char *path);
    void setSavePath(const char *path);
    void setNsaOffset(const char *off);

#ifdef MACOSX
    void checkBundled();
    bool isBundled() {return is_bundled; }
    char *bundleResPath() { return bundle_res_path; }
    char *bundleAppPath() { return bundle_app_path; }
    char *bundleAppName() { return bundle_app_name; }
#endif

    /* Command */
    int zenkakkoCommand();
    int windowchipCommand();
    int windowbackCommand();
    int versionstrCommand();
    int usewheelCommand();
    int useescspcCommand();
    int underlineCommand();
    int transmodeCommand();
    int timeCommand();
    int textgosubCommand();
    int tanCommand();
    int subCommand();
    int straliasCommand();
    int soundpressplginCommand();
    int skipCommand();
    int sinCommand();
    int shadedistanceCommand();
    int setlayerCommand();
    int setkinsokuCommand();
    int selectvoiceCommand();
    int selectcolorCommand();
    int savenumberCommand();
    int savenameCommand();
    int savedirCommand();
    int rubyonCommand();
    int rubyoffCommand();
    int roffCommand();
    int rmenuCommand();
    int rgosubCommand();
    int returnCommand();
    int pretextgosubCommand();
    int pagetagCommand();
    int numaliasCommand();
    int nsadirCommand();
    int nsaCommand();
    int nextCommand();
    int mulCommand();
    int movCommand();
    int mode_wave_demoCommand();
    int mode_sayaCommand();
    int mode_extCommand();
    int modCommand();
    int midCommand();
    int menusetwindowCommand();
    int menuselectvoiceCommand();
    int menuselectcolorCommand();
    int maxkaisoupageCommand();
    int luasubCommand();
    int luacallCommand();
    int lookbackspCommand();
    int lookbackcolorCommand();
    //int lookbackbuttonCommand();
    int logCommand();
    int loadgosubCommand();
    int linepageCommand();
    int lenCommand();
    int labellogCommand();
    int labelexistCommand();
    int kidokuskipCommand();
    int kidokumodeCommand();
    int itoaCommand();
    int intlimitCommand();
    int incCommand();
    int ifCommand();
    int humanzCommand();
    int humanposCommand();
    int gotoCommand();
    int gosubCommand();
    int globalonCommand();
    int getparamCommand();
    //int gameCommand();
    int forCommand();
    int filelogCommand();
    int errorsaveCommand();
    int englishCommand();
    int effectcutCommand();
    int effectblankCommand();
    int effectCommand();
    int dsoundCommand();
    int divCommand();
    int dimCommand();
    int defvoicevolCommand();
    int defsubCommand();
    int defsevolCommand();
    int defmp3volCommand();
    int defaultspeedCommand();
    int defaultfontCommand();
    int decCommand();
    int dateCommand();
    int csvwriteCommand();
    int csvreadCommand();
    int csvopenCommand();
    int csveofCommand();
    int csvcloseCommand();
    int cosCommand();
    int cmpCommand();
    int clickvoiceCommand();
    int clickstrCommand();
    int clickskippageCommand();
    int btnnowindoweraseCommand();
    int breakCommand();
    int autosaveoffCommand();
    int atoiCommand();
    int arcCommand();
    int addnsadirCommand();
    int addkinsokuCommand();
    int addCommand();
    
    void add_debug_level();
    void errorAndExit( const char *str, const char *reason=NULL, const char *title=NULL, bool is_simple=false );
    void errorAndCont( const char *str, const char *reason=NULL, const char *title=NULL, bool is_simple=false );

    //Mion: syntax flags
    bool allow_color_type_only;    // only allow color type (#RRGGBB) for args of color type,
                                   // i.e. not string variables
    bool set_tag_page_origin_to_1; // 'gettaglog' will consider the current page as 1, not 0
    bool answer_dialog_with_yes_ok;// give 'yesnobox' and 'okcancelbox' 'yes/ok' results
    inline const char *readColorStr() {
        if (allow_color_type_only)
            return script_h.readColor();
        else
            return script_h.readStr();
    }
protected:
    struct UserFuncLUT{
        struct UserFuncLUT *next;
        char *command;
        bool lua_flag;
        UserFuncLUT(): next(NULL), command(NULL), lua_flag(false){}
        ~UserFuncLUT(){
            if (command) delete[] command;
        };
    };

    struct UserFuncHash{
        UserFuncLUT root;
        UserFuncLUT *last;
    } user_func_hash['z'-'a'+1];

    struct NestInfo{
        enum { LABEL = 0,
               FOR   = 1 };
        struct NestInfo *previous, *next;
        int  nest_mode;
        char *next_script; // points into script_buffer; used in gosub and for
        int  var_no, to, step; // used in for
        bool textgosub_flag; // used in textgosub and pretextgosub
        int  rgosub_click_state; // used for rgosub
        bool rgosub_1byte_mode; // used for rgosub
        bool rgosub_jumpback; // used for rgosub / select

        NestInfo()
        : previous(NULL), next(NULL), nest_mode(LABEL),
          next_script(NULL), var_no(0), to(0), step(0),
          textgosub_flag(false),
          rgosub_click_state(CLICK_NONE), rgosub_1byte_mode(false),
          rgosub_jumpback(false) {}
        //pointers previous, next, & next_script do not need to be freed
    } last_tilde;

    enum { SYSTEM_NULL        = 0,
           SYSTEM_SKIP        = 1,
           SYSTEM_RESET       = 2,
           SYSTEM_SAVE        = 3,
           SYSTEM_LOAD        = 4,
           SYSTEM_LOOKBACK    = 5,
           SYSTEM_WINDOWERASE = 6,
           SYSTEM_MENU        = 7,
           SYSTEM_YESNO       = 8,
           SYSTEM_AUTOMODE    = 9,
           SYSTEM_END         = 10
    };
    enum { RET_NOMATCH   = 0,
           RET_SKIP_LINE = 1,
           RET_CONTINUE  = 2,
           RET_NO_READ   = 4,
           RET_EOL       = 8, // end of line (0x0a is found)
           RET_EOT       = 16, // end of text (the end of string_buffer is reached)
           RET_RESTART   = 32 // to facilitate full engine resets (mainly setres)
    };
    enum { CLICK_NONE    = 0,
           CLICK_WAIT    = 1,
           CLICK_NEWPAGE = 2,
           CLICK_WAITEOL = 4
    };
    enum{ NORMAL_MODE, DEFINE_MODE };
    int current_mode;
    int debug_level;

#ifdef MACOSX
    bool is_bundled;
    char *bundle_res_path;
    char *bundle_app_path;
    char *bundle_app_name;
#endif
    char *cmdline_game_id;
    DirPaths archive_path;
    DirPaths nsa_path;
    int nsa_offset;
    bool globalon_flag;
    bool labellog_flag;
    bool filelog_flag;
    bool kidokuskip_flag;
    bool kidokumode_flag;
    bool autosaveoff_flag;

    bool clickskippage_flag;

    int z_order;
    bool rmode_flag;
    bool windowback_flag;
    bool btnnowindowerase_flag;
    bool usewheel_flag;
    bool useescspc_flag;
    bool mode_wave_demo_flag;
    bool mode_saya_flag;
    bool mode_ext_flag; //enables automode capability
    bool force_button_shortcut_flag;
    bool rubyon_flag;
    bool rubyon2_flag;
    bool pagetag_flag;
    int  windowchip_sprite_no;
    
    int string_buffer_offset;

    NestInfo root_nest_info, *last_nest_info;
    ScriptHandler::LabelInfo current_label_info;
    int current_line;

#ifdef USE_LUA
    LUAHandler lua_handler;
#endif

    /* ---------------------------------------- */
    /* Global definitions */
#ifndef NO_LAYER_EFFECTS
    bool use_layers;
#endif
#ifdef RCA_SCALE
    float scr_stretch_x, scr_stretch_y;
#endif
    int preferred_width;
    int script_width, script_height;
    int screen_ratio1, screen_ratio2;
    int screen_width, screen_height;
    int screen_texture_width, screen_texture_height;
    int screen_bpp;
    char *version_str;
    int underline_value;
    int humanpos[3]; // l,c,r
    char *savedir;

    void deleteNestInfo();
    void setStr( char **dst, const char *src, int num = -1, bool to_utf8 = false );

    /* ---------------------------------------- */
    /* Effect related variables */
    struct EffectLink{
        struct EffectLink *next;
        int no;
        int effect;
        int duration;
        AnimationInfo anim;

        EffectLink(){
            next = NULL;
            effect = 10;
            duration = 0;
        };
    };
    
    EffectLink root_effect_link, *last_effect_link, window_effect, tmp_effect;
    
    int effect_blank;
    bool effect_cut_flag;

    int readEffect( EffectLink *effect );
    EffectLink *parseEffect(bool init_flag);

    /* ---------------------------------------- */
#ifndef NO_LAYER_EFFECTS
    /* Layer related variables */ //Mion
    struct LayerInfo{
        struct LayerInfo *next;
        Layer *handler;
        int num;
        Uint32 interval;
        Uint32 last_update;
        LayerInfo(){
            num = -1;
            interval = last_update = 0;
            handler = NULL;
            next = NULL;
        }
        ~LayerInfo(){
            if (handler) {
                delete handler;
                handler = NULL;
            }
        }
    } *layer_info;
    void deleteLayerInfo();
#endif
    /* ---------------------------------------- */
    /* Lookback related variables */
    //char *lookback_image_name[4];
    int lookback_sp[2];
    uchar3 lookback_color;
    
    /* ---------------------------------------- */
    /* For loop related variables */
    bool break_flag;
    
    /* ---------------------------------------- */
    /* Transmode related variables */
    int trans_mode;
    
    /* ---------------------------------------- */
    /* Save/Load related variables */
    struct SaveFileInfo{
        bool valid;
        int  month, day, hour, minute;
        char sjis_no[5];
        char sjis_month[5];
        char sjis_day[5];
        char sjis_hour[5];
        char sjis_minute[5];
    };
    unsigned int num_save_file;
    char *save_menu_name;
    char *load_menu_name;
    char *save_item_name;
    void setDefaultMenuLabels();

    unsigned char *save_data_buf;
    unsigned char *file_io_buf;
    size_t file_io_buf_ptr;
    size_t file_io_buf_len;
    size_t save_data_len;
    
    bool errorsave;
    
    /* ---------------------------------------- */
    /* Text related variables */
    char *default_env_font;
    int default_text_speed[3];
    struct Page{
        struct Page *next, *previous;

        char *text;
        int max_text;
        int text_count;
        char *tag;

        Page(): next(NULL), previous(NULL),
                text(NULL), max_text(0), text_count(0), tag(NULL){}
        ~Page(){
            if (text) delete[] text;
            text = NULL;
            if (tag)  delete[] tag;
            tag = NULL;
            next = previous = NULL;
        }
        int add(unsigned char ch){
            if (text_count >= max_text) return -1;
            text[text_count++] = ch;
            return 0;
        };
    } *page_list, *start_page, *current_page; // ring buffer
    int  max_page_list;
    int  clickstr_line;
    int  clickstr_state;
    int  linepage_mode;
    int  num_chars_in_sentence;
    bool english_mode;

    struct Kinsoku {
        unsigned int unicode;
    } *start_kinsoku, *end_kinsoku; //Mion: for kinsoku chars
    int num_start_kinsoku, num_end_kinsoku;
    void setKinsoku(const char *start_chrs, const char *end_chrs, bool add, int code = -1); //Mion
    bool isStartKinsoku(const char *str);
    bool isEndKinsoku(const char *str);
    
    /* ---------------------------------------- */
    /* Sound related variables */
    MusicStruct music_struct;
    int music_volume;
    int voice_volume;
    int se_volume;
    bool use_default_volume;

    enum { CLICKVOICE_NORMAL  = 0,
           CLICKVOICE_NEWPAGE = 1,
           CLICKVOICE_NUM     = 2
    };
    char *clickvoice_file_name[CLICKVOICE_NUM];

    enum { SELECTVOICE_OPEN   = 0,
           SELECTVOICE_OVER   = 1,
           SELECTVOICE_SELECT = 2,
           SELECTVOICE_NUM    = 3
    };
    char *selectvoice_file_name[SELECTVOICE_NUM];

    enum { MENUSELECTVOICE_OPEN   = 0,
           MENUSELECTVOICE_CANCEL = 1,
           MENUSELECTVOICE_OVER   = 2,
           MENUSELECTVOICE_CLICK  = 3,
           MENUSELECTVOICE_WARN   = 4,
           MENUSELECTVOICE_YES    = 5,
           MENUSELECTVOICE_NO     = 6,
           MENUSELECTVOICE_NUM    = 7
    };
    char *menuselectvoice_file_name[MENUSELECTVOICE_NUM];
     
    /* ---------------------------------------- */
    /* Font related variables */
    Fontinfo *current_font, sentence_font, menu_font, ruby_font;
    struct RubyStruct{
        enum { NONE,
               BODY,
               RUBY };
        int stage;
        int body_count;
        char *ruby_start;
        char *ruby_end;
        int ruby_count;
        int margin;

        int font_size_xy[2];
        char *font_name;

        RubyStruct(){
            stage = NONE;
            font_size_xy[0] = 0;
            font_size_xy[1] = 0;
            font_name = NULL;
        };
        ~RubyStruct(){
            if ( font_name ) delete[] font_name;
        };
    } ruby_struct;
    int shade_distance[2];

    /* ---------------------------------------- */
    /* RMenu related variables */
    struct RMenuLink{
        RMenuLink *next;
        char *label;
        int system_call_no;

        RMenuLink(){
            next  = NULL;
            label = NULL;
        };
        ~RMenuLink(){
            if (label) delete[] label;
        };
    } root_rmenu_link;
    unsigned int rmenu_link_num, rmenu_link_width;

    void deleteRMenuLink();
    int getSystemCallNo( const char *buffer );
    unsigned char convHexToDec( char ch );

    void setColor( uchar3 &dstcolor, uchar3 srccolor );
    void readColor( uchar3 *color, const char *buf );
    
    void allocFileIOBuf();
    int saveFileIOBuf( const char *filename, int offset=0, const char *savestr=NULL );
    int loadFileIOBuf( const char *filename );

    void writeChar( char c, bool output_flag );
    char readChar();
    void writeInt( int i, bool output_flag );
    int readInt();
    void writeStr( char *s, bool output_flag );
    void readStr( char **s );
    void writeVariables( int from, int to, bool output_flag );
    void readVariables( int from, int to );
    void writeArrayVariable( bool output_flag );
    void readArrayVariable();
    void writeLog( ScriptHandler::LogInfo &info );
    void readLog( ScriptHandler::LogInfo &info );

    /* ---------------------------------------- */
    /* System customize related variables */
    char *textgosub_label;
    char *pretextgosub_label;
    char *loadgosub_label;
    char *rgosub_label;

    ScriptHandler script_h;
    
    unsigned char *key_table;

    void createKeyTable( const char *key_exe );
};

#endif // __SCRIPT_PARSER_H__
