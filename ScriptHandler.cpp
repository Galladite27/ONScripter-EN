/* -*- C++ -*-
 *
 *  ScriptHandler.cpp - Script manipulation class of ONScripter-EN
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

#include "ScriptHandler.h"
#include "Encoding.h"
#include "ONScripterLabel.h" //so this can call doErrorBox
#include <sys/stat.h>
#include <sys/types.h>
#ifdef WIN32
#include <direct.h>
#include <windows.h>
#endif

#define TMP_SCRIPT_BUF_LEN 4096
#define STRING_BUFFER_LENGTH 2048

#define SKIP_SPACE(p) while ( *(p) == ' ' || *(p) == '\t' ) (p)++

ScriptHandler::ScriptHandler()
{
    num_of_labels = 0;
    script_buffer = NULL;
    kidoku_buffer = NULL;
    log_info[LABEL_LOG].filename = "NScrllog.dat";
    log_info[FILE_LOG].filename  = "NScrflog.dat";
    clickstr_list = NULL;

    string_buffer       = new char[STRING_BUFFER_LENGTH];
    str_string_buffer   = new char[STRING_BUFFER_LENGTH];
    saved_string_buffer = new char[STRING_BUFFER_LENGTH];
    gosub_string_buffer = new char[STRING_BUFFER_LENGTH];

    variable_data = new VariableData[VARIABLE_RANGE];
    extended_variable_data = NULL;
    num_extended_variable_data = 0;
    max_extended_variable_data = 1;
    root_array_variable = NULL;

    screen_width = 640;
    screen_height = 480;
    global_variable_border = 200;
    
    ons = NULL;
    
    archive_path = NULL;
    save_path = NULL;
    savedir = NULL;
    script_path = NULL;
    game_identifier = NULL;
    game_hash = 0;
    current_cmd[0] = '\0';
    current_cmd_type = gosub_cmd_type = CMD_NONE;
    zenkakko_flag = false;
    strict_warnings = false;

    default_script = NO_SCRIPT_PREF;
    preferred_script = NO_SCRIPT_PREF;
    system_menu_script = NO_SCRIPT_PREF;

    rgosub_wait_pos = NULL;
    rgosub_wait_1byte = NULL;
    total_rgosub_wait_size = num_rgosub_waits = cur_rgosub_wait = 0;
    is_rgosub_click = rgosub_click_newpage = rgosub_1byte_mode = false;
    rgosub_flag = false; // roto 20100205

    ignore_textgosub_newline = false;
}

ScriptHandler::~ScriptHandler()
{
    reset();

    if ( script_buffer ) delete[] script_buffer;
    if ( kidoku_buffer ) delete[] kidoku_buffer;

    delete[] string_buffer;
    delete[] str_string_buffer;
    delete[] saved_string_buffer;
    delete[] gosub_string_buffer;
    delete[] variable_data;
    
    if (script_path) delete[] script_path;
    if (save_path) delete[] save_path;
    if (savedir) delete[] savedir;

    if (game_identifier) delete[] game_identifier;
}

void ScriptHandler::reset()
{
    //for (int i=0 ; i<VARIABLE_RANGE ; i++)
    // This should NEVER have got to release... Anything like definereset
    // will wipe global variables when using the old comparison.
    // -Galladite 2024-07-16
    //
    // As of 2024-07-21 this issue still persists despite this fix having
    // been implemented. Needs further investigation. -Galladite
    for (int i=0 ; i<global_variable_border ; i++)
        variable_data[i].reset(true);

    // However, I'm not doing anything about this because global variables
    // don't work with extended_variable_data, at least for now.
    // -Galladite 2024-07-16
    if (extended_variable_data) delete[] extended_variable_data;
    extended_variable_data = NULL;
    num_extended_variable_data = 0;
    max_extended_variable_data = 1;

    ArrayVariable *av = root_array_variable;
    while(av){
        ArrayVariable *tmp = av;
        av = av->next;
        delete tmp;
    }
    root_array_variable = current_array_variable = NULL;

    // reset log info
    resetLog( log_info[LABEL_LOG] );
    resetLog( log_info[FILE_LOG] );

    // reset number alias
    Alias *alias;
    alias = root_num_alias.next;
    while (alias){
        Alias *tmp = alias;
        alias = alias->next;
        delete tmp;
    };
    last_num_alias = &root_num_alias;
    last_num_alias->next = NULL;

    // reset string alias
    alias = root_str_alias.next;
    while (alias){
        Alias *tmp = alias;
        alias = alias->next;
        delete tmp;
    };
    last_str_alias = &root_str_alias;
    last_str_alias->next = NULL;

    // reset misc. variables
    end_status = END_NONE;
    kidokuskip_flag = false;
    current_cmd[0] = '\0';
    current_cmd_type = gosub_cmd_type = CMD_NONE;
    zenkakko_flag = false;
    linepage_flag = false;
    english_mode = false;
    textgosub_flag = false;
    skip_enabled = false;
    if (clickstr_list){
        delete[] clickstr_list;
        clickstr_list = NULL;
    }
    internal_current_script = NULL;
    preferred_script = default_script;

    if (rgosub_wait_pos) delete[] rgosub_wait_pos;
    rgosub_wait_pos = NULL;
    if (rgosub_wait_1byte) delete[] rgosub_wait_1byte;
    rgosub_wait_1byte = NULL;
    total_rgosub_wait_size = num_rgosub_waits = cur_rgosub_wait = 0;
    is_rgosub_click = rgosub_click_newpage = rgosub_1byte_mode = false;
}

FILE *ScriptHandler::fopen( const char *path, const char *mode, const bool save, const bool usesavedir )
{
    const char* root;
    char *file_name;
    FILE *fp = NULL;

    if (usesavedir && savedir) {
        root = savedir;
        file_name = new char[strlen(root)+strlen(path)+1];
        sprintf( file_name, "%s%s", root, path );
        //printf("handler:fopen(\"%s\")\n", file_name);

        fp = ::fopen( file_name, mode );
    } else if (save) {
        root = save_path;
        file_name = new char[strlen(root)+strlen(path)+1];
        sprintf( file_name, "%s%s", root, path );
        //printf("handler:fopen(\"%s\")\n", file_name);

        fp = ::fopen( file_name, mode );
    } else {
        // search within archive_path(s)
        file_name = new char[archive_path->max_path_len()+strlen(path)+1];
        for (int n=0; n<archive_path->get_num_paths(); n++) {
            root = archive_path->get_path(n);
            //printf("root: %s\n", root);
            sprintf( file_name, "%s%s", root, path );
            //printf("handler:fopen(\"%s\")\n", file_name);
            fp = ::fopen( file_name, mode );
            if (fp != NULL) break;
        }
    }
    delete[] file_name;
    return fp;
}

FILE *ScriptHandler::fopen( const char *root, const char *path, const char *mode )
{
    char *file_name;
    FILE *fp = NULL;

    file_name = new char[strlen(root)+strlen(path)+1];
    sprintf( file_name, "%s%s", root, path );
    //printf("handler:fopen(\"%s\")\n", file_name);

    fp = ::fopen( file_name, mode );

    delete[] file_name;
    return fp;
}

void ScriptHandler::setKeyTable( const unsigned char *key_table )
{
    int i;
    if (key_table){
        key_table_flag = true;
        for (i=0 ; i<256 ; i++) this->key_table[i] = key_table[i];
    }
    else{
        key_table_flag = false;
        for (i=0 ; i<256 ; i++) this->key_table[i] = i;
    }
}

void ScriptHandler::setSavedir( const char *dir )
{
    savedir = new char[ strlen(dir) + strlen(save_path) + 2];
    sprintf( savedir, "%s%s%c", save_path, dir, DELIMITER );
    mkdir(savedir
#ifndef WIN32
          , 0755
#endif
         );
}

// basic parser function
const char *ScriptHandler::readToken(bool check_pretext)
{
    current_script = next_script;
    char *buf = current_script;
    end_status = END_NONE;
    current_variable.type = VAR_NONE;
    num_rgosub_waits = cur_rgosub_wait = 0;

    current_cmd_type = CMD_NONE;

    if (rgosub_flag && is_rgosub_click){
        string_counter = 0;
        char ch = rgosub_click_newpage ? '\\' : '@';
        addStringBuffer( ch );
        if (rgosub_1byte_mode)
            addStringBuffer( '`' );
        rgosub_wait_1byte[num_rgosub_waits] = rgosub_1byte_mode;
        rgosub_wait_pos[num_rgosub_waits++] = buf;
        current_cmd_type = CMD_TEXT;
        if (!rgosub_1byte_mode){
            SKIP_SPACE( buf );
        }
    } else {
        SKIP_SPACE( buf );
    }

    markAsKidoku( buf );

    if(enc.getEncoding() == Encoding::CODE_UTF8)
    {
        english_mode = true;
    }

  readTokenTop:
    if (!is_rgosub_click)
        string_counter = 0;
    char ch = *buf;
    if ((ch == ';') && !is_rgosub_click){ // comment
        while ( ch != 0x0a && ch != '\0' ){
            addStringBuffer( ch );
            ch = *++buf;
        }
    }
    else if (is_rgosub_click || ch & 0x80 ||
             (ch >= '0' && ch <= '9') ||
             ch == '@' || ch == '\\' || ch == '/' ||
             ch == '%' || ch == '?' || ch == '$' ||
             ch == '[' || ch == '(' || ch == '`' ||
             ch == '!' || ch == '#' || ch == ',' ||
             ch == '{' || ch == '<' || ch == '>' ||
             ch == '"' ||
             (enc.getEncoding() == Encoding::CODE_UTF8 &&
              ch == enc.getTextMarker())
             ){ // text or pretext

        //Mion: parsing pretext tags as separate tokens from text,
        //  which is what NScr seems to be doing
        if (check_pretext && !is_rgosub_click &&
            ( (ch == '[') ||
              (zenkakko_flag && (ch == (char)0x81) && (buf[1] == (char)0x79)) )) {

            //pretext tag
            current_cmd_type = CMD_PRETEXT;
            if (ch != '[') ++buf; //fullwidth bracket is 2 bytes
            ch = *++buf;

            bool in_1byte_mode = false;
            while (1){
                int n = enc.getBytes(ch);

                /* Check if we should exit pretext tag */
                if (!in_1byte_mode && zenkakko_flag &&
                     (ch == (char)0x81) &&
                     (buf[1] == (char)0x7A)) {
                    buf += 2;
                    break;
                } else if (!in_1byte_mode && (ch == ']')) {
                    ++buf;
                    break;
                } else if ((ch == 0x0a) || (ch == 0x00)) {
                    //Mion: NScr will actually parse until a terminating ']',
                    //  even including newlines... probably not used by anything
                    // (or a good idea), so we'll stop parsing at end of line
                    errorAndCont( "readToken: unterminated pretext tag" );
                    break;
                }

                /* Check if we should toggle 1 byte mode */
                else if (ch == '`') {
                    in_1byte_mode = !in_1byte_mode;
                    // There is no need to advance buf here; since
                    // there is no break, it will be done below
                }

                /* For each byte in the character, advance 1 byte to
                 * reach the beginning of the next
                 */
                for (int i=0; i<n; i++) {
                    addStringBuffer( ch );
                    ch = *++buf;
                }

            }
            next_script = buf;
            return string_buffer;
        }

        current_cmd_type = CMD_TEXT;
        bool ignore_click_flag = false;
        bool clickstr_flag = false;
        bool is_nscr_english = (english_mode && (ch == '>'));
        if (is_nscr_english)
            ch = *++buf;
        bool in_1byte_mode = is_nscr_english;
        if (is_rgosub_click)
            in_1byte_mode = rgosub_1byte_mode;
        while (1){
            if (rgosub_flag && (num_rgosub_waits == total_rgosub_wait_size)){
                //double the rgosub wait buffer size
                // Does this need changing? I don't really know if
                // this is UTF-8 related -Galladite 2023-06-20
                char **tmp = rgosub_wait_pos;
                bool *tmp2 = rgosub_wait_1byte;
                total_rgosub_wait_size *= 2;
                rgosub_wait_pos = new char*[total_rgosub_wait_size];
                rgosub_wait_1byte = new bool[total_rgosub_wait_size];
                for (int i=0; i<num_rgosub_waits; i++){
                    rgosub_wait_pos[i] = tmp[i];
                    rgosub_wait_1byte[i] = tmp2[i];
                }
                delete[] tmp;
                delete[] tmp2;
            }
            char *tmp = buf;
            SKIP_SPACE(tmp);
            if (!(is_nscr_english || in_1byte_mode) ||
                (*tmp == 0x0a) || (*tmp == 0x00)) {
                // always skip trailing spaces
                buf = tmp;
                ch = *buf;
            }
            if ((ch == 0x0a) || (ch == 0x00)) break;
// TODO after testing remove the conditionals
#define readt_testing

#ifdef readt_testing
            int n = enc.getBytes(ch);
            if ( n > 1 ) {
#else
            if ( IS_TWO_BYTE(ch) ) {
#endif
                if (!ignore_click_flag &&
                    (checkClickstr(buf) > 0))
                    clickstr_flag = true;
                else
                    clickstr_flag = false;
#ifdef readt_testing
                // We already know the first byte isn't 0x0a or \0
                addStringBuffer( ch );
                ch = *++buf;
                for (int i=1; i<n; i++) {
                    // Invalid multi-byte character:
                    if (ch == 0x0a || ch == '\0') break;
                    addStringBuffer( ch );
                    ch = *++buf;
                }
#else
                addStringBuffer( ch );
                ch = *++buf;
                if (ch == 0x0a || ch == '\0') break; //invalid 2-byte char
                addStringBuffer( ch );
                ch = *++buf;
#endif
                //Mion: ons-en processes clickstr chars here in readToken,
                // not in ONScripterLabel_text - adds a special
                // sequence '\@' after the clickstr char
                if (clickstr_flag) {
                    // insert a clickwait-or-newpage
                    addStringBuffer('\\');
                    addStringBuffer('@');
                    if (textgosub_flag) {
                        char *tmp = buf;
                        SKIP_SPACE(tmp);
                        // if "ignore-textgosub-newline" cmd-line option,
                        // ignore newline after clickwait if textgosub used
                        // (fixes some older onscripter-en games)
                        if (ignore_textgosub_newline && (*tmp == 0x0a))
                            buf = tmp+1;
                        break;
                    }
                    if (rgosub_flag){
                        rgosub_wait_1byte[num_rgosub_waits] = in_1byte_mode;
                        rgosub_wait_pos[num_rgosub_waits++] = buf+1;
                    }
                }
                ignore_click_flag = clickstr_flag = false;
            }
            // 1 byte character read
            else {
                // Toggle 1 byte mode
                if (ch == '`'){
                    addStringBuffer( ch );
                    ch = *++buf;
                    if (!is_nscr_english)
                        in_1byte_mode = !in_1byte_mode;
                }
                // 1 byte mode special syntax markers
                else if (in_1byte_mode) {
                    if (ch == '$'){
                        if (buf[1] == '$') ++buf; else{
                            addStrVariable(&buf);
                            while (*--buf == ' ');
                            ch = *++buf;
                            ignore_click_flag = false;
                            continue;
                        }
                    }
                    /* TODO: integer variables are not interpolated
                     * into text when in one byte mode. Should this be
                     * fixed? -Galladite 2023-06-20
                     *
                     * Update: no it shouldn't. I don't remember the
                     * reason, but ChaosKaiser said it's like that on
                     * purpose. -Galladite 2024-07-16
                     */
                    if ((ch == '_') && (checkClickstr(buf+1) > 0)) {
                        ignore_click_flag = true;
                        ch = *++buf;
                        continue;
                    }
                    if ((ch == '@') || (ch == '\\')) {
                        if (!ignore_click_flag){
                            addStringBuffer( ch );
                            if (rgosub_flag){
                                rgosub_wait_1byte[num_rgosub_waits] = in_1byte_mode;
                                rgosub_wait_pos[num_rgosub_waits++] = buf+1;
                            }
                        }
                        if (textgosub_flag){
                            ++buf;
                            // if "ignore-textgosub-newline", ignore
                            // newline after clickwait if textgosub
                            // (fixes older onscripter-en games)
                            char *tmp = buf;
                            SKIP_SPACE(tmp);
                            if (ignore_textgosub_newline &&(*tmp == 0x0a))
                                buf = tmp+1;
                            break;
                        }
                        ch = *++buf;
                        continue;
                    }
                    if (!ignore_click_flag && (checkClickstr(buf) > 0))
                        clickstr_flag = true;
                    else
                        clickstr_flag = false;
                    // no ruby in 1byte mode; escape parens
                    if (ch == '(') {
                        addStringBuffer( LEFT_PAREN );
                    } else if (ch == ')') {
                        addStringBuffer( RIGHT_PAREN );
                    } else if (ch == 0x0a || ch == '\0') break;
                    // Advance non-syntax, 1 byte, 1bm characters here
                    // (The majority of English text)
                    else
                        addStringBuffer( ch );
                    ch = *++buf;
                    //Mion: ons-en processes clickstr chars here in readToken,
                    // not in ONScripterLabel_text - adds a special
                    // sequence '\@' after the clickstr char
                    if (clickstr_flag) {
                        // insert a clickwait-or-newpage
                        addStringBuffer('\\');
                        addStringBuffer('@');
                        if (textgosub_flag) break;
                        if (rgosub_flag){
                            rgosub_wait_1byte[num_rgosub_waits] = in_1byte_mode;
                            rgosub_wait_pos[num_rgosub_waits++] = buf;
                        }
                    }
                    ignore_click_flag = clickstr_flag = false;
                }
                // Not in one byte mode, but one byte character read
                else{
                    // Interpret click waits
                    if ((ch == '@') || (ch == '\\')) {
                        if (!ignore_click_flag){
                            addStringBuffer( ch );
                            if (rgosub_flag){
                                rgosub_wait_1byte[num_rgosub_waits] = in_1byte_mode;
                                rgosub_wait_pos[num_rgosub_waits++] = buf+1;
                            }
                        }
                        if (textgosub_flag){
                            ++buf;
                            // if "ignore-textgosub-newline", ignore
                            // newline after clickwait if textgosub
                            // (fixes older onscripter-en games)
                            char *tmp = buf;
                            SKIP_SPACE(tmp);
                            if (ignore_textgosub_newline &&(*tmp == 0x0a))
                                buf = tmp+1;
                            break;
                        }
                        ch = *++buf;
                        continue;
                    }
                    // One-byte (but not one byte mode) syntax markers
                    // Aka syntax markers found in Japanese text
                    if (ch == '%' || ch == '?'){
                        addIntVariable(&buf);
                    }
                    else if (ch == '$'){
                        addStrVariable(&buf);
                        // Fix - we should create a save point whenever printing anything
                        // through interpolating variables, etc. because if text is
                        // printed like this without printing text normally beforehand,
                        // there will be no point set to return to from an rmenu call and
                        // so the engine will crash.
                        //
                        // In other words, only through printing text through a variable
                        // can a clickwait be initiated (and thus a custom rmenu be
                        // gosubbed to) before any save points have been created to jump
                        // back to after the rgosub routine is returned from.
                        //
                        // - Galladite 2024-07-16
                        //
                        //
                        //
                        // Update: temporarily (tm) disabled since we actually need to
                        // know whether the text is printed or not to decide whether or
                        // not to set a save point - but this is too early on to know
                        // that. In the meantime (at least for eien) when functions are
                        // used to print from variables without other text, we are in
                        // the function printing a nothing beforehand, just so that
                        // creates the rgosub wait position. I suggest you do the same.
                        //
                        // - Galladite 2024-07-21
                        //
                        //if ( /*isText() &&*/ rgosub_flag){
                        //    // Check if in 1-byte text mode OR actually printing text
                        //    // (OR at the beginning of a line, so printing text?)
                        //    printf("$ found *in text* - applying fix\n\n");
                        //    rgosub_wait_1byte[num_rgosub_waits] = in_1byte_mode;
                        //    rgosub_wait_pos[num_rgosub_waits++] = buf+1;
                        //}
                    }
                    else if (ch == '<'){
                        addStringBuffer(TXTBTN_START);
                        ch = *++buf;
                    }
                    else if (ch == '>'){
                        addStringBuffer(TXTBTN_END);
                        ch = *++buf;
                    }
                    else if (ch == '{') {
                        // comma list of var/val pairs
                        buf++;
                        pushCurrent(buf);
                        next_script = buf;
                        TmpVariableDataLink *tmp = &tmp_variable_data_link;
                        while (tmp->next != NULL)
                            tmp = tmp->next;
                        while( *buf != '}' ) {
                        
                            readVariable();
                            pushVariable();
                            tmp->next = new TmpVariableDataLink;
                            tmp = tmp->next;
                            tmp->vi.var_no = pushed_variable.var_no;
                            tmp->vi.type = pushed_variable.type;
                            VariableData &vd = getVariableData(tmp->vi.var_no);
                            tmp->num = vd.num;
                            //printf("variable: $%d\n", pushed_variable.var_no);
                            buf = next_script;

                            if ( tmp->vi.type & VAR_INT ) {
                                tmp->num = parseIntExpression(&buf);
                                //printf("int: %d\n", x);
                            } else if ( tmp->vi.type & VAR_STR ) {
                                bool invar_1byte_mode = false;
                                int tmp_count = 0;
                                strcpy(saved_string_buffer, "");
                                /* Begin reading string data */
                                while (*buf != 0x0a && *buf != '\0' &&
                                       (invar_1byte_mode || ((*buf != ',') && (*buf != '}')))) {
                                    if (*buf == '`')
                                        invar_1byte_mode = !invar_1byte_mode;
#ifdef readt_testing
                                    n = enc.getBytes(ch);
                                    if (tmp_count+n >= STRING_BUFFER_LENGTH)
                                        errorAndExit("readToken: var string length exceeds 2048 bytes.");
                                    if ((*buf == '\\') || (*buf == BACKSLASH)) {
                                        //Mion: I really shouldn't be modifying
                                        //  the script buffer FIXME
                                        saved_string_buffer[tmp_count++] = '\\';
                                        *buf++ = BACKSLASH;
                                    } else {
                                        for (int i=0; i<n; i++) {
                                            saved_string_buffer[tmp_count++] = *buf++;
                                        }
                                    }
                                    saved_string_buffer[tmp_count] = '\0';
#else
                                    if ((tmp_count+1 >= STRING_BUFFER_LENGTH) ||
                                        (IS_TWO_BYTE(*buf) && (tmp_count+2 >= STRING_BUFFER_LENGTH)))
                                        errorAndExit("readToken: var string length exceeds 2048 bytes.");
                                    else if (IS_TWO_BYTE(*buf)) {
                                        saved_string_buffer[tmp_count++] = *buf++;
                                        saved_string_buffer[tmp_count++] = *buf++;
                                    } else if ((*buf == '\\') || (*buf == BACKSLASH)) {
                                        //Mion: I really shouldn't be modifying
                                        //  the script buffer FIXME
                                        saved_string_buffer[tmp_count++] = '\\';
                                        *buf++ = BACKSLASH;
                                    } else
                                        saved_string_buffer[tmp_count++] = *buf++;
                                    saved_string_buffer[tmp_count] = '\0';
#endif
                                }
                                /* Finish reading string data */
                                setStr( &tmp->str, saved_string_buffer );
                                //printf("string: %s\n", saved_string_buffer);
                            }
                            next_script = checkComma(buf);
                            buf = next_script;
                            if (!(getEndStatus() & END_COMMA)) break;
                        }
                        end_status = END_NONE;
                        current_variable.type = VAR_NONE;
                        popCurrent();
                        if (*buf == '}')
                            buf++;
                    }
                    else{
                        if (ch == '_')
                            ignore_click_flag = true;
                        else
                            ignore_click_flag = false;
                        // Advance 1 char
                        addStringBuffer( ch );
                        buf++;
                        // If the next char is a space, advance one
                        // more char (Why? won't this be handled on
                        // the next pass?) -Galladite 2023-06-20
                        if (*buf == ' ') {
                            addStringBuffer( *buf );
                            buf++;
                        }
                    }
                    // Set the next ch to be the char pointerd to by
                    // the now-advanced buffer
                    ch = *buf;
                    // Again, wouldn't this be handled on the next pass?
                    // If it ain't broke, don't fix it, I guess
                    if (ch == 0x0a || ch == '\0') break;
                }
            }
        }
        //now process any {} tmp variables
        TmpVariableDataLink *tmp = tmp_variable_data_link.next;
        while (tmp != NULL) {
            if ( tmp->vi.type & VAR_INT )
                setInt( &tmp->vi, tmp->num);
            else if ( tmp->vi.type & VAR_STR )
                setStr( &variable_data[ tmp->vi.var_no ].str, tmp->str );
            TmpVariableDataLink *last = tmp;
            tmp = tmp->next;
            delete last;
        }
        tmp_variable_data_link.next = NULL;
        current_cmd_type = CMD_TEXT;
    }
    else if ((ch >= 'a' && ch <= 'z') ||
             (ch >= 'A' && ch <= 'Z') ||
             ch == '_'){ // command
        do{
            if (ch >= 'A' && ch <= 'Z') ch += 'a' - 'A';
            addStringBuffer( ch );
            ch = *++buf;
        }
        while((ch >= 'a' && ch <= 'z') ||
              (ch >= 'A' && ch <= 'Z') ||
              (ch >= '0' && ch <= '9') ||
              ch == '_');
    }
    else if (ch == '*'){ // label
        return readLabel();
    }
    else if ((ch == 0x0a) && !is_rgosub_click){
        addStringBuffer( ch );
        markAsKidoku( buf++ );
    }
    else if (ch == '~' || ch == ':'){
        addStringBuffer( ch );
        markAsKidoku( buf++ );
    }
    else if (ch != '\0'){
        snprintf(errbuf, MAX_ERRBUF_LEN,
                 "readToken: skipping unknown heading character %c (%x)", ch, ch);
        errorAndCont( errbuf );
        buf++;
        goto readTokenTop;
    }
    is_rgosub_click = false;
    next_script = checkComma(buf);

    //printf("readToken [%s] len=%d [%c(%x)] %p\n", string_buffer, strlen(string_buffer), ch, ch, next_script);

    return string_buffer;
}

