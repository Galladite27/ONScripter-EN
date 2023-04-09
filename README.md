# ONScripter-EN
*An enhanced portable open-source NScripter implementation*

## Last Updated
9 April 2023

## History
Naoki Takahashi's NScripter is a popular Japanese game engine used for both commercial and free visual novels.  It attained popularity due to its liberal terms of use and relative simplicity.  However, it is closed source software and only available for Windows.

A number of cross-platform clones exist, and Ogapee's [ONScripter](https://onscripter.osdn.jp/onscripter.html) is the most popular of these.  Due to the ease with which it can be modified to support languages other than Japanese, ONScripter has been adopted by the visual novel localisation community as the engine of choice for translated NScripter games; the patch to support English even made it into the main ONScripter source code in 2005.  An English-language centric fork of ONScripter, originally known as onscripter-insani (because it was maintained by [insani](http://nscripter.insani.org/)), was founded at that time, in order to better suit the needs of the English localization community.  Maintenance of this fork was passed on to [Haeleth](http://haeleth.net/) in 2006, and it was renamed ONScripter-EN at that time.  Maintenance then was passed on to [Uncle Mion of Sonozaki Futago-Tachi](https://web.archive.org/web/http://unclemion.com/onscripter/) in 2007, and he maintained this project until 2011.  At that time the project was abandoned.  Between 2011 and 2023, OS-level incompatibility caused the following to occur:

- ONScripter-EN no longer worked on modern macOS
- ONScripter-EN accrued a set of bugs on Windows related to the initial libraries becoming outdated, *etc*.

