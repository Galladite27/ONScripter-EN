© 2010 Roto <roto@roto1.net>

These cocoa_*.{mm,h} files provide wrappers for the Cocoa functionality that's needed by
ONScripter-EN.  These functions replace calls to the old Carbon APIs.  While the Carbon
APIs are still supported by OS X, they are not available on other OS X-like platforms
(i.e. iPhone OS.)  These wrappers are the beginnings of allowing ONScripter-EN to be
compiled properly for these devices.