const char *ScriptHandler::readName()
{
    // bare word - not a string variable
    end_status = END_NONE;
    current_variable.type = VAR_NONE;

    current_script = next_script;
    SKIP_SPACE( current_script );
    char *buf = current_script;

    string_counter = 0;
    char ch = *buf;
    if ( ((ch >= 'a') && (ch <= 'z')) ||
         ((ch >= 'A') && (ch <= 'Z')) ||
         (ch == '_') ){
        if ( (ch >= 'A') && (ch <= 'Z') )
            ch += 'a' - 'A';
        addStringBuffer( ch );
        ch = *(++buf);
        while( ((ch >= 'a') && (ch <= 'z')) ||
               ((ch >= 'A') && (ch <= 'Z')) ||
               ((ch >= '0') && (ch <= '9')) ||
               (ch == '_') ){
            if ( (ch >= 'A') && (ch <= 'Z') )
                ch += 'a' - 'A';
            addStringBuffer( ch );
            ch = *(++buf);
        }
    }
    addStringBuffer( '\0' );

    next_script = checkComma(buf);

    return string_buffer;
}

const char *ScriptHandler::readColor(bool *is_color)
{
    // bare color type - not a string variable
    end_status = END_NONE;
    current_variable.type = VAR_NONE;

    current_script = next_script;
    SKIP_SPACE( current_script );
    char *buf = current_script;

    string_counter = 0;
    addStringBuffer( '#' );
    char ch = *(++buf);
    int i;
    for (i=0; i<7; i++) {
        if ( ((ch >= '0') && (ch <= '9')) ||
             ((ch >= 'a') && (ch <= 'f')) ||
             ((ch >= 'A') && (ch <= 'F')) ) {
            addStringBuffer( ch );
            ch = *(++buf);
        } else
            break;
    }
    if (i!=6) {
        if (is_color != NULL) {
            *is_color = false;
            string_counter = 0;
            addStringBuffer( '\0' );
            return string_buffer;
        } else {
            strncpy(string_buffer, current_script, 16);
            string_buffer[16] = '\0';
            errorAndExit( "readColor: not a valid color type." );
        }
    }
    addStringBuffer( '\0' );
    next_script = checkComma(buf);
    if (is_color != NULL)
        *is_color = true;

    return string_buffer;
}

