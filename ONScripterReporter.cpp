#include "ONScripterReporter.h"

bool ONScripterReporter::reportError( const char *title, const char *errstr, bool is_simple, bool is_warning ) const {
    bool result = false;

    if(this->onscripter) {
        result = this->onscripter->doErrorBox(title, errstr, is_simple, is_warning);
    } else {
        result = false;
    }

    return result;
}
