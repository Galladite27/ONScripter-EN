#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main( int argc, char **argv )
{
    int ch;
    char *in_filename = NULL, *out_filename = NULL;
    FILE *in_fp, *out_fp;
    bool use_stdout = false;
    
    if ( (argc == 2) || (argc == 3) ) {
#ifndef WIN32
        if (strcmp(argv[1], "-")) {
            // "-" arg means "use stdin"
#endif
            in_filename = argv[1];
#ifndef WIN32
        }
#endif
        if (argc == 3) {
#ifndef WIN32
            if (strcmp(argv[2], "-")) {
                // "-" arg means "use stdout"
#endif
                out_filename = argv[2];
#ifndef WIN32
            } else {
                use_stdout = true;
            }
#endif
        }
    }
    else {
        fprintf( stderr, "Usage: nscdec nsc_file [out_file]\n");
        fprintf( stderr, "	(out_file defaults to \"result.txt\")\n");
        exit(-1);
    }

    if (in_filename)
        in_fp = fopen( in_filename, "rb" );
    else
        in_fp = stdin;
    if (!in_fp && in_filename) {
        fprintf( stderr, "Couldn't open '%s' for reading\n", in_filename);
        exit(1);
    }

    if (use_stdout)
        out_fp = stdout;
    else if (out_filename)
        out_fp = fopen( out_filename, "wb" );
    else
        out_fp = fopen( "result.txt", "wb" );
    if (!out_fp && !use_stdout) {
        if (out_filename)
            fprintf( stderr, "Couldn't open '%s' for writing\n", out_filename);
        else
            fprintf( stderr, "Couldn't open '%s' for writing\n", "result.txt");
        exit(1);
    }

    int last_ch = '\0';
    ch = fgetc(in_fp);
    bool last_was_newline = true;
    while (!feof(in_fp)) {
        ch ^= 0x84;
        last_was_newline = false;
        if ((ch == '\r') || ((ch == '\n') && (last_ch != '\r'))) {
#ifdef WIN32
            fputc('\r', out_fp);
#endif
            fputc('\n', out_fp);
            last_was_newline = true;
        } else
            fputc(ch, out_fp);
        last_ch = ch;
        ch = fgetc(in_fp);
    }

    //throw in an ending newline if there wasn't one
    if (!last_was_newline) {
#ifdef WIN32
        fputc('\r', out_fp);
#endif
        fputc('\n', out_fp);
    }

    if (!use_stdout)
        fclose(out_fp);
    if (in_filename)
        fclose(in_fp);

    exit(0);
}