const char *ScriptHandler::readLabel()
{
    // *NAME, "*NAME", or $VAR that equals "*NAME"
    end_status = END_NONE;
    current_variable.type = VAR_NONE;

    current_script = next_script;
    SKIP_SPACE( current_script );
    char *buf = current_script;
    char *tmp = NULL;

    string_counter = 0;
    char ch = *buf;
    if ((ch == '$') || (ch == '"') || (ch == '`')){
        parseStr(&buf);
        tmp = buf;
        string_counter = 0;
        buf = str_string_buffer;
        SKIP_SPACE(buf);
        ch = *buf;
    }
    if (ch == '*') {
        while (ch == '*'){
            addStringBuffer( ch );
            ch = *(++buf);
        }
        SKIP_SPACE(buf);

        ch = *buf;
        while( ((ch >= 'a') && (ch <= 'z')) ||
               ((ch >= 'A') && (ch <= 'Z')) ||
               ((ch >= '0') && (ch <= '9')) ||
               (ch == '_') ){
            if ( (ch >= 'A') && (ch <= 'Z') )
                ch += 'a' - 'A';
            addStringBuffer( ch );
            ch = *(++buf);
        }
    }
    addStringBuffer( '\0' );
    if ( (string_buffer[0] == '\0') || (string_buffer[1] == '\0') ){
        buf = current_script;
        while (*buf && (*buf != 0x0a))
            ++buf;
        ch = *buf;
        *buf = '\0';
        if (tmp != NULL) {
            snprintf(errbuf, MAX_ERRBUF_LEN, 
                     "Invalid label specification '%s' ('%s')",
                     current_script, str_string_buffer);
            *buf = ch;
            errorAndExit(errbuf);
        } else {
            snprintf(errbuf, MAX_ERRBUF_LEN,
                     "Invalid label specification '%s'", current_script);
            *buf = ch;
            errorAndExit(errbuf);
        }
    }
    if (tmp != NULL)
        buf = tmp;

    next_script = checkComma(buf);

    return string_buffer;
}

const char *ScriptHandler::readStr()
{
    end_status = END_NONE;
    current_variable.type = VAR_NONE;

    current_script = next_script;
    SKIP_SPACE( current_script );
    char *buf = current_script;

    string_buffer[0] = '\0';
    string_counter = 0;

    while(1){
        parseStr(&buf);
        buf = checkComma(buf);
        string_counter += strlen(str_string_buffer);
        if (string_counter+1 >= STRING_BUFFER_LENGTH)
            errorAndExit("readStr: string length exceeds 2048 bytes.");
        strcat(string_buffer, str_string_buffer);
        if (buf[0] != '+') break;
        buf++;
    }
    next_script = buf;

    return string_buffer;
}