For a long time, chaoskaiser72 of [kaisernet](https://kaisernet.org/) had been offering cash bounties related to updating and maintenance of ONScripter-EN.  In 2023, [@Galladite27](https://github.com/Galladite27) took on the task of maintaining ONScripter-EN going forward.  Independently of this, on 1 April 2023 [insani](http://insani.org) came out of its 14-year retirement, and began contributing to the ONScripter-EN codebase shortly thereafter.

We have now come full circle.  In the words of *True Remembrance*:

> Our time is now.    
> And this is our story.

### Related Efforts
As of April 2023, [onscripter-insani](https://github.com/insani-org/onscripter-insani) is a sister project to ONScripter-EN.  This is a branch of ONScripter which focuses on portability of compilation and support of modern macOS, as well as on continued support of insani's indie novel game localizations.  It does not have (and does not plan to add) many of the enhancements that are in ONScripter-EN, and the plan of that project is to remain as close to upstream as possible.  For your novel game localizations, we (and insani) recommend ONScripter-EN over onscripter-insani.

## Playing Games
Playing an existing game with ONScripter-EN requires two additional files to be added to the game directory:

- *default.ttf*, a monospaced TrueType font.  If you wish to be maximally compatible with Japanese-only NScripter titles, this font must support Japanese glyphs.  The following are canonical options:
  - [Genjyuu Gothic X Monospace/Monospace Bold](http://jikasei.me/font/genjyuu/)
  - [Migu 2M/2M Bold](http://mix-mplus-ipa.osdn.jp/)
  - [Sazanami Gothic](https://osdn.net/projects/efont/releases/)

- Optionally, *game.id*, a text file containing simply the name of the game.  This is used to identify a separate directory where saved data is stored; unlike the original ONScripter, which places saved games in the game directory, ONScripter-EN does not require games to be kept in a world-writable location.

This done, the game can be played either by dropping the onscripter-en executable into the game directory and running it, or by running it with the -r command-line option to identify the desired game.

The saved game location behaviour can be overridden with the -s command-line option if you wish for some reason to restore the default ONScripter behaviour or to place saves somewhere else.

Please run onscripter-en --help for details of other command-line options (note that on Windows systems, these details and all other console output will be placed in text files in the folder %ALLUSERSPROFILE%\Application Data\ONScripter-En).

## Packaging Games
ONScripter-EN includes some features designed to make packaging new games or translations in a platform-appropriate fashion easier and neater.

Firstly, *game.id* can be replaced by a line

```gameid Your Title Here```

at the start of the script, or immediately after the modeline if present.

Secondly, the saved-game location behaviour described above means that you do not have to worry about installing to a world-writable location; this should simplify matters considerably on Windows Vista+ and on Unix-type platforms.

In the latter case, the recommended approach is to store game data somewhere appropriate (e.g. /usr/local/share/<title>) and to create a shell script in an appropriate place (e.g. /usr/local/bin/<title>) that launches the onscripter executable with a suitable argument to -r.  Since there is no convenient way to distribute binaries that will work even on multiple Linux distributions, let alone other Unix platforms, it is recommended that packages instead depend on the user supplying a separate ONScripter-EN binary, either built from source or from an OS/distro-specific package.

We currently do not support macOS builds, although that support is in the works.  macOS, although it still has a FreeBSD-derived UNIX-alike userland, is not truly a Unix.  It also uses the concept of *App Bundles*, which are specially-formatted directory structures that look to the end user like a single item.  Furthermore, as of more recent macOS versions, it is now impossible to run code that has not been properly code-signed and notarized by an approved Apple Developer.  This makes redistribution of many open-source projects a much more cumbersome procedure (for instance, it costs $99/year to maintain an Apple Developer license).  Our sister project, [onscripter-insani](https://github.com/insani-org/onscripter-insani) provides official Intel and Apple Silicon macOS builds, and we will be porting over that build system to ONScripter-EN as time goes on.

## Build Requirements
ONScripter is based on SDL, and should run on any platform for which SDL is available.  The original version has been successfully compiled on platforms as diverse as iOS, Android, the Nintendo Switch, the PSP/PS Vita, Symbian, *etc*.

ONScripter-EN has slightly more strict requirements, however.  Since ONScripter's original build system is extremely difficult to use (requiring the creation of a custom makefile for every minor platform variation), it has been replaced in this branch with a more conventional configure-build system; this depends on a POSIX-like environment with GNU make.  The code has been tested primarily with the GNU C++ compiler; Intel C++ 10 has also been tried successfully.

The primary test environment is GNU/Linux (x86_64).  Assuming standard build tools are installed, the full-source distribution is likely to compile out-of-the-box or with only minimal changes on truly
POSIX-conformant platforms (*e.g.* GNU/Linux, *BSD).  The primary target environment, on the other hand, is 32-bit Windows (i686).  [MSYS2](https://msys2.org) on Windows 10+ is assumed for these builds, and due to multiple incompatibilities between MSYS2 and the original configure script, specially-modified Makefile for both x86-64 and i686 Windows compilation are provided.  Development of those Makefiles can be found [here](https://github.com/insani-org/onscripter-en-msys2-configure-makefile).

Please see INSTALL in the source distribution for other specific build instructions.

## Localization
While ONScripter proper, and previous versions of ONScripter-EN, require English or Japanese settings to be selected at compile-time, this version can be run in both English and Japanese modes.

In Japanese mode, the built-in menus are in Japanese, numbers are printed using full-width characters, and line-breaking decisions are based on Japanese rules (breaks are allowed in the middle of words, but not before or after certain special characters).

In English mode, the built-in menus are in English, numbers are printed using half-width characters, and line-breaking decisions are based on English rules (breaks are only allowed between words).

You can select a mode within a script by using the commands

```language english```

and

```language japanese```

It is recommended that you just set the language once at startup, but you can switch between them during the game if you need to (if you do, and your game uses the built-in menus, be sure to switch back to your primary language before any point at which the player can bring up a menu, or they will probably be very confused).

The default mode is determined by the name of the ONScripter binary (or app bundle, on macOS).  If it is something like "onscripter-en" or "ONScripter-En.exe", the default mode will be English; otherwise it will be Japanese (this is intended to allow users to play existing games in an appropriate mode -- if releasing a game yourself, you should use an explicit language command instead of relying on this).  You can also use the command-line option "--english" or "--japanese" to specify the default mode.


## Contact Information
The author of ONScripter itself is Ogapee, who can be reached through his ONScripter project website:

- [ONScripter](https://onscripter.osdn.jp/onscripter.html)

Please refrain from contacting Ogapee about ONScripter-EN, as its codebase is now substantially different from mainline ONScripter.

The maintainer of ONScripter-EN (as of February 2023) is Galladite, who can be reached at:

- *E-mail*: galladite@yandex.com
- *GitHub*: [@Galladite27](https://github.com/Galladite27)

The initial maintainer of ONScripter-EN (and current contributor to ONScripter-EN) is Seung Park of insani, who can be reached at:

- *GitHub*: [@lightbinder](https://github.com/lightbinder)
- *GitHub Organization*: [@insani-org](https://github.com/insani-org)

The previous maintainer, "Uncle" Mion Sonozaki, may be reachable at:

- *E-mail*: UncleMion@gmail.com

We do not recommend reaching out to him about ONScripter-EN issues, though, as he is not currently involved with our project.

Portions of this README were written primarily by a previous maintainer, Haeleth, whose website is still up as of 2023:

- *Website*: [haeleth.net](http://haeleth.net/)
  
Haeleth officially retired from our community in 2010, and as such should not be contacted about ONScripter-EN.

Correspondence in English, or regarding issues related to this branch in particular, should be directed to [@Galladite27](https://github.com/Galladite27) or to [@lightbinder](https://github.com/lightbinder), as it contains a large number of customisations that have nothing to do with Ogapee, and neither Haeleth nor Uncle Mion are actively maintaining ONScripter-EN.

## License
ONScripter and ONScripter-EN are distributed under the terms of the GNU General Public License v2.  See COPYING for details.
