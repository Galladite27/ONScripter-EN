/* -*- C++ -*-
 *
 *  ONScripterLabel.h - Execution block parser of ONScripter-EN
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

// Modified by Haeleth, autumn 2006, to remove unnecessary diagnostics,
// and on many occasions thereafter; see SVN logs for all changes

// Modified by Mion, March 2008, to update from
// Ogapee's 20080121 release source code.

// Modified by Mion, April 2009, to update from
// Ogapee's 20090331 release source code.

#ifndef __ONSCRIPTER_LABEL_H__
#define __ONSCRIPTER_LABEL_H__

#include "DirPaths.h"
#include "ScriptParser.h"
#include "DirtyRect.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

#ifdef MP3_MAD
#include "MadWrapper.h"
#else
#include <smpeg.h>
#endif

#define DEFAULT_VIDEO_SURFACE_FLAG (SDL_SWSURFACE)

#define DEFAULT_BLIT_FLAG (0)
//#define DEFAULT_BLIT_FLAG (SDL_RLEACCEL)

#define MAX_SPRITE_NUM 1000
#define MAX_SPRITE2_NUM 256
#define MAX_PARAM_NUM 100
#define MAX_EFFECT_NUM 256

#define DEFAULT_VOLUME 100
#define ONS_MIX_CHANNELS 50
#define ONS_MIX_EXTRA_CHANNELS 5
#define MIX_WAVE_CHANNEL (ONS_MIX_CHANNELS+0)
#define MIX_CLICKVOICE_CHANNEL (ONS_MIX_CHANNELS+1)
#define MIX_BGM_CHANNEL (ONS_MIX_CHANNELS+2)
#define MIX_LOOPBGM_CHANNEL0 (ONS_MIX_CHANNELS+3)
#define MIX_LOOPBGM_CHANNEL1 (ONS_MIX_CHANNELS+4)

#define FONT_DEFAULT_TTF 0
#define FONT_DEFAULT_TTC 1
#define FONT_DEFAULT_OTF 2
#define FONT_DEFAULT_OTC 3
#define FONT_ARCHIVE_TTF 4
#define FONT_ARCHIVE_TTC 5
#define FONT_ARCHIVE_OTF 6
#define FONT_ARCHIVE_OTC 7
#define FONT_WIN32_MSGOTHIC_TTC 8
#define FONT_WIN32_MSGOTHIC_TTF 9
#define FONT_MACOS_HIRAGINO 10

#if defined(PDA) && !defined(PSP)
#define DEFAULT_AUDIO_RATE 22050
#else
#define DEFAULT_AUDIO_RATE 44100
#endif

#define DEFAULT_WM_TITLE "ONScripter-EN"
#define DEFAULT_WM_ICON  "Ons-en"

#define NUM_GLYPH_CACHE 30

#define KEYPRESS_NULL ((SDLKey)(SDLK_LAST+1)) // "null" for keypress variables

void clearTimer(SDL_TimerID &timer_id);

class ONScripterLabel : public ScriptParser
{
public:
    typedef AnimationInfo::ONSBuf ONSBuf;

    ONScripterLabel();
    ~ONScripterLabel();

    void executeLabel();
    void runScript();

    // ----------------------------------------
    // start-up options
    void enableCDAudio();
    void setCDNumber(int cdrom_drive_number);
    void setAudiodriver(const char *driver);
    void setAudioBufferSize(int kbyte_size);
    void setMatchBgmAudio(bool flag);
    void setFontFile(const char *filename);
    void setRegistryFile(const char *filename);
    void setDLLFile(const char *filename);
    void setFileVersion(const char *ver);
    void setFullscreenMode();
    void setWindowMode();
#ifndef NO_LAYER_EFFECTS
    void setNoLayers();
#endif
#ifdef WIN32
    void setUserAppData();
#endif
    void setUseAppIcons();
    void setIgnoreTextgosubNewline();
    void setSkipPastNewline();
    void setPreferredWidth(const char *widthstr);
    void enableButtonShortCut();
    void setPreferredAutomodeTime(const char *timestr);
    void enableWheelDownAdvance();
    void disableCpuGfx();
    void disableRescale();
    void enableEdit();
    void setKeyEXE(const char *path);
#ifdef RCA_SCALE
    void setWidescreen();
#endif
    void setScaled();
    void setNoMovieUpscale();
    inline void setStrict() { script_h.strict_warnings = true; }
    void setGameIdentifier(const char *gameid);
    enum {
        PNG_MASK_AUTODETECT    = 0,
        PNG_MASK_USE_ALPHA     = 1,
        PNG_MASK_USE_NSCRIPTER = 2
    };
    inline void setMaskType( int mask_type ) { png_mask_type = mask_type; }
    inline void setEnglishDefault()
        { script_h.default_script = ScriptHandler::LATIN_SCRIPT; }
    inline void setJapaneseDefault()
        { script_h.default_script = ScriptHandler::JAPANESE_SCRIPT; }
    inline void setEnglishPreferred()
        { script_h.preferred_script = ScriptHandler::LATIN_SCRIPT; }
    inline void setJapanesePreferred()
        { script_h.preferred_script = ScriptHandler::JAPANESE_SCRIPT; }
    inline void setEnglishMenu()
        { script_h.system_menu_script = ScriptHandler::LATIN_SCRIPT; }
    inline void setJapaneseMenu()
        { script_h.system_menu_script = ScriptHandler::JAPANESE_SCRIPT; }

    int  init();
    void runEventLoop();

    void reset(); // used if definereset
    void resetSub(); // used if reset
    void resetFlags(); // for resetting (non-pointer) definereset variables
    void resetFlagsSub(); // for resetting (non-pointer) reset variables

    //Mion: routines for error handling & cleanup
    bool doErrorBox( const char *title, const char *errstr, bool is_simple=false, bool is_warning=false );
#ifdef WIN32
    void openDebugFolders();
#endif
    /* ---------------------------------------- */
    /* Commands */
    int yesnoboxCommand();
    int wavestopCommand();
    int waveCommand();
    int waittimerCommand();
    int waitCommand();
    int vspCommand();
    int voicevolCommand();
    int vCommand();
    int trapCommand();
    int transbtnCommand();
    int textspeeddefaultCommand();
    int textspeedCommand();
    int textshowCommand();
    int textonCommand();
    int textoffCommand();
    int texthideCommand();
    int textexbtnCommand();
    int textclearCommand();
    int textbtnstartCommand();
    int textbtnoffCommand();
    int texecCommand();
    int tateyokoCommand();
    int talCommand();
    int tablegotoCommand();
    int systemcallCommand();
    int strspCommand();
    int strpxlenCommand();
    int stopCommand();
    int sp_rgb_gradationCommand();
    int spstrCommand();
    int spreloadCommand();
    int splitonceCommand();
    int splitCommand();
    int spclclkCommand();
    int spbtnCommand();
    int skipoffCommand();
    int shellCommand();
    int sevolCommand();
    int setwindow4Command();
    int setwindow3Command();
    int setwindow2Command();
    int setwindowCommand();
    int setresCommand();
    int seteffectspeedCommand();
    int setcursorCommand();
    int selectCommand();
    int savetimeCommand();
    int savepointCommand();
    int saveonCommand();
    int saveoffCommand();
    int savegameCommand();
    int savefileexistCommand();
    int savescreenshotCommand();
    int rndCommand();
    int rmodeCommand();
    int resettimerCommand();
    int resetCommand();
    int repaintCommand();
    int quakeCommand();
    int puttextCommand();
    int prnumclearCommand();
    int prnumCommand();
    int printCommand();
    int playstopCommand();
    int playonceCommand();
    int playCommand();
    int ofscopyCommand();
    int negaCommand();
    int mvCommand();
    int mspCommand();
    int mp3volCommand();
    int mp3stopCommand();
    int mp3fadeoutCommand();
    int mp3fadeinCommand();
    int mp3Command();
    int movieCommand();
    int movemousecursorCommand();
    int mousemodeCommand();
    int monocroCommand();
    int minimizewindowCommand();
    int mesboxCommand();
    int menu_windowCommand();
    int menu_waveonCommand();
    int menu_waveoffCommand();
    int menu_fullCommand();
    int menu_click_pageCommand();
    int menu_click_defCommand();
    int menu_automodeCommand();
    int lsp2Command();
    int lspCommand();
    int loopbgmstopCommand();
    int loopbgmCommand();
    int lookbackflushCommand();
    int lookbackbuttonCommand();
    int logspCommand();
    int locateCommand();
    int loadgameCommand();
    int linkcolorCommand();
    int ldCommand();
    int languageCommand();
    int jumpfCommand();
    int jumpbCommand();
    int ispageCommand();
    int isfullCommand();
    int isskipCommand();
    int isdownCommand();
    int inputCommand();
    int indentCommand();
    int humanorderCommand();
    int getzxcCommand();
    int getvoicevolCommand();
    int getversionCommand();
    int gettimerCommand();
    int gettextbtnstrCommand();
    int gettextCommand();
    int gettaglogCommand();
    int gettagCommand();
    int gettabCommand();
    int getspsizeCommand();
    int getspmodeCommand();
    int getskipoffCommand();
    int getsevolCommand();
    int getscreenshotCommand();
    int getsavestrCommand();
    int getretCommand();
    int getresCommand();
    int getregCommand();
    int getpageupCommand();
    int getpageCommand();
    int getmp3volCommand();
    int getmouseposCommand();
    int getmouseoverCommand();
    int getmclickCommand();
    int getlogCommand();
    int getinsertCommand();
    int getfunctionCommand();
    int getenterCommand();
    int getcursorposCommand();
    int getcursorCommand();
    int getcselstrCommand();
    int getcselnumCommand();
    int gameCommand();
    int flushoutCommand();
    int fileexistCommand();
    int exec_dllCommand();
    int exbtnCommand();
    int erasetextwindowCommand();
    int erasetextbtnCommand();
    int engineresetCommand();
    int endCommand();
    int effectskipCommand();
    int dwavestopCommand();
    int dwaveCommand();
    int dvCommand();
    int drawtextCommand();
    int drawsp3Command();
    int drawsp2Command();
    int drawspCommand();
    int drawfillCommand();
    int drawclearCommand();
    int drawbg2Command();
    int drawbgCommand();
    int drawCommand();
    int deletescreenshotCommand();
    int delayCommand();
    int defineresetCommand();
    int cspCommand();
    int cselgotoCommand();
    int cselbtnCommand();
    int clickCommand();
    int clCommand();
    int chvolCommand();
    int checkpageCommand();
    int checkkeyCommand();
    int cellCommand();
    int captionCommand();
    int btnwait2Command();
    int btnwaitCommand();
    int btntime2Command();
    int btntimeCommand();
    int btndownCommand();
    int btndefCommand();
    int btnareaCommand();
    int btnCommand();
    int brCommand();
    int bltCommand();
    int bgmdownmodeCommand();
    int bgcopyCommand();
    int bgCommand();
    int barclearCommand();
    int barCommand();
    int aviCommand();
    int autosaveoffCommand();
    int automode_timeCommand();
    int autoclickCommand();
    int allsp2resumeCommand();
    int allspresumeCommand();
    int allsp2hideCommand();
    int allsphideCommand();
    int amspCommand();

    int insertmenuCommand();
    int resetmenuCommand();
    int layermessageCommand();

