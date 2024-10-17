/* -*- C++ -*-
 * 
 *  onscripter.cpp -- main function of ONScripter-EN
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

#include "ONScripterLabel.h"
#include "version.h"

#include "SDL.h"
#include "SDL_main.h"

#include <cstdio>

#define CFG_FILE "ons.cfg"

static void optionHelp()
{
    printf( "Usage: onscripter [option ...]\n" );
    printf( "      --cdaudio\t\tuse CD audio if available\n");
    printf( "      --cdnumber no\tchoose the CD-ROM drive number\n");
    printf( "      --match-audiodevice-to-bgm\treset audio to match bgm specs\n");
    printf( "      --nomatch-audiodevice-to-bgm\tdon't reset audio to match bgm specs (default)\n");
    printf( "  -f, --font file\tset a TTF, OTF, TTC, or OTC font file\n");
    printf( "      --registry file\tset a registry file\n");
    printf( "      --english\t\tset preferred text mode to English (default)\n");
    printf( "      --japanese\tset preferred text mode to Japanese\n");
    printf( "      --english-menu\tuse English system menu messages (default)\n");
    printf( "      --japanese-menu\tuse Japanese system menu messages\n");
#if   defined WIN32
    printf( "      --waveout-audio\tuse the Windows waveout audio driver (instead of Direct Sound)\n");
    printf( "      --dll file\tset a dll file\n");
    printf( "  -r, --root path\tset the root path to the archives\n");
    printf( "  -s, --save path\tset the path to use for saved games (default: folder in All Users profile)\n");
    printf( "      --current-user-appdata\tuse the current user's AppData folder instead of AllUsers' AppData\n");
#elif defined MACOSX
    printf( "  -r, --root path\tset the root path to the archives (default: Resources in ONScripter bundle)\n");
    printf( "  -s, --save path\tset the path to use for saved games (default: folder in ~/Library/Application Support)\n");
#elif defined LINUX
    printf( "  -r, --root path\tset the root path to the archives\n");
    printf( "  -s, --save path\tset the path to use for saved games (default: hidden subdirectory in ~)\n");
#else
    printf( "  -r, --root path\tset the root path to the archives\n");
    printf( "  -s, --save path\tset the path to use for saved games (default: same as root path)\n");
#endif
    printf( "      --use-app-icons\tuse the icns for the current application, if bundled/embedded\n");
    printf( "      --gameid id\t\tset game identifier (like with game.id)\n");
#ifndef NO_LAYER_EFFECTS
    printf( "      --no-layers\tignore layer-based cmds\n");
#endif
    printf( "      --fullscreen\tstart in fullscreen mode\n");
    printf( "      --window\t\tstart in window mode\n");
#ifndef PDA
    printf( "      --window-width width\t\tset preferred window width\n");
#endif
#ifdef RCA_SCALE
    printf( "      --widescreen\ttransform game to match widescreen monitors\n");
#endif
    printf( "      --scale\t\tscale game to native display size. Yields small sharp text.\n");
    printf( "      --no-movie-upscale\t\tdon't resize a movie larger than its native size.\n");
    printf( "      --force-png-alpha\t\talways use PNG alpha channels\n");
    printf( "      --force-png-nscmask\talways use NScripter-style masks\n");
    printf( "      --detect-png-nscmask\tdetect PNG alpha images that actually use masks\n");
    printf( "      --force-button-shortcut\tignore useescspc and getenter command\n");
#ifdef USE_X86_GFX
    printf( "      --disable-cpu-gfx\tdo not use MMX/SSE2 graphics acceleration routines\n");
#elif  USE_PPC_GFX
    printf( "      --disable-cpu-gfx\tdo not use Altivec graphics acceleration routines\n");
#endif
    printf( "      --automode-time time\tdefault time at clickwaits before continuing, when in automode\n");
    printf( "      --enable-wheeldown-advance\tadvance the text on mouse wheeldown event\n");
    printf( "      --disable-rescale\tdo not rescale the images in the archives when compiled with -DPDA\n");
    printf( "      --edit\t\tenable editing the volumes and the variables when 'z' is pressed\n");
    printf( "      --fileversion\tset the ONS file version for loading unversioned files\n");
    printf( "      --key-exe file\tset a file (*.EXE) that includes a key table\n");
    printf( "      --nsa-offset offset\tuse byte offset x when reading arc*.nsa files\n");
    printf( "      --allow-color-type-only\tsyntax option for only recognizing color type for color arguments\n");
    printf( "      --set-tag-page-origin-to-1\tsyntax option for setting 'gettaglog' origin to 1 instead of 0\n");
    printf( "      --answer-dialog-with-yes-ok\thave 'yesnobox' and 'okcancelbox' give 'yes/ok' result\n");
    printf( "      --ignore-textgosub-newline\tignore newline after a clickwait when in textgosub mode\n");
    printf( "      --skip-past-newline\twhen doing a 'click to skip', don't leave that skip mode at newlines\n");
    printf( "      --audiodriver dev\tset the SDL_AUDIODRIVER to dev\n");
    printf( "      --audiobuffer size\tset the audio buffer size in kB (default: 2)\n");
    printf( "      --strict\t\ttreat warnings more like errors\n");
    printf( "      --debug\t\tgenerate runtime debugging output (use multiple times to increase debug level)\n");
    printf( "  -h, --help\t\tshow this help and exit\n");
    printf( "  -v, --version\t\tshow the version information and exit\n");
    exit(0);
}

static void optionVersion()
{
#ifdef ONS_CODENAME
    printf("ONScripter-EN version %s '%s' (%d.%02d)\n", ONS_VERSION, ONS_CODENAME, NSC_VERSION/100, NSC_VERSION%100 );
#else
    printf("ONScripter-EN version %s (%d.%02d)\n", ONS_VERSION, NSC_VERSION/100, NSC_VERSION%100 );
#endif
    printf("Original written by Ogapee <ogapee@aqua.dti2.ne.jp>,\n");
    printf("English fork originally maintained by Seung Park <@lightbinder on Github>\n");
    printf("Previously maintained by \"Uncle\" Mion Sonozaki <UncleMion@gmail.com>\n");
    printf("Currently maintained by Galladite <galladite@yandex.com>\n\n");
    printf("Copyright (c) 2001-2011 Ogapee, 2007-2011 Sonozaki, 2023 Galladite\n");
    printf("This is free software; see the source for copying conditions.\n");
    exit(0);
}

static void parseOptions(int argc, char **argv, ONScripterLabel &ons, bool &hasArchivePath)
{
    argv++;
    while( argc > 1 ){
        if ( argv[0][0] == '-' ){
            if ( !strcmp( argv[0]+1, "h" ) || !strcmp( argv[0]+1, "-help" ) ){
                optionHelp();
            }
            else if ( !strcmp( argv[0]+1, "v" ) || !strcmp( argv[0]+1, "-version" ) ){
                optionVersion();
            }
            else if ( !strcmp( argv[0]+1, "-cdaudio" ) ){
                ons.enableCDAudio();
            }
            else if ( !strcmp( argv[0]+1, "-cdnumber" ) ){
                argc--;
                argv++;
                ons.setCDNumber(atoi(argv[0]));
            }
#ifdef WIN32
            else if ( !strcmp( argv[0]+1, "-waveout-audio" ) ){
                ons.setAudiodriver("waveout");
            }
            else if ( !strcmp( argv[0]+1, "-current-user-appdata" ) ){
                ons.setUserAppData();
            }
#endif
            else if ( !strcmp( argv[0]+1, "-audiodriver" ) ){
                argc--;
                argv++;
                ons.setAudiodriver(argv[0]);
            }
            else if ( !strcmp( argv[0]+1, "-audiobuffer" ) ){
                argc--;
                argv++;
                ons.setAudioBufferSize(atoi(argv[0]));
            }
            else if ( !strcmp( argv[0]+1, "-match-audiodevice-to-bgm" ) ){
                ons.setMatchBgmAudio(true);
            }
            else if ( !strcmp( argv[0]+1, "-nomatch-audiodevice-to-bgm" ) ){
                ons.setMatchBgmAudio(false);
            }
            else if ( !strcmp( argv[0]+1, "f" ) || !strcmp( argv[0]+1, "-font" ) ){
                argc--;
                argv++;
                ons.setFontFile(argv[0]);
            }
            else if ( !strcmp( argv[0]+1, "-registry" ) ){
                argc--;
                argv++;
                ons.setRegistryFile(argv[0]);
            }
            else if ( !strcmp( argv[0]+1, "-dll" ) ){
                argc--;
                argv++;
                ons.setDLLFile(argv[0]);
            }
            else if ( !strcmp( argv[0]+1, "-english" ) ){
                ons.setEnglishPreferred();
            }
            else if ( !strcmp( argv[0]+1, "-japanese" ) ){
                ons.setJapanesePreferred();
            }
            else if ( !strcmp( argv[0]+1, "-english-menu" ) ){
                ons.setEnglishMenu();
            }
            else if ( !strcmp( argv[0]+1, "-japanese-menu" ) ){
                ons.setJapaneseMenu();
            }
            else if ( !strcmp( argv[0]+1, "r" ) || !strcmp( argv[0]+1, "-root" ) ){
                hasArchivePath = true;
                argc--;
                argv++;
                ons.setArchivePath(argv[0]);
            }
            else if ( !strcmp( argv[0]+1, "s" ) || !strcmp( argv[0]+1, "-save" ) ){
                argc--;
                argv++;
                ons.setSavePath(argv[0]);
            }
            else if ( !strcmp( argv[0]+1, "-use-app-icons" ) ){
                ons.setUseAppIcons();
            }
            else if ( !strcmp( argv[0]+1, "-fileversion" ) ){
                argc--;
                argv++;
                ons.setFileVersion(argv[0]);
            }
            else if ( !strcmp( argv[0]+1, "-fullscreen" ) ){
                ons.setFullscreenMode();
            }
            else if ( !strcmp( argv[0]+1, "-window" ) ){
                ons.setWindowMode();
            }
            else if ( !strcmp( argv[0]+1, "-window-width" ) ){
                argc--;
                argv++;
                ons.setPreferredWidth(argv[0]);
            }
#ifndef NO_LAYER_EFFECTS
            else if ( !strcmp( argv[0]+1, "-no-layers" ) ){
                ons.setNoLayers();
            }
#endif
            else if ( !strcmp( argv[0]+1, "-gameid" ) ){
                argc--;
                argv++;
                ons.setGameIdentifier(argv[0]);
            }
            else if ( !strcmp( argv[0]+1, "-nsa-offset" ) ){
                argc--;
                argv++;
                ons.setNsaOffset(argv[0]);
            }
            else if ( !strcmp( argv[0]+1, "-force-button-shortcut" ) ){
                ons.enableButtonShortCut();
            }
            else if ( !strcmp( argv[0]+1, "-automode-time" ) ){
                argc--;
                argv++;
                ons.setPreferredAutomodeTime(argv[0]);
            }
            else if ( !strcmp( argv[0]+1, "-enable-wheeldown-advance" ) ){
                ons.enableWheelDownAdvance();
            }
            else if ( !strcmp( argv[0]+1, "-debug" ) ){
                ons.add_debug_level();
            }
#if defined (USE_X86_GFX) || defined(USE_PPC_GFX)
            else if ( !strcmp( argv[0]+1, "-disable-cpu-gfx" ) ){
                ons.disableCpuGfx();
                printf("disabling CPU accelerated graphics routines\n");
            }
#endif
            else if ( !strcmp( argv[0]+1, "-disable-rescale" ) ){
                ons.disableRescale();
            }
            else if ( !strcmp( argv[0]+1, "-allow-color-type-only" ) ){
                ons.allow_color_type_only = true;
            }
            else if ( !strcmp( argv[0]+1, "-set-tag-page-origin-to-1" ) ){
                ons.set_tag_page_origin_to_1 = true;
            }
            else if ( !strcmp( argv[0]+1, "-answer-dialog-with-yes-ok" ) ){
                ons.answer_dialog_with_yes_ok = true;
            }
            else if ( !strcmp( argv[0]+1, "-ignore-textgosub-newline" ) ){
                ons.setIgnoreTextgosubNewline();
            }
            else if ( !strcmp( argv[0]+1, "-skip-past-newline" ) ){
                ons.setSkipPastNewline();
            }
            else if ( !strcmp( argv[0]+1, "-edit" ) ){
                ons.enableEdit();
            }
            else if ( !strcmp( argv[0]+1, "-strict" ) ){
                ons.setStrict();
            }
            else if ( !strcmp( argv[0]+1, "-key-exe" ) ){
                argc--;
                argv++;
                ons.setKeyEXE(argv[0]);
            }
#ifdef RCA_SCALE
            else if ( !strcmp( argv[0]+1, "-widescreen" ) ){
                ons.setWidescreen();
            }
#endif
            else if ( !strcmp( argv[0]+1, "-scale" ) ){
                ons.setScaled();
            }
            else if ( !strcmp( argv[0]+1, "-no-movie-upscale" ) ){
                ons.setNoMovieUpscale();
            }
            else if ( !strcmp( argv[0]+1, "-detect-png-nscmask" ) ){
                ons.setMaskType( ONScripterLabel::PNG_MASK_AUTODETECT );
            }
            else if ( !strcmp( argv[0]+1, "-force-png-alpha" ) ){
                ons.setMaskType( ONScripterLabel::PNG_MASK_USE_ALPHA );
            }
            else if ( !strcmp( argv[0]+1, "-force-png-nscmask" ) ){
                ons.setMaskType( ONScripterLabel::PNG_MASK_USE_NSCRIPTER );
            }
            else{
                char errstr[256];
                snprintf(errstr, 256, "unknown option %s", argv[0]);
                ons.errorAndCont(errstr, NULL, "Command-Line Issue", true);
            }
        }
        else if (!hasArchivePath) {
            hasArchivePath = true;
            ons.setArchivePath(argv[0]);
            argc--;
            argv++;
        }
        else{
            optionHelp();
        }
        argc--;
        argv++;
    }
}

static bool parseOptionFile(const char *filename, ONScripterLabel &ons, bool &hasArchivePath)
{
    int argc;
    char **argv = NULL;

    argc = 1;
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        //printf("Couldn't open option file '%s'\n", filename);
        return false;
    }

    printf("Reading command-line options from '%s'\n", filename);
    int numlines = 1;
    int curlen = 0, maxlen = 0;
    while (!feof(fp)) {
        char ch = (char) fgetc(fp);
        ++curlen;
        if ((ch == '\0') || (ch == '\r') || (ch == '\n')) {
            ++numlines;
            if (curlen > maxlen)
                maxlen = curlen;
            curlen = 0;
        }
    }
    if (curlen > 0) {
        if (curlen > maxlen)
            maxlen = curlen;
        ++numlines;
    }
    if (numlines > 0) {
        fseek(fp, 0, SEEK_SET);
        numlines *= 2;
        argv = new char*[numlines+1];
        for (int i=0; i<=numlines; i++)
            argv[i] = NULL;
        char *tmp = new char[maxlen+1];
        while (!feof(fp) && (argc<numlines)) {
            char *ptmp = tmp;
            if (fgets(ptmp,maxlen+1,fp) == NULL)
                break;
            curlen = strlen(tmp);
            while ((curlen > 0) && ((tmp[curlen-1] == '\n') || (tmp[curlen-1] == '\r'))) {
                tmp[curlen-1] = '\0';
                curlen = strlen(tmp);
            }
            if (curlen == 0) continue;
            if (*ptmp == '#') continue;
            ptmp = strchr(tmp, '=');
            if (ptmp != NULL) {
                *ptmp++ = '\0';
                curlen = strlen(tmp);
                argv[argc] = new char[curlen+3];
                sprintf(argv[argc], "--%s", tmp);
                curlen = strlen(ptmp);
                argv[argc+1] = new char[curlen+1];
                sprintf(argv[argc+1], "%s", ptmp);
                //printf("Got option '%s'='%s'\n", argv[argc], argv[argc+1]);
                argc += 2;
            } else {
                argv[argc] = new char[curlen+3];
                sprintf(argv[argc], "--%s", tmp);
                //printf("Got option '%s'\n", argv[argc]);
                ++argc;
            }
        }
        delete [] tmp;
    }
    fclose(fp);

    // now parse the options
    if ((argv != NULL)) {
        if ((argc > 1) && (argv[1] != NULL))
            parseOptions(argc, argv, ons, hasArchivePath);
        for (int i=0; i<=numlines; i++)
            if (argv[i] != NULL)
                delete[] argv[i];
        delete[] argv;
        return true;
    }
    return false;
}

void redirect_output();

int main( int argc, char **argv )
{
    // Handle redirection of stdout/stderr on a per-platform basis.
    redirect_output();

    ONScripterLabel ons;

#ifdef PSP
    ons.disableRescale();
    ons.enableButtonShortCut();
#endif

#ifdef MACOSX
    //Check for application bundle on Mac OS X
    ons.checkBundled();
#endif

    // ----------------------------------------
    // Parse options
    bool hasArchivePath = false;
#ifdef MACOSX
    if (ons.isBundled()) {
        const int maxpath=32768;
        char cfgpath[maxpath];
        char *tmp = ons.bundleResPath();
        if (tmp) {
            sprintf(cfgpath, "%s/%s", tmp, CFG_FILE);
            parseOptionFile(cfgpath, ons, hasArchivePath);
        }
        tmp = ons.bundleAppPath();
        if (tmp) {
            sprintf(cfgpath, "%s/%s", tmp, CFG_FILE);
            parseOptionFile(cfgpath, ons, hasArchivePath);
        }
    } else
#endif
    parseOptionFile(CFG_FILE, ons, hasArchivePath);
    parseOptions(argc, argv, ons, hasArchivePath);

    // ----------------------------------------
    // Run ONScripter

    // This facilitates semi-hard engine restarts - they won't affect
    // anything like the command-line flags or such, but they cause
    // the engine to entirely restart its setup and execution process.
    //
    // Ways of exiting such as the end command and reaching the end of
    // a script all still work; they all handle their own exiting
    // procedures.
    //
    // Useful for setres command.
    for (;;) {
        if (ons.init()) exit(-1);
        ons.executeLabel();
        SDL_Quit();
        printf("Engine restarting...\n\n");
    }
    
    exit(0);
}

#ifdef WIN32
#include <Windows.h>

static char outputPath[MAX_PATH];
static char stdoutPath[MAX_PATH];
static char stderrPath[MAX_PATH];

#define CSIDL_APPDATA 0x001a // for [Profiles]/[User]/Application Data
# define DIR_SEPARATOR TEXT("/")
typedef HRESULT (WINAPI *SHGetFolderPathA_t )(HWND, int, HANDLE, DWORD, LPTSTR);

/* The standard output files */
#define STDOUT_FILE	TEXT("stdout.txt")
#define STDERR_FILE	TEXT("stderr.txt")

