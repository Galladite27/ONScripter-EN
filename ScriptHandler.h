/* -*- C++ -*-
 * 
 *  ScriptHandler.h - Script manipulation class of ONScripter-EN
 *
 *  Copyright (c) 2001-2009 Ogapee. All rights reserved.
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

// Modified by Mion, April 2009, to update from
// Ogapee's 20090331 release source code.

// Modified by Mion, November 2009, to update from
// Ogapee's 20091115 release source code.

#ifndef __SCRIPT_HANDLER_H__
#define __SCRIPT_HANDLER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "BaseReader.h"
#include "DirPaths.h"
#include "Encoding.h"
#include "Reporter.h"

#define VARIABLE_RANGE 4096

#define IS_TWO_BYTE(x) \
        ( ((x) & 0xe0) == 0xe0 || ((x) & 0xe0) == 0x80 )

#define MAX_ERRBUF_LEN 512

typedef unsigned char uchar3[3];

class ONScripterLabel;

class ScriptHandler
{
public:
    enum { END_NONE       = 0,
           END_COMMA      = 1,
           END_1BYTE_CHAR = 2,
           END_COMMA_READ = 4 // for LUA
    };
    struct LabelInfo{
        char *name;
        char *label_header;
        char *start_address;
        int  start_line;
        int  num_of_lines;
    };

    struct ArrayVariable{
        struct ArrayVariable* next;
        int no;
        int num_dim;
        int dim[20];
        int *data;
        ArrayVariable(){
            next = NULL;
            data = NULL;
        };
        ~ArrayVariable(){
            if (data) delete[] data;
        };
        ArrayVariable& operator=(const ArrayVariable& av){
            no = av.no;
            num_dim = av.num_dim;

            int total_dim = 1;
            for (int i=0 ; i<20 ; i++){
                dim[i] = av.dim[i];
                total_dim *= dim[i];
            }

            if (data) delete[] data;
            data = NULL;
            if (av.data){
                data = new int[total_dim];
                memcpy(data, av.data, sizeof(int)*total_dim);
            }

            return *this;
        };
    };

    enum { VAR_NONE  = 0,
           VAR_INT   = 1,  // integer
           VAR_ARRAY = 2,  // array
           VAR_STR   = 4,  // string
           VAR_CONST = 8,  // direct value or alias, not variable
           VAR_PTR   = 16  // pointer to a variable, e.g. i%0, s%0
    };
    struct VariableInfo{
        int type;
        int var_no;   // for integer(%), array(?), string($) variable
        ArrayVariable array; // for array(?)
    };

    ScriptHandler();
    ~ScriptHandler();

    void reset();
    FILE *fopen( const char *path, const char *mode, const bool save = false, const bool usesavedir = false );
    FILE *fopen( const char *root, const char *path, const char *mode);
    void setKeyTable( const unsigned char *key_table );

    void setSavedir( const char *dir );
    inline void setReporter( Reporter *newreporter){ reporter = newreporter; }

    // basic parser function
    const char *readToken(bool check_pretext);
    const char *readName();
    const char *readColor(bool *is_color = NULL);
    const char *readLabel();
    void readVariable( bool reread_flag=false );
    const char *readStr();
    int  readInt();
    int  parseInt( char **buf );
    void skipToken();

    // function for string access
    inline char *getStringBuffer(){ return string_buffer; };
    inline char *getSavedStringBuffer(){ return saved_string_buffer; };
    char *saveStringBuffer();
    void addStringBuffer( char ch );
    void trimStringBuffer( unsigned int n );
    void pushStringBuffer(int offset); // used in textgosub and pretextgosub
    int  popStringBuffer(); // used in textgosub and pretextgosub
    
    // function for direct manipulation of script address 
    inline char *getCurrent(){ return current_script; };
    inline char *getNext(){ return next_script; };
    void setCurrent(char *pos, bool nowarn=false);
    void pushCurrent( char *pos );
    void popCurrent();

    void enterExternalScript(char *pos); // LUA
    void leaveExternalScript();
    bool isExternalScript();

    int  getOffset( char *pos );
    char *getAddress( int offset );
    int  getLineByAddress( char *address );
    char *getAddressByLine( int line );
    LabelInfo getLabelByAddress( char *address );
    LabelInfo getLabelByLine( int line );

    bool isName( const char *name );
    bool isText();
    bool isPretext();
    bool compareString( const char *buf );
    void setEndStatus(int val){ end_status |= val; };
    inline int getEndStatus(){ return end_status; };
    inline void toggle1ByteEndStatus() {
        if (end_status & END_1BYTE_CHAR)
            end_status &= ~END_1BYTE_CHAR;
        else
            end_status |= END_1BYTE_CHAR;
    }
    void skipLine( int no=1 );
    void setLinepage( bool val ){ linepage_flag = val; };
    void setZenkakko( bool val ){ zenkakko_flag = val; };
    void setEnglishMode( bool val ){ english_mode = val; };

    // function for kidoku history
    bool isKidoku();
    void markAsKidoku( char *address=NULL );
    void setKidokuskip( bool kidokuskip_flag );
    void saveKidokuData(bool no_error=false);
    void loadKidokuData();

    void addStrVariable(char **buf);
    void addIntVariable(char **buf, bool no_zenkaku=false);
    void declareDim();

    void enableTextgosub(bool val);
    void enableRgosub(bool val);
    void setClickstr( const char *list );
    int  checkClickstr(const char *buf, bool recursive_flag=false);

    void setInt( VariableInfo *var_info, int val, int offset=0 );
    void setNumVariable( int no, int val );
    void pushVariable();
    int  getIntVariable( VariableInfo *var_info=NULL );

    void setStr( char **dst, const char *src, int num=-1 );

    int  getStringFromInteger( char *buffer, int no, int num_column,
                               bool is_zero_inserted=false,
                               bool use_zenkaku=false );

    int  readScriptSub( FILE *fp, char **buf, int encrypt_mode );
    int  readScript( DirPaths &path );
    int  labelScript();

    LabelInfo lookupLabel( const char* label );
    LabelInfo lookupLabelNext( const char* label );
    bool hasLabel( const char* label );

    ArrayVariable *getRootArrayVariable();
    void loadArrayVariable( FILE *fp );
    
    void addNumAlias( const char *str, int no );
    void addStrAlias( const char *str1, const char *str2 );

    bool findNumAlias( const char *str, int *value );
    bool findStrAlias( const char *str, char* buffer );

    enum { LABEL_LOG = 0,
           FILE_LOG = 1
    };
    struct LogLink{
        LogLink *next;
        char *name;

        LogLink(){
            next = NULL;
            name = NULL;
        };
        ~LogLink(){
            if ( name ) delete[] name;
        };
    };
    struct LogInfo{
        LogLink root_log;
        LogLink *current_log;
        int num_logs;
        const char *filename;
    } log_info[2];
    LogLink *findAndAddLog( LogInfo &info, const char *name, bool add_flag );
    void resetLog( LogInfo &info );
    
    /* ---------------------------------------- */
    /* Variable */
    struct VariableData{
        int num;
        bool num_limit_flag;
        int num_limit_upper;
        int num_limit_lower;
        char *str;

        VariableData(){
            str = NULL;
            reset(true);
        };
        void reset(bool limit_reset_flag){
            num = 0;
            if (limit_reset_flag)
                num_limit_flag = false;
            if (str){
                delete[] str;
                str = NULL;
            }
        };
    };
    VariableData &getVariableData(int no);

    VariableInfo current_variable, pushed_variable;
    
    int screen_width;
    int screen_height;
    int global_variable_border;
    
    char *game_identifier;
    char *save_path;
    //Mion: savedir is set by savedirCommand, stores save files
    // and main stored gamedata files except envdata
    char *savedir;
    int  game_hash;

    //Mion: for more helpful error msgs
    bool strict_warnings;
    char current_cmd[64];
    enum { CMD_NONE    = 0,
           CMD_BUILTIN = 1,
           CMD_TEXT    = 2,
           CMD_PRETEXT = 3,
           CMD_USERDEF = 4,
           CMD_UNKNOWN = 5
    } current_cmd_type, gosub_cmd_type;
    char errbuf[MAX_ERRBUF_LEN]; //intended for use creating error messages
                                 // before they are passed to errorAndExit,
                                 // simpleErrorAndExit or processError
    void processError( const char *str, const char *title=NULL,
                       const char *detail=NULL, bool is_warning=false,
                       bool is_simple=false );

    BaseReader *cBR;
    Encoding enc;

    enum LanguageScript {
        NO_SCRIPT_PREF  = 0,
        LATIN_SCRIPT    = 1,
        JAPANESE_SCRIPT = 2
    } preferred_script, default_script, system_menu_script;

    //Mion: these are used to keep track of clickwait points in the script
    //for rgosub, to use as return script points
    char **rgosub_wait_pos;
    bool *rgosub_wait_1byte;
    int total_rgosub_wait_size;
    int num_rgosub_waits;
    int cur_rgosub_wait;
    
    bool is_rgosub_click;
    bool rgosub_click_newpage;
    bool rgosub_1byte_mode;

    bool ignore_textgosub_newline;

    //Mion: onscripter-en special text escape characters
    enum {
        TXTBTN_START = 0x01, // for '<' in unmarked text as a textbtn delimiter
        LEFT_PAREN   = 0x02, // for '(' in text and backlog, to avoid ruby
        RIGHT_PAREN  = 0x03, // for ')' in text and backlog, to avoid ruby
        TXTBTN_END   = 0x04, // for '>' in unmarked text as a textbtn delimiter
        BACKSLASH    = 0x1F  // for '\' in {}-varlist strings _within script_ (indicates str newline)
    };

    //Mion: for lookback text relocating
    //  Watch onscripter play teletype :)
    enum {
        TEXT_TAB   = 0x09,  //horizontal tab, like indenting 1 fullwidth space
        TEXT_LF    = 0x0A,  //newline!
        TEXT_VTAB  = 0x0B,  //like newline, but doesn't change position in line
        TEXT_FF    = 0x0C,  //implicit "locate -1,0"; form feed
        TEXT_CR    = 0x0D,  //implicit "locate 0,-1"; carriage return
        TEXT_UP    = 0x11,  //move up 1 character on screen (deprecated)
        TEXT_RIGHT = 0x12,  //move right 1 fullwidth character on screen (deprecated)
        TEXT_DOWN  = 0x13,  //move down 1 character on screen (deprecated)
        TEXT_LEFT  = 0x14   //move left 1 fullwidth character on screen (deprecated)
    };
    
