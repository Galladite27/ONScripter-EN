// A system for storing files within the executable rather than the
// data directory.

#ifndef RESOURCES_H
#define RESOURCES_H

struct InternalResource {
    const char* filename;
    const unsigned char* buffer;
    long int size;
};

extern const InternalResource* getResource(const char* filename);

#endif