protected:
    /* ---------------------------------------- */
    /* Event related variables */
    enum {
        NOT_EDIT_MODE            = 0,
        EDIT_SELECT_MODE         = 1,
        EDIT_VOLUME_MODE         = 2,
        EDIT_VARIABLE_INDEX_MODE = 3,
        EDIT_VARIABLE_NUM_MODE   = 4,
        EDIT_MP3_VOLUME_MODE     = 5,
        EDIT_VOICE_VOLUME_MODE   = 6,
        EDIT_SE_VOLUME_MODE      = 7
    };

    int variable_edit_mode;
    //Mion: These 3 variables are only used in _event;
    //  could they be static variables there instead of
    //  instance variables here?
    int variable_edit_index;
    int variable_edit_num;
    int variable_edit_sign;

    bool key_pressed_flag;
    int  shift_pressed_status;
    int  ctrl_pressed_status;
#ifdef MACOSX
    int apple_pressed_status;
#endif
    bool bgmdownmode_flag;
    // the default behavior when in "click to skip" mode is to stop
    // the skip at the next clickwait or newline, whichever comes first.
    //Since some very old onscripters didn't stop at newlines,
    // setting skip_past_newline will help when playing older onscripter games.
    bool skip_past_newline; // don't leave 'click to skip' mode at a newline in text cmds

    // helper function declarations for fonts
    bool file_exists(const char *fileName);
    char* create_filepath(DirPaths archive_path, const char* filename);

    SDL_keysym transKey(SDL_keysym key, bool isdown);
    void variableEditMode( SDL_KeyboardEvent *event );
    bool keyDownEvent( SDL_KeyboardEvent *event );
    void keyUpEvent( SDL_KeyboardEvent *event );
    bool keyPressEvent( SDL_KeyboardEvent *event );
    bool mousePressEvent( SDL_MouseButtonEvent *event );
    bool mouseMoveEvent( SDL_MouseMotionEvent *event );
    void animEvent();
    void timerEvent();
    void flushEventSub( SDL_Event &event );
    void flushEvent();
    void advancePhase( int count=0 );
    void advanceAnimPhase( int count=0 );
    void waitEventSub(int count);
    bool waitEvent(int count);
    void trapHandler();
    void initSDL();
    void openAudio(int freq=DEFAULT_AUDIO_RATE, Uint16 format=MIX_DEFAULT_FORMAT, int channels=MIX_DEFAULT_CHANNELS);