int ScriptHandler::readInt()
{
    string_counter = 0;
    string_buffer[string_counter] = '\0';

    end_status = END_NONE;
    current_variable.type = VAR_NONE;

    current_script = next_script;
    SKIP_SPACE( current_script );
    char *buf = current_script;

    int ret = parseIntExpression(&buf);

    next_script = checkComma(buf);

    return ret;
}

void ScriptHandler::skipToken()
{
    SKIP_SPACE( current_script );
    char *buf = current_script;

    bool quote_flag = false;
    bool text_flag = false;
    while(1){
        if ( *buf == 0x0a ||
             (!quote_flag && !text_flag && (*buf == ':' || *buf == ';') ) ) break;
        if ( *buf == '"' ) quote_flag = !quote_flag;

        int n = enc.getBytes(*buf);

        buf += n;
        if (n > 1 && !quote_flag) text_flag = true;

        // TODO: probably remove this
        /*
        if ( IS_TWO_BYTE(*buf) ){
            buf += 2;
            if ( !quote_flag ) text_flag = true;
        }
        else
            buf++;
        */
    }
    if (text_flag && *buf == 0x0a) buf++;
    
    next_script = buf;
}

// string access function
char *ScriptHandler::saveStringBuffer()
{
    strcpy( saved_string_buffer, string_buffer );
    return saved_string_buffer;
}

// script address direct manipulation function
void ScriptHandler::setCurrent(char *pos, bool nowarn)
{
    // warn if directly setting current_script outside the script_buffer
    if (!nowarn &&
        ( (pos < script_buffer) || (pos >= script_buffer + script_buffer_length) ))
        errorAndCont( "setCurrent: outside script bounds", NULL, "Address Issue" );

    current_script = next_script = pos;
}

void ScriptHandler::pushCurrent( char *pos )
{
    // push to use a temporary address for quick buffer parsing
    pushed_current_script = current_script;
    pushed_next_script = next_script;

    setCurrent(pos, true);
}

void ScriptHandler::popCurrent()
{
    current_script = pushed_current_script;
    next_script = pushed_next_script;
}

void ScriptHandler::enterExternalScript(char *pos)
{
    internal_current_script = current_script;
    internal_next_script = next_script;
    setCurrent(pos);
    internal_end_status = end_status;
    internal_current_variable = current_variable;
    internal_pushed_variable = pushed_variable;
}

void ScriptHandler::leaveExternalScript()
{
    setCurrent(internal_current_script);
    internal_current_script = NULL;
    next_script = internal_next_script;
    end_status = internal_end_status;
    current_variable = internal_current_variable;
    pushed_variable = internal_pushed_variable;
}

bool ScriptHandler::isExternalScript()
{
    return (internal_current_script != NULL);
}

int ScriptHandler::getOffset( char *pos )
{
    return pos - script_buffer;
}

char *ScriptHandler::getAddress( int offset )
{
    return script_buffer + offset;
}

int ScriptHandler::getLineByAddress( char *address )
{
    if ( (address < script_buffer) || (address >= script_buffer + script_buffer_length) ) {
        errorAndExit( "getLineByAddress: outside script bounds", NULL, "Address Error" );
        return -1; //dummy
    }
    LabelInfo label = getLabelByAddress( address );

    char *addr = label.label_header;
    int line = 0;
    while ( address > addr && line < label.num_of_lines){
        if ( *addr == 0x0a ) line++;
        addr++;
    }
    return line;
}

char *ScriptHandler::getAddressByLine( int line )
{
    LabelInfo label = getLabelByLine( line );

    int l = line - label.start_line;
    char *addr = label.label_header;
    while ( l > 0 ){
        while( *addr != 0x0a ) addr++;
        addr++;
        l--;
    }
    return addr;
}

ScriptHandler::LabelInfo ScriptHandler::getLabelByAddress( char *address )
{
    int i;
    for ( i=0 ; i<num_of_labels-1 ; i++ ){
        if ( label_info[i+1].start_address > address )
            return label_info[i];
    }
    return label_info[i];
}

ScriptHandler::LabelInfo ScriptHandler::getLabelByLine( int line )
{
    int i;
    for ( i=0 ; i<num_of_labels-1 ; i++ ){
        if ( label_info[i+1].start_line > line )
            return label_info[i];
    }
    if (i == num_of_labels-1) {
        int num_lines = label_info[i].start_line + label_info[i].num_of_lines;
        if (line >= num_lines) {
            snprintf(errbuf, MAX_ERRBUF_LEN,
                     "getLabelByLine: line %d outside script bounds (%d lines)",
                     line, num_lines);
            errorAndExit( errbuf, NULL, "Address Error" );
        }
    }
    return label_info[i];
}

bool ScriptHandler::isName( const char *name )
{
    return (strncmp( name, string_buffer, strlen(name) )==0)?true:false;
}

bool ScriptHandler::isText()
{
    return (current_cmd_type == CMD_TEXT);
}

bool ScriptHandler::isPretext()
{
    return (current_cmd_type == CMD_PRETEXT);
}

bool ScriptHandler::compareString(const char *buf)
{
    SKIP_SPACE(next_script);
    unsigned int i, num = strlen(buf);
    for (i=0 ; i<num ; i++){
        char ch = next_script[i];
        if ('A' <= ch && 'Z' >= ch) ch += 'a' - 'A';
        if (ch != buf[i]) break;
    }
    return (i==num)?true:false;
}

void ScriptHandler::skipLine( int no )
{
    for ( int i=0 ; i<no ; i++ ){
        while ( *current_script != 0x0a ) current_script++;
        current_script++;
    }
    next_script = current_script;
}

// function for kidoku history
bool ScriptHandler::isKidoku()
{
    return skip_enabled;
}

void ScriptHandler::markAsKidoku( char *address )
{
    if (!kidokuskip_flag || internal_current_script != NULL) return;

    int offset = current_script - script_buffer;
    if ( address ) offset = address - script_buffer;
    //printf("mark (%c)%x:%x = %d\n", *current_script, offset /8, offset%8, kidoku_buffer[ offset/8 ] & ((char)1 << (offset % 8)));
    if ( kidoku_buffer[ offset/8 ] & ((char)1 << (offset % 8)) )
        skip_enabled = true;
    else
        skip_enabled = false;
    kidoku_buffer[ offset/8 ] |= ((char)1 << (offset % 8));
}

void ScriptHandler::setKidokuskip( bool kidokuskip_flag )
{
    this->kidokuskip_flag = kidokuskip_flag;
}

void ScriptHandler::saveKidokuData(bool no_error)
{
    FILE *fp;

    if ( ( fp = fopen( "kidoku.dat", "wb", true, true ) ) == NULL ){
        if (!no_error)
            errorAndCont( "can't open kidoku.dat for writing", NULL, "I/O Warning" );
        return;
    }

    if ( fwrite( kidoku_buffer, 1, script_buffer_length/8, fp ) !=
         size_t(script_buffer_length/8) ){
        if (!no_error)
            errorAndCont( "couldn't write to kidoku.dat", NULL, "I/O Warning" );
    }
    fclose( fp );
}

void ScriptHandler::loadKidokuData()
{
    FILE *fp;

    setKidokuskip( true );
    kidoku_buffer = new char[ script_buffer_length/8 + 1 ];
    memset( kidoku_buffer, 0, script_buffer_length/8 + 1 );

    if ( ( fp = fopen( "kidoku.dat", "rb", true, true ) ) != NULL ){
        if (fread( kidoku_buffer, 1, script_buffer_length/8, fp ) !=
            size_t(script_buffer_length/8)) {
            if (ferror(fp))
                errorAndCont( "couldn't read from kidoku.dat", NULL, "I/O Warning" );
        }
        fclose( fp );
    }
}

void ScriptHandler::addIntVariable(char **buf, bool no_zenkaku)
{
    char num_buf[20];
    int no = parseInt(buf);

    int len = getStringFromInteger( num_buf, no, -1, false, !no_zenkaku );
    for (int i=0 ; i<len ; i++)
        addStringBuffer( num_buf[i] );
}

void ScriptHandler::addStrVariable(char **buf)
{
    (*buf)++;
    int no = parseInt(buf);
    VariableData &vd = getVariableData(no);
    if ( vd.str ){
        for (unsigned int i=0 ; i<strlen( vd.str ) ; i++){
            addStringBuffer( vd.str[i] );
        }
    }
}

void ScriptHandler::enableTextgosub(bool val)
{
    textgosub_flag = val;
}

void ScriptHandler::enableRgosub(bool val)
{
    rgosub_flag = val;

    if (rgosub_flag && !rgosub_wait_pos){
        total_rgosub_wait_size = 4;
        rgosub_wait_pos = new char*[total_rgosub_wait_size];
        rgosub_wait_1byte = new bool[total_rgosub_wait_size];
    }
}

void ScriptHandler::setClickstr(const char *list)
{
    if (clickstr_list) delete[] clickstr_list;
    clickstr_list = new char[strlen(list)+2];
    memcpy( clickstr_list, list, strlen(list)+1 );
    clickstr_list[strlen(list)+1] = '\0';
}

int ScriptHandler::checkClickstr(const char *buf, bool recursive_flag)
{
    if ((buf[0] == '\\') && (buf[1] == '@')) return -2;  //clickwait-or-page
    if ((buf[0] == '@') || (buf[0] == '\\')) return -1;

    // Check for clickstr characters
    if (clickstr_list == NULL) return 0;
    bool only_double_byte_check = true;
    char *click_buf = clickstr_list;
    int n;

    while(click_buf[0]){

        if (click_buf[0] == '`'){
            click_buf++;
            only_double_byte_check = false;
            continue;
        }

        int n = enc.getBytes(click_buf[0]);
        int m = enc.getBytes(buf[0]);

        if (! only_double_byte_check){
            if (n == 1 && m == 1 && click_buf[0] == buf[0]) {
                if (!recursive_flag && checkClickstr(buf+1, true) != 0) return 0;
                return 1;
            }
        }

        if (n > 1 && m == n) {
            for (int i=0; i<n; i++) {
                if (click_buf[i] != buf[i]) goto ccs_skip;
            }

            if (!recursive_flag && checkClickstr(buf + n, true) != 0) return 0;
            return 2;
        }
ccs_skip:
        // goto isn't *that* cursed, right? ...right?
        // -Galladite 2023-6-21

        click_buf += n;
    }

    return 0;
}

