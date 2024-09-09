#include "ONScripterLabel.h"
#include "Reporter.h"

#ifndef _ONSCRIPTERREPORTER_H
#define _ONSCRIPTERREPORTER_H

class ONScripterReporter : public Reporter {
public:
    ONScripterReporter(ONScripterLabel *onscripter) {
        this->onscripter = onscripter;
    };

    ~ONScripterReporter() {
        this->onscripter = NULL;
    };

    bool reportError( const char *title, const char *errstr, bool is_simple=false, bool is_warning=false ) const;
private:
    ONScripterLabel *onscripter;
};

#endif
