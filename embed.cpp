// A simple program to create resources.cpp
// Usage: embed -o output [input1 internalname1 input2...]

#include <iostream>
#include <fstream>
#include <cstdlib>

struct item {
    const char *file;
    int len;
    item *next;
    item(): file(0), len(0), next(0) {};
    item(const char *f, int l): file(f), len(l), next(0) {};
} names;

int main(int argc, char** argv)
{
    int result = 0;
    std::ofstream output;

    if (argc < 4) {
        std::cerr << "ERROR : too few arguments!" << std::endl;
        std::cerr << "Usage: " << argv[0] << " -o output [input1 internalname1 input2...]" << std::endl;
        result = 1;
        exit(result);
    }
    output.open(argv[2]);
    output << "// Generated file - do not edit\n";
    output << "\n";
    output << "#include <string.h>\n";
    output << "#include \"resources.h\"" << std::endl;
    for (int i = 3, j = 1; i < argc; i += 2, ++j) {
        FILE* f = fopen(argv[i], "rb");
        if (!f) {
            output.close();
            std::cerr << "ERROR : could not open " << argv[i] << std::endl;
            result = 1;
            exit(result);
        } else {
            int len = 0, c;
            char* formatted = new char[4];
            output << "\nstatic const unsigned char resource_" << j << "_buffer[] = {";
            while ((c = getc(f)) != EOF) {
                if (len) output << ',';

                if (len++ % 16 == 0) output << "\n\t"; else output << ' ';

		snprintf(formatted, 4, "%3d", c);

                output << formatted;
            }
            delete formatted;
            fclose(f);
            output << "\n};" << std::endl;
            names.next = new item(argv[i+1], len);
        }
    }

    output << std::endl << "static const InternalResource resource_list[] = {";
    int i = 1;
    const int len = 256;
    
    for (item *iptr = names.next; iptr != 0; ++i) {
        char* str = new char[len];
        snprintf(str, len, "\n\t{ \"%s\", resource_%d_buffer, %d },",
               iptr->file, i, iptr->len);
        output << str;
        delete str;
        item *tmp = iptr;
        iptr = iptr->next;
        delete tmp;
    }
    names.next = 0;

    output << "\n\t{ 0, 0, 0 }\n};\n";
    output << "\nconst InternalResource* getResource(const char* filename)";
    output << "\n{";
    output << "\n\tfor (const InternalResource* rv = resource_list; rv->buffer; ++rv) {";
    output << "\n\t\tif (strcmp(rv->filename, filename) == 0) return rv;";
    output << "\n\t}";
    output << "\n\treturn NULL;";
    output << "\n}" << std::endl;

    output.close();

    return result;
}