int ScriptHandler::getIntVariable( VariableInfo *var_info )
{
    if ( var_info == NULL ) var_info = &current_variable;

    if ( var_info->type == VAR_INT )
        return getVariableData(var_info->var_no).num;
    else if ( var_info->type == VAR_ARRAY )
        return *getArrayPtr( var_info->var_no, var_info->array, 0 );
    return 0;
}

void ScriptHandler::readVariable( bool reread_flag )
{
    end_status = END_NONE;
    current_variable.type = VAR_NONE;
    if ( reread_flag ) next_script = current_script;
    current_script = next_script;
    char *buf = current_script;

    SKIP_SPACE(buf);

    bool ptr_flag = false;
    if ( *buf == 'i' || *buf == 's' ){
        ptr_flag = true;
        buf++;
    }

    if ( *buf == '%' ){
        buf++;
        current_variable.var_no = parseInt(&buf);
        current_variable.type = VAR_INT;
    }
    else if ( *buf == '?' ){
        ArrayVariable av;
        current_variable.var_no = parseArray( &buf, av );
        current_variable.type = VAR_ARRAY;
        current_variable.array = av;
    }
    else if ( *buf == '$' ){
        buf++;
        current_variable.var_no = parseInt(&buf);
        current_variable.type = VAR_STR;
    }

    if (ptr_flag) current_variable.type |= VAR_PTR;

    next_script = checkComma(buf);
}

void ScriptHandler::setInt( VariableInfo *var_info, int val, int offset )
{
    if ( var_info->type & VAR_INT ){
        setNumVariable( var_info->var_no + offset, val );
    }
    else if ( var_info->type & VAR_ARRAY ){
        *getArrayPtr( var_info->var_no, var_info->array, offset ) = val;
    }
    else{
        errorAndExit( "setInt: no integer variable." );
    }
}

void ScriptHandler::setStr( char **dst, const char *src, int num )
{
    if ( *dst ) delete[] *dst;
    *dst = NULL;
    
    if ( src ){
        if (num >= 0){
            *dst = new char[ num + 1 ];
            memcpy( *dst, src, num );
            (*dst)[num] = '\0';
        }
        else{
            *dst = new char[ strlen( src ) + 1];
            strcpy( *dst, src );
        }
    }
}

void ScriptHandler::pushVariable()
{
    pushed_variable = current_variable;
}

void ScriptHandler::setNumVariable( int no, int val )
{
    VariableData &vd = getVariableData(no);
    if ( vd.num_limit_flag ){
        if ( val < vd.num_limit_lower )
            val = vd.num_limit_lower;
        else if ( val > vd.num_limit_upper )
            val = vd.num_limit_upper;
    }
    vd.num = val;
}

int ScriptHandler::getStringFromInteger( char *buffer, int no, int num_column,
                                         bool is_zero_inserted,
                                         bool use_zenkaku )
{
    int i, num_space=0, num_minus = 0;
    if (no < 0){
        num_minus = 1;
        no = -no;
    }
    int num_digit=1, no2 = no;
    while(no2 >= 10){
        no2 /= 10;
        num_digit++;
    }

    if (num_column < 0) num_column = num_digit+num_minus;
    if (num_digit+num_minus <= num_column)
        num_space = num_column - (num_digit+num_minus);
    else{
        for (i=0 ; i<num_digit+num_minus-num_column ; i++)
            no /= 10;
        num_digit -= num_digit+num_minus-num_column;
    }

    // Half-width characters
    if (!use_zenkaku) {
        if (num_minus == 1) no = -no;
        char format[6];
        if (is_zero_inserted)
            sprintf(format, "%%0%dd", num_column);
        else
            sprintf(format, "%%%dd", num_column);
        
        sprintf(buffer, format, no);
        return num_column;
    }

    // Full-width characters
    int code = enc.getEncoding();
    int n = 2; // bytes per character
    if (code == Encoding::CODE_UTF8)
        n = 3;
    int c = 0;
    if (is_zero_inserted){
        for (i=0 ; i<num_space ; i++){
            buffer[c++] = ((char*)"‚O")[0];
            buffer[c++] = ((char*)"‚O")[1];
        }
    }
    else{
        for (i=0 ; i<num_space ; i++){
            buffer[c++] = ((char*)"@")[0];
            buffer[c++] = ((char*)"@")[1];
        }
    }
    if (num_minus == 1){
        if (code == Encoding::CODE_CP932){
            // This probably should use the bigger dash, but SJIS
            // doesn't like that. Can this file use UTF-8?
            buffer[c++] = "|"[0];
            buffer[c++] = "|"[1];
        }
        if (code == Encoding::CODE_UTF8){
            buffer[c++] = 0xef;
            buffer[c++] = 0xbc;
            buffer[c++] = 0x8d;
        }
    }
    c = (num_column-1)*n;
    char num_str[] = "‚O‚P‚Q‚R‚S‚T‚U‚V‚W‚X";
    for (i=0 ; i<num_digit ; i++){
        if (code == Encoding::CODE_CP932){
            buffer[c]   = num_str[no % 10 * 2];
            buffer[c+1] = num_str[no % 10 * 2 + 1];
        }
        if (code == Encoding::CODE_UTF8){
            buffer[c]   = 0xef;
            buffer[c+1] = 0xbc;
            buffer[c+2] = 0x90 + no%10;
        }
        no /= 10;
        c -= n;
    }
    buffer[num_column*n] = '\0';

    return num_column*n;
}

int ScriptHandler::readScriptSub( FILE *fp, char **buf, int encrypt_mode )
{
    unsigned char magic[5] = {0x79, 0x57, 0x0d, 0x80, 0x04 };
    int  magic_counter = 0;
    bool newline_flag = true;
    bool cr_flag = false;

    if (encrypt_mode == 3 && !key_table_flag)
        simpleErrorAndExit("readScriptSub: the EXE file must be specified with --key-exe option.");

    size_t len=0, count=0;
    while(1){
        if (len == count){
            len = fread(tmp_script_buf, 1, TMP_SCRIPT_BUF_LEN, fp);
            if (len == 0){
                if (cr_flag) *(*buf)++ = 0x0a;
                break;
            }
            count = 0;
        }
        unsigned char ch = tmp_script_buf[count++];
        if      ( encrypt_mode == 1 ) ch ^= 0x84;
        else if ( encrypt_mode == 2 ){
            ch = (ch ^ magic[magic_counter++]) & 0xff;
            if ( magic_counter == 5 ) magic_counter = 0;
        }
        else if ( encrypt_mode == 3){
            ch = key_table[(unsigned char)ch] ^ 0x84;
        }

        if ( cr_flag && ch != 0x0a ){
            *(*buf)++ = 0x0a;
            newline_flag = true;
            cr_flag = false;
        }

        if ( ch == '*' && newline_flag ) num_of_labels++;
        if ( ch == 0x0d ){
            cr_flag = true;
            continue;
        }
        if ( ch == 0x0a ){
            *(*buf)++ = 0x0a;
            newline_flag = true;
            cr_flag = false;
        }
        else{
            *(*buf)++ = ch;
            if ( ch != ' ' && ch != '\t' )
                newline_flag = false;
        }
    }

    *(*buf)++ = 0x0a;
    return 0;
}