private:
    enum { OP_INVALID = 0, // 000
           OP_PLUS    = 2, // 010
           OP_MINUS   = 3, // 011
           OP_MULT    = 4, // 100
           OP_DIV     = 5, // 101
           OP_MOD     = 6  // 110
    };
    
    struct Alias{
        struct Alias *next;
        char *alias;
        int  num;
        char *str;

        Alias(){
            next = NULL;
            alias = NULL;
            str = NULL;
        };
        Alias( const char *name, int num ){
            next = NULL;
            alias = new char[ strlen(name) + 1];
            strcpy( alias, name );
            str = NULL;
            this->num = num;
        };
        Alias( const char *name, const char *str ){
            next = NULL;
            alias = new char[ strlen(name) + 1];
            strcpy( alias, name );
            this->str = new char[ strlen(str) + 1];
            strcpy( this->str, str );
        };
        ~Alias(){
            if (alias) delete[] alias;
            if (str)   delete[] str;
        };
    };
    
    int findLabel( const char* label );

    char *checkComma( char *buf );
    void parseStr( char **buf );
    int  parseIntExpression( char **buf );
    void readNextOp( char **buf, int *op, int *num );
    int  calcArithmetic( int num1, int op, int num2 );
    int  parseArray( char **buf, ArrayVariable &array );
    int  *getArrayPtr( int no, ArrayVariable &array, int offset );

    /* ---------------------------------------- */
    /* Variable */
    struct VariableData *variable_data;
    struct ExtendedVariableData{
        int no;
        VariableData vd;
    } *extended_variable_data;
    int num_extended_variable_data;
    int max_extended_variable_data;
    struct TmpVariableDataLink{
        VariableInfo vi;
        int num;
        char *str;
        TmpVariableDataLink *next;
        TmpVariableDataLink(){
            str = NULL;
            next = NULL;
        };
        ~TmpVariableDataLink(){
            if (str) delete[] str;
        };
    } tmp_variable_data_link;

    Alias root_num_alias, *last_num_alias;
    Alias root_str_alias, *last_str_alias;
    
    ArrayVariable *root_array_variable, *current_array_variable;

    Reporter *reporter; //SeanMcG: error reporting interface
    void errorAndExit( const char *str, const char *title=NULL, const char *detail=NULL, bool is_warning=false );
    void errorAndCont( const char *str, const char *title=NULL, const char *detail=NULL );
    void simpleErrorAndExit( const char *str, const char *title=NULL, const char *detail=NULL, bool is_warning=false );

    DirPaths *archive_path; //points to ScriptParser's archive_path
    char *script_path;
    int  script_buffer_length;
    char *script_buffer;
    char *tmp_script_buf;
    
    char *string_buffer; // update only be readToken
    int  string_counter;
    char *saved_string_buffer; // updated only by saveStringBuffer
    char *str_string_buffer; // updated only by readStr
    char *gosub_string_buffer; // used in textgosub and pretextgosub
    int gosub_string_offset; // used in textgosub and pretextgosub

    LabelInfo *label_info;
    int num_of_labels;

    bool skip_enabled;
    bool kidokuskip_flag;
    char *kidoku_buffer;

    bool zenkakko_flag;
    int  end_status;
    bool linepage_flag;
    bool textgosub_flag;
    bool rgosub_flag;
    char *clickstr_list;
    bool english_mode;

    char *current_script;
    char *next_script;

    char *pushed_current_script;
    char *pushed_next_script;

    char *internal_current_script;
    char *internal_next_script;
    int  internal_end_status;
    VariableInfo internal_current_variable, internal_pushed_variable;

    unsigned char key_table[256];
    bool key_table_flag;
};


#endif // __SCRIPT_HANDLER_H__