private:
    enum {
        DISPLAY_MODE_NORMAL  = 0, 
        DISPLAY_MODE_TEXT    = 1,
        DISPLAY_MODE_UPDATED = 2
    };
    enum {
        IDLE_EVENT_MODE      = 0,
        WAIT_RCLICK_MODE     = 1,   // for lrclick
        WAIT_BUTTON_MODE     = 2,   // For select, btnwait and rmenu.
        WAIT_INPUT_MODE      = 4,   // can be skipped by a click
        WAIT_TEXTOUT_MODE    = 8,   // can be skipped by a click
        WAIT_SLEEP_MODE      = 16,  // cannot be skipped by ctrl but not click
        WAIT_TIMER_MODE      = 32,
        WAIT_TEXTBTN_MODE    = 64,
        WAIT_VOICE_MODE      = 128,
        WAIT_TEXT_MODE       = 256, // clickwait, newpage, select
        WAIT_NO_ANIM_MODE    = 512
    };
    enum {
        EFFECT_DST_GIVEN     = 0,
        EFFECT_DST_GENERATE  = 1
    };
    enum {
        ALPHA_BLEND_CONST          = 1,
        ALPHA_BLEND_MULTIPLE       = 2,
        ALPHA_BLEND_FADE_MASK      = 3,
        ALPHA_BLEND_CROSSFADE_MASK = 4
    };

    // ----------------------------------------
    // start-up options
    bool cdaudio_flag;
    int  audiobuffer_size;
    bool match_bgm_audio_flag;
    char *default_font;
    char *registry_file;
    char *dll_file;
    char *getret_str;
    int  getret_int;
    bool enable_wheeldown_advance_flag;
    bool disable_rescale_flag;
    bool edit_flag;
    char *key_exe_file;