int ScriptHandler::readScript( DirPaths &path )
{
    archive_path = &path;

    FILE *fp = NULL;
    char filename[12];
    char *file_extension = "";
    int i, n=0, encrypt_mode = 0;
    while ((fp == NULL) && (n<archive_path->get_num_paths())) {
        const char *curpath = archive_path->get_path(n);
        const char *filename = "";
        
        // SHIFT-JIS scripts:
        if ((fp = fopen(curpath, "0.txt", "rb")) != NULL){
            encrypt_mode = 0;
            filename = "0.txt";
            file_extension = ".txt";
        }
        else if ((fp = fopen(curpath, "00.txt", "rb")) != NULL){
            encrypt_mode = 0;
            filename = "00.txt";
            file_extension = ".txt";
        }

        // UTF-8 scripts:
        else if ((fp = fopen(curpath, "0.utf", "rb")) != NULL){
            encrypt_mode = 0;
            filename = "0.utf";
            file_extension = ".utf";

            enc.setEncoding(Encoding::CODE_UTF8);
            printf("0.utf detected; entering UTF-8 mode\n");
        }
        else if ((fp = fopen(curpath, "0.utf.txt", "rb")) != NULL){
            encrypt_mode = 0;
            filename = "0.utf.txt";
            file_extension = ".utf.txt";

            enc.setEncoding(Encoding::CODE_UTF8);
            printf("0.utf.txt detected; entering UTF-8 mode\n");
        }
        else if ((fp = fopen(curpath, "00.utf", "rb")) != NULL){
            encrypt_mode = 0;
            filename = "00.utf";
            file_extension = ".utf";

            enc.setEncoding(Encoding::CODE_UTF8);
            printf("00.utf detected; entering UTF-8 mode\n");
        }
        else if ((fp = fopen(curpath, "00.utf.txt", "rb")) != NULL){
            encrypt_mode = 0;
            filename = "00.utf.txt";
            file_extension = ".utf.txt";

            enc.setEncoding(Encoding::CODE_UTF8);
            printf("00.utf.txt detected; entering UTF-8 mode\n");
        }

        // Obfuscated SHIFT-JIS scripts:
        else if ((fp = fopen(curpath, "nscr_sec.dat", "rb")) != NULL){
            encrypt_mode = 2;
            filename = "nscr_sec.dat";
        }
        else if ((fp = fopen(curpath, "nscript.___", "rb")) != NULL){
            encrypt_mode = 3;
            filename = "nscript.___";
        }
        else if ((fp = fopen(curpath, "nscript.dat", "rb")) != NULL){
            encrypt_mode = 1;
            filename = "nscript.dat";
        }

        // Obfuscated UTF-8 scripts:
        else if ((fp = fopen(curpath, "pscript.dat", "rb")) != NULL){
            encrypt_mode = 1;
            filename = "pscript.dat";

            enc.setEncoding(Encoding::CODE_UTF8);
            printf("pscript.dat detected; entering UTF-8 mode\n");
        }

        if (fp != NULL) {
            fprintf(stderr, "Script found: %s%s\n", curpath, filename);
            setStr(&script_path, curpath);
        }
        n++;
    }
    if (fp == NULL){
#if defined(MACOSX) 
        // Why is the mac version like this? Why does it have 4 stirng
        // arguments?? -Galladite 2023-6-15
        /*
        simpleErrorAndExit("No game data found.\nThis application must be run "
                           "from a directory containing ONScripter game data.",
                           "can't open any of 0.txt, 00.txt, 0.utf, 00.utf, 0.utf.txt, 00.utf.txt, nscript.dat, or pscript.dat",
                           "Missing game data");
        */

        // I'll just change it and hope it doesn't break.
        simpleErrorAndExit("No game script found.",
                           "can't open any of 0.txt, 00.txt, 0.utf, 00.utf, 0.utf.txt, 00.utf.txt, nscript.dat, or pscript.dat",
                           "Missing game data");
#else
        simpleErrorAndExit("No game script found.",
                           "can't open any of 0.txt, 00.txt, 0.utf, 00.utf, 0.utf.txt, 00.utf.txt, nscript.dat, or pscript.dat",
                           "Missing game data");
#endif
        return -1;
    }

    fseek( fp, 0, SEEK_END );
    int estimated_buffer_length = ftell( fp ) + 1;

    if (encrypt_mode == 0){
        fclose(fp);
        for (i=1 ; i<100 ; i++){
            sprintf(filename, "%d%s", i, file_extension);
            if ((fp = fopen(script_path, filename, "rb")) == NULL){
                sprintf(filename, "%02d%s", i, file_extension);
                fp = fopen(script_path, filename, "rb");
            }
            if (fp){
                fseek( fp, 0, SEEK_END );
                estimated_buffer_length += ftell(fp)+1;
                fclose(fp);
            }
        }
    }

    if ( script_buffer ) delete[] script_buffer;
    script_buffer = new char[ estimated_buffer_length ];

    char *p_script_buffer;
    current_script = p_script_buffer = script_buffer;

    tmp_script_buf = new char[TMP_SCRIPT_BUF_LEN];
    if (encrypt_mode > 0){
        fseek( fp, 0, SEEK_SET );
        readScriptSub( fp, &p_script_buffer, encrypt_mode );
        fclose( fp );
    }
    else{
        for (i=0 ; i<100 ; i++){
            sprintf(filename, "%d%s", i, file_extension);
            if ((fp = fopen(script_path, filename, "rb")) == NULL){
                sprintf(filename, "%02d%s", i, file_extension);
                fp = fopen(script_path, filename, "rb");
            }
            if (fp){
                readScriptSub( fp, &p_script_buffer, 0 );
                fclose(fp);
            }
        }
    }
    delete[] tmp_script_buf;

    // Haeleth: Search for gameid file (this overrides any builtin
    // ;gameid directive, or serves its purpose if none is available)
    if (!game_identifier) { //Mion: only if gameid not already set
        fp = fopen(script_path, "game.id", "rb"); //Mion: search only the script path
        if (fp) {
            size_t line_size = 0;
            char c;
            do {
                c = fgetc(fp);
                ++line_size;
            } while (c != '\r' && c != '\n' && c != EOF);
            fseek(fp, 0, SEEK_SET);
            game_identifier = new char[line_size];
            if (fgets(game_identifier, line_size, fp) == NULL)
                fputs("Warning: couldn't read game ID from game.id\n", stderr);
            fclose(fp);
        }
    }

    script_buffer_length = p_script_buffer - script_buffer;
    game_hash = script_buffer_length;  // Reasonable "hash" value

    /* ---------------------------------------- */
    /* screen size and value check */
    char *buf = script_buffer;

    // Skip whitespace - some old games put ;gameid on the second
    // line, leaving the first blank (no ;mode given).
    //
    // -Galladite 2024-08-26
    while (*buf == '\n' || *buf == '\r' || *buf == '\t' || *buf == ' ')
        buf++;

    while( script_buffer[0] == ';' ){
        if ( !strncmp( buf, ";mode", 5 ) ){
            buf += 5;
            if ( !strncmp( buf, "800", 3 ) ) {
                screen_width = 800;
                screen_height = 600;
                buf += 3;
            } else if ( !strncmp( buf, "400", 3 ) ) {
                screen_width = 400;
                screen_height = 300;
                buf += 3;
            } else if ( !strncmp( buf, "320", 3 ) ) {
                screen_width = 320;
                screen_height = 240;
                buf += 3;
            } else if (!strncmp( buf, "w720", 4 )) {
                screen_width  = 1280;
                screen_height = 720;
                buf += 4;
            }
            else {
                screen_width = 640;
                screen_height = 480;
            }
        }
        else if ( !strncmp( buf, ";value", 6 ) ){
            buf += 6;
            SKIP_SPACE(buf);
            global_variable_border = 0;
            while ( *buf >= '0' && *buf <= '9' )
                global_variable_border = global_variable_border * 10 + *buf++ - '0';
            //printf("set global_variable_border: %d\n", global_variable_border);
        }
        else if (!(strncmp( buf, ";$", 2 ))) {
            buf += 2;
            while ( *buf != '\n' ) {
                if (*buf == 'g' || *buf == 'G') {
                    buf++;
                    SKIP_SPACE(buf);
                    global_variable_border = 0;
                    while ( *buf >= '0' && *buf <= '9' )
                        global_variable_border = global_variable_border*10 + *buf++ - '0';
                }
                else if (*buf == 's' || *buf == 'S') {
                    buf++;

                    if (*buf == '?') {
                        // Signal to other code that we are using a
                        // variable resolution. Handled later.
                        screen_width = -1;
                        screen_height = -1;
                        buf++;

                        goto after_cres;
                    }

                    if (!(*buf >= '0' && *buf <= '9')) break;
                    screen_width = 0;
                    while (*buf >= '0' && *buf <= '9')
                        screen_width = screen_width*10 + *buf++ - '0';
                    while (*buf == ',' || *buf == ' ' || *buf == '\t') buf++;
                    screen_height = 0;
                    while (*buf >= '0' && *buf <= '9')
                        screen_height = screen_height*10 + *buf++ - '0';
after_cres:
                    ;
                }
                else if (*buf == 'l' || *buf == 'L') { // This command was unimplemented in ONScripter
                    buf++;
                    SKIP_SPACE(buf);
                    while (*buf >= '0' && *buf <= '9') buf++;
                }
                else if (*buf == 'v' || *buf == 'V') { // This command would require major, unnecessary work to get implemented; since ONScripter-en already has unbounded variables, the logical option is just to fix the bug preventing *global* variables over 4095.
                    buf++;
                    SKIP_SPACE(buf);
                    while (*buf >= '0' && *buf <= '9') buf++;
                }
                else {
                    buf++;
                }
            }
        }
        else if ( !strncmp( buf, ";gameid", 7 ) && !game_identifier ){
            buf += 8; // Also skip the space (simple way)
            int i = 0;
            while ( buf[i++] != '\n' );
            game_identifier = new char[i];
            strncpy( game_identifier, buf, i - 1 );
            game_identifier[i - 1] = 0;
            buf += i;
        }
        else{
            break;
        }
        if ( *buf != ',' ){
        	while ( *buf++ != '\n' );
        	break;
        }
        buf++;
    }


    return labelScript();
}

int ScriptHandler::labelScript()
{
    int label_counter = -1;
    int current_line = 0;
    char *buf = script_buffer;
    label_info = new LabelInfo[ num_of_labels+1 ];

    while ( buf < script_buffer + script_buffer_length ){
        SKIP_SPACE( buf );
        if ( *buf == '*' ){
            setCurrent( buf );
            readLabel();
            label_info[ ++label_counter ].name = new char[ strlen(string_buffer) ];
            strcpy( label_info[ label_counter ].name, string_buffer+1 );
            label_info[ label_counter ].label_header = buf;
            label_info[ label_counter ].num_of_lines = 1;
            label_info[ label_counter ].start_line   = current_line;
            buf = getNext();
            if ( *buf == 0x0a ){
                buf++;
                SKIP_SPACE(buf);
                current_line++;
            }
            label_info[ label_counter ].start_address = buf;
        }
        else{
            if ( label_counter >= 0 )
                label_info[ label_counter ].num_of_lines++;
            while( *buf != 0x0a ) buf++;
            buf++;
            current_line++;
        }
    }

    label_info[num_of_labels].start_address = NULL;

    return 0;
}

struct ScriptHandler::LabelInfo ScriptHandler::lookupLabel( const char *label )
{
    int i = findLabel( label );

    if (i == -1) {
        snprintf(errbuf, MAX_ERRBUF_LEN, "Label \"*%s\" not found.", label);
        errorAndExit( errbuf, NULL, "Label Error" );
    }

    findAndAddLog( log_info[LABEL_LOG], label_info[i].name, true );
    return label_info[i];
}

struct ScriptHandler::LabelInfo ScriptHandler::lookupLabelNext( const char *label )
{
    int i = findLabel( label );

    if (i == -1) {
        snprintf(errbuf, MAX_ERRBUF_LEN, "Label \"*%s\" not found.", label);
        errorAndExit( errbuf, NULL, "Label Error" );
    }

    if ( i+1 < num_of_labels ){
        findAndAddLog( log_info[LABEL_LOG], label_info[i+1].name, true );
        return label_info[i+1];
    }

    return label_info[num_of_labels];
}

bool ScriptHandler::hasLabel( const char *label )
{
    return (findLabel( label ) != -1);
}

ScriptHandler::LogLink *ScriptHandler::findAndAddLog( LogInfo &info, const char *name, bool add_flag )
{
    char capital_name[256];
    for ( unsigned int i=0 ; i<strlen(name)+1 ; i++ ){
        capital_name[i] = name[i];
        if ( 'a' <= capital_name[i] && capital_name[i] <= 'z' ) capital_name[i] += 'A' - 'a';
        else if ( capital_name[i] == '/' ) capital_name[i] = '\\';
    }

    LogLink *cur = info.root_log.next;
    while( cur ){
        if ( !strcmp( cur->name, capital_name ) ) break;
        cur = cur->next;
    }
    if ( !add_flag || cur ) return cur;

    LogLink *link = new LogLink();
    link->name = new char[strlen(capital_name)+1];
    strcpy( link->name, capital_name );
    info.current_log->next = link;
    info.current_log = info.current_log->next;
    info.num_logs++;

    return link;
}

void ScriptHandler::resetLog( LogInfo &info )
{
    LogLink *link = info.root_log.next;
    while( link ){
        LogLink *tmp = link;
        link = link->next;
        delete tmp;
    }

    info.root_log.next = NULL;
    info.current_log = &info.root_log;
    info.num_logs = 0;
}

ScriptHandler::ArrayVariable *ScriptHandler::getRootArrayVariable(){
    return root_array_variable;
}

void ScriptHandler::addNumAlias( const char *str, int no )
{
    Alias *p_num_alias = new Alias( str, no );
    last_num_alias->next = p_num_alias;
    last_num_alias = last_num_alias->next;
}