// This will run pre-main and disable SDL's redirect, allowing us to redirect output ourselves to a preferred directory.
int ret = SDL_putenv("SDL_STDIO_REDIRECT=0");

void redirect_output()
{
    DWORD pathlen = 0;
    FILE *newfp = NULL;
    outputPath[0] = 0;
    HMODULE shdll = LoadLibrary("shfolder");
    if (shdll) {
        SHGetFolderPathA_t SHGetFolderPathA = (SHGetFolderPathA_t)GetProcAddress(shdll, "SHGetFolderPathA");
        if (SHGetFolderPathA) {
            char hpath[MAX_PATH];
            HRESULT res = SHGetFolderPathA(0, CSIDL_APPDATA, 0, 0, hpath); //now user-based

            if (res != S_FALSE && res != E_FAIL && res != E_INVALIDARG) {
                sprintf(outputPath, "%s\\ONScripter-EN\\", hpath);
                CreateDirectory(outputPath, 0);
                pathlen = strlen(outputPath);
            }
        }
        FreeLibrary(shdll);
    }
    if (outputPath[0] == 0) pathlen = GetModuleFileName(NULL, outputPath, SDL_arraysize(outputPath));
    
    while ( pathlen > 0 && outputPath[pathlen] != '\\' ) {
        --pathlen;
    }
    outputPath[pathlen] = '\0';

    SDL_strlcpy( stdoutPath, outputPath, SDL_arraysize(stdoutPath) );
    SDL_strlcat( stdoutPath, DIR_SEPARATOR STDOUT_FILE, SDL_arraysize(stdoutPath) );

    SDL_strlcpy( stderrPath, outputPath, SDL_arraysize(stderrPath) );
    SDL_strlcat( stderrPath, DIR_SEPARATOR STDERR_FILE, SDL_arraysize(stderrPath) );
    
    /* Redirect standard output */
    newfp = freopen(stdoutPath, TEXT("w"), stdout);

    if ( newfp == NULL ) {    /* This happens on NT */
#if !defined(stdout)
        stdout = fopen(stdoutPath, TEXT("w"));
#else
        newfp = fopen(stdoutPath, TEXT("w"));
        if ( newfp ) {
            *stdout = *newfp;
        }
#endif
    }

    /* Redirect standard error */
    newfp = freopen(stderrPath, TEXT("w"), stderr);
    if ( newfp == NULL ) {    /* This happens on NT */
#if !defined(stderr)
        stderr = fopen(stderrPath, TEXT("w"));
#else
        newfp = fopen(stderrPath, TEXT("w"));
        if ( newfp ) {
            *stderr = *newfp;
        }
#endif
    }

    setvbuf(stdout, NULL, _IOLBF, BUFSIZ); /* stdout should be buffered */
    setbuf(stderr, NULL);                  /* whereas stderr isn't buffered */
}

#else // For platforms we haven't yet redirected.
void redirect_output()
{

}
#endif