#ifdef RCA_SCALE
    bool widescreen_flag;
#endif
    bool scaled_flag;
    bool nomovieupscale_flag;

    //Mion: inlines for image/screen resizing & scaling
    int ExpandPos(int val);
    int ContractPos(int val);
#ifdef RCA_SCALE
    int StretchPosX(int val);
    int StretchPosY(int val);
#else
    inline int StretchPosX(int val) { return ExpandPos(val); }
    inline int StretchPosY(int val) { return ExpandPos(val); }
#endif
    inline void UpdateAnimPosXY(AnimationInfo *animp) {
        animp->pos.x = ExpandPos(animp->orig_pos.x);
        animp->pos.y = ExpandPos(animp->orig_pos.y);
    }
    inline void UpdateAnimPosWH(AnimationInfo *animp) {
        animp->pos.w = ExpandPos(animp->orig_pos.w);
        animp->pos.h = ExpandPos(animp->orig_pos.h);
    }
    inline void UpdateAnimPosStretchXY(AnimationInfo *animp) {
        animp->pos.x = StretchPosX(animp->orig_pos.x);
        animp->pos.y = StretchPosY(animp->orig_pos.y);
    }
    inline void UpdateAnimPosStretchWH(AnimationInfo *animp) {
        animp->pos.w = StretchPosX(animp->orig_pos.w);
        animp->pos.h = StretchPosY(animp->orig_pos.h);
    }

    // ----------------------------------------
    // Global definitions
    long internal_timer;
    bool automode_flag;
    bool preferred_automode_time_set;
    long preferred_automode_time;
    long automode_time;
    long autoclick_time;

    bool saveon_flag;
    bool internal_saveon_flag; // to saveoff at the head of text

    bool monocro_flag;
    uchar3 monocro_color;
    uchar3 monocro_color_lut[256];
    int  nega_mode;

    enum {
        TRAP_NONE        = 0,
        TRAP_LEFT_CLICK  = 1,
        TRAP_RIGHT_CLICK = 2,
        TRAP_NEXT_SELECT = 4,
        TRAP_STOP        = 8
    };
    int  trap_mode;
    char *trap_dest; //label to jump to when trapped
    char *wm_title_string;
    char *wm_icon_string;
    char wm_edit_string[256];
    bool fullscreen_mode;
    bool window_mode; //ons-specific, for cmd-line option --window
    int fileversion;
#ifdef WIN32
    bool current_user_appdata;
