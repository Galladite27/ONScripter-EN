#ifndef _REPORTER_H
#define _REPORTER_H

class Reporter {
public:
    virtual ~Reporter() {};
    virtual bool reportError( const char *title, const char *errstr, bool is_simple=false, bool is_warning=false ) const = 0;
};

#endif