void ScriptHandler::addStrAlias( const char *str1, const char *str2 )
{
    Alias *p_str_alias = new Alias( str1, str2 );
    last_str_alias->next = p_str_alias;
    last_str_alias = last_str_alias->next;
}

bool ScriptHandler::findNumAlias( const char *str, int *value )
{
    Alias *p_num_alias = root_num_alias.next;
    while( p_num_alias ){
	if ( !strcmp( p_num_alias->alias, str ) ){
	    *value = p_num_alias->num;
	    return true;
	}
	p_num_alias = p_num_alias->next;
    }
    return false;
}

bool ScriptHandler::findStrAlias( const char *str, char* buffer )
{
    Alias *p_str_alias = root_str_alias.next;
    while( p_str_alias ){
	if ( !strcmp( p_str_alias->alias, str ) ){
	    strcpy( buffer, p_str_alias->str );
	    return true;
	}
	p_str_alias = p_str_alias->next;
    }
    return false;
}

void ScriptHandler::processError( const char *str, const char *title, const char *detail, bool is_warning, bool is_simple )
{
    //if not yet running the script, no line no/cmd - keep it simple
    if (script_buffer == NULL)
        is_simple = true;

    if (title == NULL)
        title = "Error";
    const char *type = is_warning ? "Warning" : "Fatal";

    if (is_simple) {
        fprintf(stderr, " ***[%s] %s: %s ***\n", type, title, str);
        if (detail)
            fprintf(stderr, "\t%s\n", detail);

        if (is_warning && !strict_warnings) return;

        if (!ons) exit(-1); //nothing left to do without doErrorBox

        if (! ons->doErrorBox(title, str, true, is_warning))
            return;

        if (is_warning)
            fprintf(stderr, " ***[Fatal] User terminated at warning ***\n");

    } else {

        char errhist[1024], errcmd[128];

        char *cur = getCurrent();
        LabelInfo label = getLabelByAddress(cur);
        int lblinenum = -1, linenum = -1;
        if ((cur >= script_buffer) && (cur < script_buffer + script_buffer_length)) {
            lblinenum = getLineByAddress(getCurrent());
            linenum = label.start_line + lblinenum + 1;
        }

        errcmd[0] = '\0';
        if (strlen(current_cmd) > 0) {
            if (current_cmd_type == CMD_BUILTIN)
                snprintf(errcmd, 128, ", cmd \"%s\"", current_cmd);
            else if (current_cmd_type == CMD_USERDEF)
                snprintf(errcmd, 128, ", user-defined cmd \"%s\"", current_cmd);
        }
        if (linenum < 0) {
            fprintf(stderr, " ***[%s] %s at line ?? (*%s:)%s - %s ***\n",
                    type, title, label.name, errcmd, str);
        } else {
            fprintf(stderr, " ***[%s] %s at line %d (*%s:%d)%s - %s ***\n",
                    type, title, linenum, label.name, lblinenum, errcmd, str);
        }
        if (detail)
            fprintf(stderr, "\t%s\n", detail);
        if (string_buffer && *string_buffer)
            fprintf(stderr, "\t(String buffer: [%s])\n", string_buffer);

        if (is_warning && !strict_warnings) return;

        if (!ons) exit(-1); //nothing left to do without doErrorBox

        if (is_warning) {
            if (linenum < 0) {
                snprintf(errhist, 1024, "%s\nat line ?? (*%s:)%s\n%s",
                         str, label.name, errcmd,
                         detail ? detail : "");
            } else {
                snprintf(errhist, 1024, "%s\nat line %d (*%s:%d)%s\n%s",
                         str, linenum, label.name, lblinenum, errcmd,
                         detail ? detail : "");
            }

            if(!ons->doErrorBox(title, errhist, false, true))
                return;

            fprintf(stderr, " ***[Fatal] User terminated at warning ***\n");
        }

        //Mion: grabbing the current line in the script & up to 2 previous ones,
        // in-place (replaces the newlines with '\0', and then puts the newlines
        // back when finished)
        int i;
        char *line[3], *tmp[4];
        for (i=0; i<4; i++) tmp[i] = NULL;
        char *end = getCurrent();
        while (*end && *end != 0x0a) end++;
        if (*end) tmp[3] = end;
        *end = '\0';
        char *buf = getCurrent();
        for (i=2; i>=0; i--) {
            if (linenum + i - 3 > 0) {
                while (*buf != 0x0a) buf--;
                tmp[i] = buf;
                *buf = '\0';
                line[i] = buf+1;
            } else if (linenum + i - 3 == 0) {
                line[i] = script_buffer;
            } else
                line[i] = end;
        }

        snprintf(errhist, 1024, "%s\nat line %d (*%s:%d)%s\n\n| %s\n| %s\n> %s",
                 str, linenum, label.name, lblinenum, errcmd,
                 line[0], line[1], line[2]);

        for (i=0; i<4; i++) {
            if (tmp[i]) *(tmp[i]) = 0x0a;
        }

        if (! ons->doErrorBox(title, errhist, false, false))
            return;

    }

    exit(-1);
}

void ScriptHandler::errorAndExit( const char *str, const char *detail, const char *title, bool is_warning )
{
    if (title == NULL)
        title = "Script Error";

    processError(str, title, detail, is_warning);
}

void ScriptHandler::errorAndCont( const char *str, const char *detail, const char *title )
{
    if (title == NULL)
        title = "Script Warning";

    processError(str, title, detail, true);
}

// TODO - change this for clarity; there is no reason for it to be how
// it is (with title before detail). -Galladite 2023-6-15
void ScriptHandler::simpleErrorAndExit( const char *str, const char *title, const char *detail, bool is_warning )
{
    if (title == NULL)
        title = "Script Error";

    processError(str, detail, title, is_warning, true);
}

void ScriptHandler::addStringBuffer( char ch )
{
    if (string_counter+1 == STRING_BUFFER_LENGTH)
        errorAndExit("addStringBuffer: string length exceeds 2048.");
    string_buffer[string_counter++] = ch;
    string_buffer[string_counter] = '\0';
}

void ScriptHandler::trimStringBuffer( unsigned int n )
{
    string_counter -= n;
    if (string_counter < 0)
        string_counter = 0;
    string_buffer[string_counter] = '\0';
}

void ScriptHandler::pushStringBuffer(int offset)
{
    strcpy(gosub_string_buffer, string_buffer);
    gosub_string_offset = offset;
    gosub_cmd_type = current_cmd_type;
}

int ScriptHandler::popStringBuffer()
{
    strcpy(string_buffer, gosub_string_buffer);
    current_cmd_type = gosub_cmd_type;
    return gosub_string_offset;
}

ScriptHandler::VariableData &ScriptHandler::getVariableData(int no)
{
    if (no >= 0 && no < VARIABLE_RANGE)
        return variable_data[no];

    for (int i=0 ; i<num_extended_variable_data ; i++)
        if (extended_variable_data[i].no == no) 
            return extended_variable_data[i].vd;
        
    num_extended_variable_data++;
    if (num_extended_variable_data == max_extended_variable_data){
        ExtendedVariableData *tmp = extended_variable_data;
        extended_variable_data = new ExtendedVariableData[max_extended_variable_data*2];
        if (tmp){
            memcpy(extended_variable_data, tmp, sizeof(ExtendedVariableData)*max_extended_variable_data);
            delete[] tmp;
        }
        max_extended_variable_data *= 2;
    }

    extended_variable_data[num_extended_variable_data-1].no = no;

    return extended_variable_data[num_extended_variable_data-1].vd;
}

// ----------------------------------------
// Private methods

int ScriptHandler::findLabel( const char *label )
{
    int i;
    char capital_label[256];
    int len = strlen(label) + 1;
    if (len > 256) len = 256; //prevent overrun

    for ( i=0 ; i<len ; i++ ){
        capital_label[i] = label[i];
        if ( 'A' <= capital_label[i] && capital_label[i] <= 'Z' ) capital_label[i] += 'a' - 'A';
    }
    for ( i=0 ; i<num_of_labels ; i++ ){
        if ( !strcmp( label_info[i].name, capital_label ) )
            return i;
    }

    return -1;
}

char *ScriptHandler::checkComma( char *buf )
{
    SKIP_SPACE( buf );
    if (*buf == ','){
        end_status |= END_COMMA;
        buf++;
        SKIP_SPACE( buf );
    }

    return buf;
}