#endif
    bool use_app_icons;

    bool btntime2_flag;
    long btntime_value;
    long btnwait_time;
    bool btndown_flag;
    bool transbtn_flag;

    SDLKey last_keypress;

    void quit(bool no_error=false);

    /* ---------------------------------------- */
    /* Script related variables */
    enum {
        REFRESH_NONE_MODE   = 0,
        REFRESH_NORMAL_MODE = 1,
        REFRESH_SAYA_MODE   = 2,
        REFRESH_WINDOW_MODE = 4,  //show textwindow background
        REFRESH_TEXT_MODE   = 8,  //show textwindow text
        REFRESH_CURSOR_MODE = 16  //show textwindow cursor
    };

    int refresh_window_text_mode;
    int display_mode;
    bool did_leavetext;
    int event_mode;
    SDL_Surface *accumulation_surface; // Final image, i.e. picture_surface (+ text_window + text_surface)
    SDL_Surface *backup_surface; // Final image w/o (text_window + text_surface) used in leaveTextDisplayMode()
    SDL_Surface *screen_surface; // Text + Select_image + Tachi image + background
    SDL_Surface *effect_dst_surface; // Intermediate source buffer for effect
    SDL_Surface *effect_src_surface; // Intermediate destination buffer for effect
    SDL_Surface *effect_tmp_surface; // Intermediate buffer for effect
    SDL_Surface *screenshot_surface; // Screenshot
    SDL_Surface *image_surface; // Reference for loadImage() - 32bpp

    unsigned char *tmp_image_buf;
    unsigned long tmp_image_buf_length;
    unsigned long mean_size_of_loaded_images;
    unsigned long num_loaded_images;

    /* ---------------------------------------- */
    /* Button related variables */
    AnimationInfo btndef_info;

    struct ButtonState{
        int x, y, button;
        bool down_flag, valid_flag;
        //Mion - initialize the button
        ButtonState()
        : x(0), y(0), button(0), down_flag(false), valid_flag(false)
        {}
        void reset(){ //Mion - clear the button state
            button = 0;
            valid_flag = false;
        };
        void set(int val){ //Mion - set button & valid_flag
            button = val;
            valid_flag = true;
        };
    } current_button_state, volatile_button_state, last_mouse_state, shelter_mouse_state;

    struct ButtonLink{
        typedef enum {
            NORMAL_BUTTON     = 0,
            SPRITE_BUTTON     = 1,
            EX_SPRITE_BUTTON  = 2,
            LOOKBACK_BUTTON   = 3,
            TMP_SPRITE_BUTTON = 4,
            TEXT_BUTTON       = 5
        } BUTTON_TYPE;

        struct ButtonLink *next;
        struct ButtonLink *same; //Mion: to link buttons that act in concert
        BUTTON_TYPE button_type;
        int no;
        int sprite_no;
        char *exbtn_ctl;
        int show_flag; // 0...show nothing, 1... show anim[0], 2 ... show anim[1]
        SDL_Rect select_rect;
        SDL_Rect image_rect;
        AnimationInfo *anim[2];

        ButtonLink(){
            button_type = NORMAL_BUTTON;
            next = NULL;
            same = NULL;
            exbtn_ctl = NULL;
            anim[0] = anim[1] = NULL;
            show_flag = 0;
        };
        ~ButtonLink(){
            if ((button_type == NORMAL_BUTTON || 
                 button_type == TMP_SPRITE_BUTTON ||
                 button_type == TEXT_BUTTON) && anim[0]) delete anim[0];
            anim[0] = anim[1] = NULL;
            if ( exbtn_ctl ) delete[] exbtn_ctl;
            exbtn_ctl = NULL;
            next = NULL;
            same = NULL;
        };
        void insert( ButtonLink *button ){
            button->next = this->next;
            this->next = button;
        };
        void connect( ButtonLink *button ){
            button->same = this->same;
            this->same = button;
        };
        void removeSprite( int no ){
            ButtonLink *p = this;
            while(p->next){
                if ((p->next->sprite_no == no) &&
                    ( (p->next->button_type == SPRITE_BUTTON) ||
                      (p->next->button_type == EX_SPRITE_BUTTON) )){
                    ButtonLink *p2 = p->next;
                    p->next = p->next->next;
                    delete p2;
                }
                else{
                    p = p->next;
                }
            }
        };
    } root_button_link, *current_button_link, *shelter_button_link,
      exbtn_d_button_link, exbtn_d_shelter_button_link, text_button_link;
    bool is_exbtn_enabled;

    bool current_button_valid;
    int current_over_button;

    /* ---------------------------------------- */
    /* Mion: textbtn related variables */
    struct TextButtonInfoLink{
        struct TextButtonInfoLink *next;
        char *text; //actual "text" of the button
        char *prtext; // button text as printed (w/linebreaks)
        ButtonLink *button;
        int xy[2];
        int no;
        TextButtonInfoLink()
        : next(NULL), text(NULL), prtext(NULL), button(NULL){
            xy[0] = xy[1] = -1;
            no = -1;
        };
        ~TextButtonInfoLink(){
            if (text) delete[] text;
            if (prtext) delete[] prtext;
        };
        void insert( TextButtonInfoLink *info ){
            info->next = this->next;
            this->next = info;
        };
    } text_button_info;
    int txtbtn_start_num;
    int next_txtbtn_num;
    bool in_txtbtn;
    bool txtbtn_show;
    bool txtbtn_visible;
    uchar3 linkcolor[2];

    bool getzxc_flag;
    bool gettab_flag;
    bool getpageup_flag;
    bool getpagedown_flag;
    bool getinsert_flag;
    bool getfunction_flag;
    bool getenter_flag;
    bool getcursor_flag;
    bool spclclk_flag;
    bool getmclick_flag;
    bool getskipoff_flag;
    bool getmouseover_flag;
    int  getmouseover_min, getmouseover_max;
    bool btnarea_flag;
    int  btnarea_pos;

    void resetSentenceFont();
    void deleteButtonLink();
    void processTextButtonInfo();
    void deleteTextButtonInfo();
    void terminateTextButton();
    void textbtnColorChange();
    void refreshMouseOverButton();
    void refreshSprite( int sprite_no, bool active_flag, int cell_no, SDL_Rect *check_src_rect, SDL_Rect *check_dst_rect );

    void decodeExbtnControl( const char *ctl_str, SDL_Rect *check_src_rect=NULL, SDL_Rect *check_dst_rect=NULL );

    void disableGetButtonFlag();
    int getNumberFromBuffer( const char **buf );

    /* ---------------------------------------- */
    /* General image-related variables */
    int png_mask_type;

    /* ---------------------------------------- */
    /* Background related variables */
    AnimationInfo bg_info;

    /* ---------------------------------------- */
    /* Tachi-e related variables */
    /* 0 ... left, 1 ... center, 2 ... right */
    AnimationInfo tachi_info[3];
    int human_order[3];

    /* ---------------------------------------- */
    /* Sprite related variables */
    AnimationInfo *sprite_info;
    AnimationInfo *sprite2_info;
    bool all_sprite_hide_flag;
    bool all_sprite2_hide_flag;

    //Mion: track the last few sprite numbers loaded, for sprite data reuse
    enum {
        SPRITE_NUM_LAST_LOADS = 4
    };
    int last_loaded_sprite[SPRITE_NUM_LAST_LOADS];
    int last_loaded_sprite_ind;

    /* ---------------------------------------- */
    /* Parameter related variables */
    AnimationInfo *bar_info[MAX_PARAM_NUM], *prnum_info[MAX_PARAM_NUM];

    /* ---------------------------------------- */
    /* Cursor related variables */
    enum {
        CURSOR_WAIT_NO    = 0,
        CURSOR_NEWPAGE_NO = 1
    };
    AnimationInfo cursor_info[2];

    void loadCursor( int no, const char *str, int x, int y, bool abs_flag = false );
    void saveAll(bool no_error=false);
    void loadEnvData();
    void saveEnvData();

    /* ---------------------------------------- */
    /* Lookback related variables */
    AnimationInfo lookback_info[4];

    /* ---------------------------------------- */
    /* Text related variables */
    AnimationInfo text_info, shelter_text_info;
    AnimationInfo sentence_font_info;
    char *font_file;
    int erase_text_window_mode;
    bool text_on_flag; // suppress the effect of erase_text_window_mode
    bool draw_cursor_flag;
    int  textgosub_clickstr_state;
    int  indent_offset;
    int  line_enter_status; // 0 - no enter, 1 - pretext, 2 - body start, 3 - within body
    int  page_enter_status; // 0 ... no enter, 1 ... body
    struct GlyphCache{
        GlyphCache *next;
        Uint16 text;
        TTF_Font *font;
        SDL_Surface *surface;
        GlyphCache()
        : next(NULL), text(0), font(NULL), surface(NULL) {}
        ~GlyphCache() { SDL_FreeSurface(surface); }
    } *root_glyph_cache, glyph_cache[NUM_GLYPH_CACHE];
    int last_textpos_xy[2];

    int  refreshMode();
    void setwindowCore(bool utf8_precalc = true);
    // New option specifies if we should calculate early the total px
    // from the full-width px and number of columns

    SDL_Surface *renderGlyph(TTF_Font *font, Uint16 text);
    void drawGlyph( SDL_Surface *dst_surface, Fontinfo *info, SDL_Color &color, char *text, int xy[2], bool shadow_flag, AnimationInfo *cache_info, SDL_Rect *clip, SDL_Rect &dst_rect );
    void drawChar( char* text, Fontinfo *info, bool flush_flag, bool lookback_flag, SDL_Surface *surface, AnimationInfo *cache_info, int abs_offset=0, SDL_Rect *clip=NULL );
    void drawString( const char *str, uchar3 color, Fontinfo *info, bool flush_flag, SDL_Surface *surface, int abs_offset=0, SDL_Rect *rect = NULL, AnimationInfo *cache_info=NULL, bool skip_whitespace_flag=true );
    void restoreTextBuffer(SDL_Surface *surface = NULL);
    void enterTextDisplayMode(bool text_flag = true);
    void leaveTextDisplayMode(bool force_leave_flag = false);
    bool doClickEnd();
    bool clickWait();
    bool clickNewPage();
    void startRuby(char *buf, Fontinfo &info);
    void endRuby(bool flush_flag, bool lookback_flag, SDL_Surface *surface, AnimationInfo *cache_info);
    int  textCommand();
    void processEOT();
    bool processText();

    // UTF-8 related
    int u8strlen(const char *s);
    //float strpxlen(const char *buf, Fontinfo *fi, bool *bold_flag, bool *italics_flag);
    float strpxlen(const char *buf, Fontinfo *fi);
    //float getPixelLength(const char *buf, Fontinfo *fi, bool *bold_flag, bool *italics_flag);
    // May be unnecessary, I haven't been using it so far bc I didn't realise it existed lol
    //void getNextChar(const char *buf, int offset, char *out_chars);

    //Mion: variables & functions for special text processing
    bool *string_buffer_breaks;  // can it break before a particular offset?
    char *string_buffer_margins; // where are the ruby margins, how long (in pixels)
    bool line_has_nonspace;
    enum LineBreakType {
        SPACEBREAK = 1, // Western-style, break before spaces
        KINSOKU    = 2  // Eastern-style, break anywhere except before/after forbidden chars
    } last_line_break_type;
    char doLineBreak(bool isHardBreak=false);
    int isTextCommand(const char *buf);
    void processRuby(unsigned int i, int cmd);
    bool processBreaks(bool cont_line, LineBreakType style);
    int findNextBreak(int offset, int &len);

    /* ---------------------------------------- */
    /* Skip mode */
    enum {
        SKIP_NONE    = 0,
        SKIP_NORMAL  = 1, // skip endlessly/to unread text (press 's' button)
        SKIP_TO_EOP  = 2, // skip to end of page (press 'o' button)
        SKIP_TO_WAIT = 4, // skip to next clickwait
        SKIP_TO_EOL  = 8  // skip to end of line
    };
    int skip_mode;

    /* ---------------------------------------- */
    /* Effect related variables */
    DirtyRect dirty_rect, dirty_rect_tmp; // only this region is updated
    int effect_counter, effect_duration; // counter in each effect
    int effect_timer_resolution;
    int effect_start_time;
    int effect_start_time_old;
    int effect_tmp; //tmp variable for use by effect routines
    bool in_effect_blank;
    bool effectskip_flag;
    bool skip_effect;
    enum {
        EFFECTSPEED_NORMAL  = 0,
        EFFECTSPEED_QUICKER = 1,
        EFFECTSPEED_INSTANT = 2
    };
    int effectspeed;

    enum {
        //some constants for trig tables
        TRIG_TABLE_SIZE = 256,
        TRIG_FACTOR     = 16384
    };
    int *sin_table, *cos_table;
    int *whirl_table;

    void buildSinTable();
    void buildCosTable();
    void buildWhirlTable();
    bool setEffect( EffectLink *effect, bool generate_effect_dst, bool update_backup_surface );
    bool doEffect( EffectLink *effect, bool clear_dirty_region=true );
    void drawEffect( SDL_Rect *dst_rect, SDL_Rect *src_rect, SDL_Surface *surface );
    void generateMosaic( SDL_Surface *src_surface, int level );
    void doFlushout( int level );
    void effectCascade( char *params, int duration );
    void effectTrvswave( char *params, int duration );
    void effectWhirl( char *params, int duration );

    struct BreakupCell {
        int cell_x, cell_y;
        int dir;
        int state;
        int radius;
        BreakupCell()
        : cell_x(0), cell_y(0),
          dir(0), state(0), radius(0)
        {}
    } *breakup_cells;
    bool *breakup_cellforms, *breakup_mask;
    void buildBreakupCellforms();
    void buildBreakupMask();
    void initBreakup( char *params );
    void effectBreakup( char *params, int duration );

    /* ---------------------------------------- */
    /* Select related variables */
    enum {
        SELECT_GOTO_MODE  = 0,
        SELECT_GOSUB_MODE = 1,
        SELECT_NUM_MODE   = 2,
        SELECT_CSEL_MODE  = 3
    };
    struct SelectLink{
        struct SelectLink *next;
        char *text;
        char *label;

        SelectLink()
        : next(NULL), text(NULL), label(NULL)
        {}
        ~SelectLink(){
            if ( text )  delete[] text;
            if ( label ) delete[] label;
        }
    } root_select_link, *shelter_select_link;
    struct NestInfo select_label_info;
    int shortcut_mouse_line;

    void deleteSelectLink();
    AnimationInfo *getSentence( char *buffer, Fontinfo *info, int num_cells, bool flush_flag = true, bool nofile_flag = false, bool skip_whitespace = true );
    struct ButtonLink *getSelectableSentence( char *buffer, Fontinfo *info, bool flush_flag = true, bool nofile_flag = false, bool skip_whitespace = true );

    /* ---------------------------------------- */
    /* Sound related variables */
    enum{
        SOUND_NONE          =  0,
        SOUND_PRELOAD       =  1,
        SOUND_WAVE          =  2,
        SOUND_OGG           =  4,
        SOUND_OGG_STREAMING =  8,
        SOUND_MP3           = 16,
        SOUND_SEQMUSIC      = 32, //MIDI/XM/MOD
        SOUND_OTHER         = 64
    };
    int  cdrom_drive_number;
    char *default_cdrom_drive;
    bool cdaudio_on_flag; // false if mute
    bool volume_on_flag; // false if mute
    SDL_AudioSpec audio_format;
    bool audio_open_flag;

    bool wave_play_loop_flag;
    char *wave_file_name;

    bool seqmusic_play_loop_flag;
    char *seqmusic_file_name;
    Mix_Music *seqmusic_info;

    SDL_CD *cdrom_info;
    int current_cd_track;
    bool cd_play_loop_flag;
    bool music_play_loop_flag;
    bool mp3save_flag;
    char *music_file_name;
    unsigned char *music_buffer; // for looped music
    long music_buffer_length;
    SMPEG *mp3_sample;
    Uint32 mp3fade_start;
    Uint32 mp3fadeout_duration;
    Uint32 mp3fadein_duration;
    Mix_Music *music_info;
    char *loop_bgm_name[2];

    int channelvolumes[ONS_MIX_CHANNELS]; //insani's addition
    bool channel_preloaded[ONS_MIX_CHANNELS]; //seems we need to track this...
    Mix_Chunk *wave_sample[ONS_MIX_CHANNELS+ONS_MIX_EXTRA_CHANNELS];

    char *music_cmd;
    char *seqmusic_cmd;

    int playSound(const char *filename, int format, bool loop_flag, int channel=0);
    void playCDAudio();
    int playWave(Mix_Chunk *chunk, int format, bool loop_flag, int channel);
    int playMP3();
    int playOGG(int format, unsigned char *buffer, long length, bool loop_flag, int channel);
    int playExternalMusic(bool loop_flag);
    int playSequencedMusic(bool loop_flag);
    // Mion: for music status and fades
    int playingMusic();
    int setCurMusicVolume(int volume);
    int setVolumeMute(bool do_mute);

    enum { WAVE_PLAY        = 0,
           WAVE_PRELOAD     = 1,
           WAVE_PLAY_LOADED = 2
    };
    void stopBGM( bool continue_flag );
    void stopDWAVE( int channel );
    void stopAllDWAVE();
    void playClickVoice();
    OVInfo *openOggVorbis(unsigned char *buf, long len, int &channels, int &rate);
    int  closeOggVorbis(OVInfo *ovi);

    /* ---------------------------------------- */
    /* Movie related variables */
    SMPEG *async_movie;
    unsigned char *movie_buffer;
    SDL_Surface *async_movie_surface;
    SDL_Rect async_movie_rect;
    SDL_Rect *surround_rects;
    bool movie_click_flag, movie_loop_flag;
    int playMPEG( const char *filename, bool async_flag, bool use_pos=false, int xpos=0, int ypos=0, int width=0, int height=0 );
    int playAVI( const char *filename, bool click_flag );
    void stopMovie(SMPEG *mpeg);

    /* ---------------------------------------- */
    /* Text event related variables */
    TTF_Font *text_font;
    bool new_line_skip_flag;
    int text_speed_no;

    void displayTextWindow( SDL_Surface *surface, SDL_Rect &clip );
    void clearCurrentPage();
    void newPage( bool next_flag );

    void flush( int refresh_mode, SDL_Rect *rect=NULL, bool clear_dirty_flag=true, bool direct_flag=false );
    void flushDirect( SDL_Rect &rect, int refresh_mode, bool updaterect=true );
    int parseLine();
    
    void readToken();

    void mouseOverCheck( int x, int y );

    /* ---------------------------------------- */
    /* Animation */
    int  proceedAnimation();
    int  proceedCursorAnimation();
    int  estimateNextDuration( AnimationInfo *anim, SDL_Rect &rect, int minimum );
    void resetRemainingTime( int t );
    void resetCursorTime( int t );
