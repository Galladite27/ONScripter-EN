#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char def_out[] = "nscript.dat";

#ifdef main
#undef main
#endif

int main( int argc, char **argv )
{
    int ch, last_ch='\n';
    char *out_filename = NULL, *in_filename = NULL;
    FILE *in_fp, *out_fp;
    bool use_stdout = false, use_stdin = false;
    
    if ( argc > 1 ) {
        if ( !strcmp( argv[1], "-o" ) ) {
            argc--;
            argv++;
            if (argc < 3)
                argc = -1;
            else {
#ifndef WIN32
                if (strcmp(argv[1], "-")) {
                    // "-" arg means "use stdout"
#endif
                    out_filename = argv[1];
#ifndef WIN32
                } else {
                    use_stdout = true;
                }
#endif
                argc--;
                argv++;
            }
        }
#ifndef WIN32
        if ((argc > 1) && !strcmp(argv[1], "-")) {
            // "-" arg means "use stdin"
            use_stdin = true;
        }
#endif
    } else
        argc = -1;
    if (argc < 0) {
        fprintf( stderr, "Usage: nscmake [-o nsc_file] in_file(s)\n");
        fprintf( stderr, "	(nsc_file defaults to \"nscript.dat\")\n");
        exit(-1);
    }

    if (use_stdout)
        out_fp = stdout;
    else if (out_filename)
        out_fp = fopen( out_filename, "wb" );
    else
        out_fp = fopen( def_out, "wb" );
    if (!out_fp && !use_stdout) {
        if (out_filename)
            fprintf( stderr, "Couldn't open '%s' for writing\n", out_filename);
        else
            fprintf( stderr, "Couldn't open '%s' for writing\n", def_out);
        exit(1);
    }

    if (use_stdin)
        argc = -1;

    do {
        if (!use_stdin) {
            in_filename = argv[1];
            in_fp = fopen( in_filename, "rb" );
        } else {
            in_filename = NULL;
            in_fp = stdin;
        }
        if (!in_fp && !use_stdin) {
            fprintf( stderr, "Couldn't open '%s' for reading\n", in_filename);
            exit(1);
        }

        ch = fgetc(in_fp);
        bool last_was_newline = true;
        while (!feof(in_fp)) {
            //write a single '\n' for the newline sequences '\r\n', '\r', '\n'
            if (ch == '\r')
                fputc('\n' ^ 0x84, out_fp);
            else if (!((ch == '\n') && (last_ch == '\r')))
                fputc(ch ^ 0x84, out_fp);

            last_was_newline = ((ch == '\n') || (ch == '\r'));
            last_ch = ch;
            ch = fgetc(in_fp);
        }

        //throw in an ending newline if there wasn't one
        if (!last_was_newline)
            fputc('\n' ^ 0x84, out_fp);

        if (!use_stdin)
            fclose(in_fp);
        argc--;
        argv++;
    } while (argc > 1);

    if (!use_stdout)
        fclose(out_fp);

    exit(0);
}