void ScriptHandler::parseStr( char **buf )
{
    SKIP_SPACE( *buf );

    if ( **buf == '(' ){
        // (foo) bar baz : apparently returns bar if foo has been
        // viewed, baz otherwise.
        // (Rather like a trigram implicitly using "fchk")

        (*buf)++;
        parseStr(buf);
        SKIP_SPACE( *buf );
        if ( (*buf)[0] != ')' ) errorAndExit("parseStr: missing ')'.");
        (*buf)++;

        if ( findAndAddLog( log_info[FILE_LOG], str_string_buffer, false ) ){
            parseStr(buf);
            char *tmp_buf = new char[ strlen( str_string_buffer ) + 1 ];
            strcpy( tmp_buf, str_string_buffer );
            parseStr(buf);
            strcpy( str_string_buffer, tmp_buf );
            delete[] tmp_buf;
        }
        else{
            parseStr(buf);
            parseStr(buf);
        }
        current_variable.type |= VAR_CONST;
    }
    else if ( **buf == '$' ){
        (*buf)++;
        int no = parseInt(buf);
        VariableData &vd = getVariableData(no);

        if ( vd.str )
            strcpy( str_string_buffer, vd.str );
        else
            str_string_buffer[0] = '\0';
        current_variable.type = VAR_STR;
        current_variable.var_no = no;
    }
    else if ( **buf == '"' ){
        int c=0;
        (*buf)++;
        while ( **buf != '"' && **buf != 0x0a )
            str_string_buffer[c++] = *(*buf)++;
        str_string_buffer[c] = '\0';
        if ( **buf == '"' ) (*buf)++;
        current_variable.type |= VAR_CONST;
    }
    else if ( **buf == '`' ){
        int c=0;
        str_string_buffer[c++] = *(*buf)++;
        while ( **buf != '`' && **buf != 0x0a )
            str_string_buffer[c++] = *(*buf)++;
        str_string_buffer[c] = '\0';
        if ( **buf == '`' ) (*buf)++;
        current_variable.type |= VAR_CONST;
        end_status |= END_1BYTE_CHAR;
    }
    else if ( **buf == '#' ){ // for color
        for ( int i=0 ; i<7 ; i++ )
            str_string_buffer[i] = *(*buf)++;
        str_string_buffer[7] = '\0';
        current_variable.type = VAR_NONE;
    }
    else if ( **buf == '*' ){ // label
        int c=0;
        str_string_buffer[c++] = *(*buf)++;
        SKIP_SPACE(*buf);
        char ch = **buf;
        while((ch >= 'a' && ch <= 'z') || 
              (ch >= 'A' && ch <= 'Z') ||
              (ch >= '0' && ch <= '9') ||
              ch == '_'){
            if (ch >= 'A' && ch <= 'Z') ch += 'a' - 'A';
            str_string_buffer[c++] = ch;
            ch = *++(*buf);
        }
        str_string_buffer[c] = '\0';
        current_variable.type |= VAR_CONST;
    }
    else{ // str alias
        const char* fmt = "Undefined string alias '%s'";
        char ch, alias_buf[MAX_ERRBUF_LEN - (strlen(fmt) - 2)]; // minus 2 accounts for the %s format specifier
        unsigned int alias_buf_len = 0;
        bool first_flag = true;

        while(1){
            if ( alias_buf_len == sizeof(alias_buf) - 1 ) break;
            ch = **buf;

            if ( (ch >= 'a' && ch <= 'z') ||
                 (ch >= 'A' && ch <= 'Z') ||
                 ch == '_' ){
                if (ch >= 'A' && ch <= 'Z') ch += 'a' - 'A';
                first_flag = false;
                alias_buf[ alias_buf_len++ ] = ch;
            }
            else if ( ch >= '0' && ch <= '9' ){
                if ( first_flag )
                    errorAndExit("parseStr: string alias cannot start with a digit.");
                first_flag = false;
                alias_buf[ alias_buf_len++ ] = ch;
            }
            else break;
            (*buf)++;
        }
        alias_buf[alias_buf_len] = '\0';

        if ( alias_buf_len == 0 ){
            str_string_buffer[0] = '\0';
            current_variable.type = VAR_NONE;
            return;
        }

        if (!findStrAlias( (const char*)alias_buf, str_string_buffer )) {
            snprintf(errbuf, MAX_ERRBUF_LEN, fmt, alias_buf);
            errorAndExit(errbuf);
        }
        current_variable.type |= VAR_CONST;
    }
}

int ScriptHandler::parseInt( char **buf )
{
    int ret = 0;

    SKIP_SPACE( *buf );

    if ( **buf == '%' ){
        (*buf)++;
        current_variable.var_no = parseInt(buf);
        current_variable.type = VAR_INT;
        return getVariableData(current_variable.var_no).num;
    }
    else if ( **buf == '?' ){
        ArrayVariable av;
        current_variable.var_no = parseArray( buf, av );
        current_variable.type = VAR_ARRAY;
        current_variable.array = av;
        return *getArrayPtr( current_variable.var_no, current_variable.array, 0 );
    }
    else{
        char ch, alias_buf[256];
        int alias_buf_len = 0, alias_no = 0;
        bool direct_num_flag = false;
        bool num_alias_flag = false;

        char *buf_start = *buf;
        while( 1 ){
            ch = **buf;

            if ( (ch >= 'a' && ch <= 'z') ||
                 (ch >= 'A' && ch <= 'Z') ||
                 ch == '_' ){
                if (ch >= 'A' && ch <= 'Z') ch += 'a' - 'A';
                if ( direct_num_flag ) break;
                num_alias_flag = true;
                alias_buf[ alias_buf_len++ ] = ch;
            }
            else if ( ch >= '0' && ch <= '9' ){
                if ( !num_alias_flag ) direct_num_flag = true;
                if ( direct_num_flag )
                    alias_no = alias_no * 10 + ch - '0';
                else
                    alias_buf[ alias_buf_len++ ] = ch;
            }
            else break;
            (*buf)++;
        }

        if ( *buf - buf_start  == 0 ){
            current_variable.type = VAR_NONE;
            return 0;
        }

        /* ---------------------------------------- */
        /* Solve num aliases */
        if ( num_alias_flag ){
            alias_buf[ alias_buf_len ] = '\0';

	    if ( !findNumAlias( (const char*) alias_buf, &alias_no ) ) {
                //printf("can't find num alias for %s... assume 0.\n", alias_buf );
                current_variable.type = VAR_NONE;
                *buf = buf_start;
                return 0;
	    }
	}
        current_variable.type = VAR_INT | VAR_CONST;
        ret = alias_no;
    }

    SKIP_SPACE( *buf );

    return ret;
}

int ScriptHandler::parseIntExpression( char **buf )
{
    int num[3], op[2]; // internal buffer

    SKIP_SPACE( *buf );

    readNextOp( buf, NULL, &num[0] );

    readNextOp( buf, &op[0], &num[1] );
    if ( op[0] == OP_INVALID )
        return num[0];

    while(1){
        readNextOp( buf, &op[1], &num[2] );
        if ( op[1] == OP_INVALID ) break;

        if ( !(op[0] & 0x04) && (op[1] & 0x04) ){ // if priority of op[1] is higher than op[0]
            num[1] = calcArithmetic( num[1], op[1], num[2] );
        }
        else{
            num[0] = calcArithmetic( num[0], op[0], num[1] );
            op[0] = op[1];
            num[1] = num[2];
        }
    }
    return calcArithmetic( num[0], op[0], num[1] );
}

/*
 * Internal buffer looks like this.
 *   num[0] op[0] num[1] op[1] num[2]
 * If priority of op[0] is higher than op[1], (num[0] op[0] num[1]) is computed,
 * otherwise (num[1] op[1] num[2]) is computed.
 * Then, the next op and num is read from the script.
 * Num is an immediate value, a variable or a bracketed expression.
 */
void ScriptHandler::readNextOp( char **buf, int *op, int *num )
{
    bool minus_flag = false;
    SKIP_SPACE(*buf);
    char *buf_start = *buf;

    if ( op ){
        if      ( (*buf)[0] == '+' ) *op = OP_PLUS;
        else if ( (*buf)[0] == '-' ) *op = OP_MINUS;
        else if ( (*buf)[0] == '*' ) *op = OP_MULT;
        else if ( (*buf)[0] == '/' ) *op = OP_DIV;
        else if ( (*buf)[0] == 'm' &&
                  (*buf)[1] == 'o' &&
                  (*buf)[2] == 'd' &&
                  ( (*buf)[3] == ' '  ||
                    (*buf)[3] == '\t' ||
                    (*buf)[3] == '$' ||
                    (*buf)[3] == '%' ||
                    (*buf)[3] == '?' ||
                    ( (*buf)[3] >= '0' && (*buf)[3] <= '9') ))
            *op = OP_MOD;
        else{
            *op = OP_INVALID;
            return;
        }
        if ( *op == OP_MOD ) *buf += 3;
        else                 (*buf)++;
        SKIP_SPACE(*buf);
    }
    else{
        if ( (*buf)[0] == '-' ){
            minus_flag = true;
            (*buf)++;
            SKIP_SPACE(*buf);
        }
    }

    if ( (*buf)[0] == '(' ){
        (*buf)++;
        *num = parseIntExpression( buf );
        if (minus_flag) *num = -*num;
        SKIP_SPACE(*buf);
        if ( (*buf)[0] != ')' ) errorAndExit("Missing ')' in expression");
        (*buf)++;
    }
    else{
        *num = parseInt( buf );
        if (minus_flag) *num = -*num;
        if ( current_variable.type == VAR_NONE ){
            if (op) *op = OP_INVALID;
            *buf = buf_start;
        }
    }
}

int ScriptHandler::calcArithmetic( int num1, int op, int num2 )
{
    int ret=0;

    if      ( op == OP_PLUS )  ret = num1+num2;
    else if ( op == OP_MINUS ) ret = num1-num2;
    else if ( op == OP_MULT )  ret = num1*num2;
    else if ( op == OP_DIV )   ret = num1/num2;
    else if ( op == OP_MOD )   ret = num1%num2;

    current_variable.type = VAR_INT | VAR_CONST;

    return ret;
}

int ScriptHandler::parseArray( char **buf, struct ArrayVariable &array )
{
    SKIP_SPACE( *buf );

    (*buf)++; // skip '?'
    int no = parseInt( buf );

    SKIP_SPACE( *buf );
    array.num_dim = 0;
    while ( **buf == '[' ){
        (*buf)++;
        array.dim[array.num_dim] = parseIntExpression(buf);
        array.num_dim++;
        SKIP_SPACE( *buf );
        if ( **buf != ']' ) errorAndExit( "parseArray: missing ']'." );
        (*buf)++;
    }
    for ( int i=array.num_dim ; i<20 ; i++ ) array.dim[i] = 0;

    return no;
}

int *ScriptHandler::getArrayPtr( int no, ArrayVariable &array, int offset )
{
    ArrayVariable *av = root_array_variable;
    while(av){
        if (av->no == no) break;
        av = av->next;
    }
    if (av == NULL) {
        snprintf(errbuf, MAX_ERRBUF_LEN, "Undeclared array number %d", no);
        errorAndExit( errbuf, NULL, "Access Error" );
    }

    int dim = 0, i;
    for ( i=0 ; i<av->num_dim ; i++ ){
        if ( av->dim[i] <= array.dim[i] )
            errorAndExit( "Array access out of bounds", "dim[i] <= array.dim[i]", "Access Error" );
        dim = dim * av->dim[i] + array.dim[i];
    }
    if ( av->dim[i-1] <= array.dim[i-1] + offset )
        errorAndExit( "Array access out of bounds", "dim[i-1] <= array.dim[i-1] + offset", "Access Error" );

    return &av->data[dim+offset];
}

void ScriptHandler::declareDim()
{
    current_script = next_script;
    char *buf = current_script;

    if (current_array_variable){
        current_array_variable->next = new ArrayVariable();
        current_array_variable = current_array_variable->next;
    }
    else{
        root_array_variable = new ArrayVariable();
        current_array_variable = root_array_variable;
    }

    ArrayVariable array;
    current_array_variable->no = parseArray( &buf, array );

    int dim = 1;
    current_array_variable->num_dim = array.num_dim;
    for ( int i=0 ; i<array.num_dim ; i++ ){
        current_array_variable->dim[i] = array.dim[i]+1;
        dim *= (array.dim[i]+1);
    }
    current_array_variable->data = new int[dim];
    memset( current_array_variable->data, 0, sizeof(int) * dim );

    next_script = buf;
}