#ifdef RCA_SCALE
    void setupAnimationInfo( AnimationInfo *anim, Fontinfo *info=NULL, float stretch_x=1.0, float stretch_y=1.0 );
#else
    void setupAnimationInfo( AnimationInfo *anim, Fontinfo *info=NULL );
#endif
    bool sameImageTag(const AnimationInfo &anim1, const AnimationInfo &anim2);
    void parseTaggedString( AnimationInfo *anim, bool is_mask=false );
    void drawTaggedSurface( SDL_Surface *dst_surface, AnimationInfo *anim, SDL_Rect &clip );
    void stopCursorAnimation( int click );

    /* ---------------------------------------- */
    /* File I/O */
    void searchSaveFile( SaveFileInfo &info, int no );
    int  loadSaveFile( int no, bool input_flag=true );
    void saveMagicNumber( bool output_flag );
    int  saveSaveFile( int no, const char *savestr=NULL, bool no_error=false );

    int  loadSaveFile2( int file_version, bool input_flag=true );
    void saveSaveFile2( bool output_flag );

    /* ---------------------------------------- */
    /* Image processing */
    SDL_Surface *loadImage(char *filename, bool *has_alpha=NULL);
    SDL_Surface *createRectangleSurface(char *filename);
    SDL_Surface *createSurfaceFromFile(char *filename, int *location);

    void shiftCursorOnButton( int diff );
    void effectBlend( SDL_Surface *mask_surface, int trans_mode,
                      Uint32 mask_value = 255, SDL_Rect *clip=NULL,
                      SDL_Surface *src1=NULL, SDL_Surface *src2=NULL,
                      SDL_Surface *dst=NULL );
    void alphaBlendText( SDL_Surface *dst_surface, SDL_Rect dst_rect,
                         SDL_Surface *txt_surface, SDL_Color &color,
                         SDL_Rect *clip, bool rotate_flag );
    void makeNegaSurface( SDL_Surface *surface, SDL_Rect &clip );
    void makeMonochromeSurface( SDL_Surface *surface, SDL_Rect &clip );
    void refreshSurface( SDL_Surface *surface, SDL_Rect *clip_src, int refresh_mode = REFRESH_NORMAL_MODE );
    void createBackground();

    /* ---------------------------------------- */
    /* rmenu and system call */
    bool system_menu_enter_flag;
    int  system_menu_mode;

    enum SelectReleaseFlags {
        SELECT_RELEASE_NONE     = 0,
        SELECT_RELEASE_REQUIRED = 1 << 0,
        SELECT_RELEASE_ENABLED  = 1 << 1,
        SELECT_RELEASE_RGOSUB   = 1 << 2
    };
    int select_release; // Used to break out of "select" command loops

    int  shelter_event_mode;
    int  shelter_display_mode;
    bool shelter_draw_cursor_flag;
    struct Page *cached_page;
    AnimationInfo *system_menu_title;

    enum MessageId {
        MESSAGE_SAVE_EXIST,
        MESSAGE_SAVE_EMPTY,
        MESSAGE_SAVE_CONFIRM,
        MESSAGE_LOAD_CONFIRM,
        MESSAGE_RESET_CONFIRM,
        MESSAGE_END_CONFIRM,
        MESSAGE_YES,
        MESSAGE_NO
    };
    const char* getMessageString( MessageId which );
    
    void enterSystemCall();
    void leaveSystemCall( bool restore_flag = true );
    bool executeSystemCall();

    void executeSystemMenu();
    void executeSystemSkip();
    void executeSystemAutomode();
    bool executeSystemReset();
    void executeSystemEnd();
    void executeWindowErase();
    bool executeSystemLoad();
    void executeSystemSave();
    bool executeSystemYesNo( int caller, int file_no=0 );
    void setupLookbackButton();
    void executeSystemLookback();
};

#endif // __ONSCRIPTER_LABEL_H__
