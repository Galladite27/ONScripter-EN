// A simple program to create resources.cpp
// Usage: embed [input1 internalname1 input2...]

#include <stdio.h>

struct item {
    const char *file;
    int len;
    item *next;
    item(): file(0), len(0), next(0) {};
    item(const char *f, int l): file(f), len(l), next(0) {};
} names;

int main(int argc, char** argv)
{
    puts("// Generated file - do not edit");
    puts("");
    puts("#include <string.h>");
    puts("#include \"resources.h\"");
    for (int i = 1, j = 1; i < argc; i += 2, ++j) {
        FILE* f = fopen(argv[i], "rb");
        if (f) {
            int len = 0, c;
            printf("\nstatic const unsigned char resource_%d_buffer[] = {", j);
            while ((c = getc(f)) != EOF) {
                if (len) putchar(',');

                if (len++ % 16 == 0) printf("\n\t");else putchar(' ');

                printf("%3d", c);
            }
            fclose(f);
            puts("\n};");
            names.next = new item(argv[i+1], len);
        }
    }

    printf("\nstatic const InternalResource resource_list[] = {");
    int i = 1;
    
    for (item *iptr = names.next; iptr != 0; ++i) {
        printf("\n\t{ \"%s\", resource_%d_buffer, %d },",
               iptr->file, i, iptr->len);
        item *tmp = iptr;
        iptr = iptr->next;
        delete tmp;
    }
    names.next = 0;

    puts("\n\t{ 0, 0, 0 }\n};");
    puts("");
    puts("const InternalResource* getResource(const char* filename)");
    puts("{");
    puts("\tfor (const InternalResource* rv = resource_list; rv->buffer; ++rv) {");
    puts("\t\tif (strcmp(rv->filename, filename) == 0) return rv;");
    puts("\t}");
    puts("\treturn NULL;");
    puts("}");
}
