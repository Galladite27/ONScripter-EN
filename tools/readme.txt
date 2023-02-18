ONScripter-Tools:

All tools are meant to be run from the command line.

nscdec.exe: nscdec nsc_file [out_file]
For converting a packaged script file (e.g. "nscript.dat") to a text script
("result.txt" by default)

nscmake.exe: nscmake [-o nsc_file] in_file(s)
For converting text script file(s) to a packaged script file
("nscript.dat" by default); using wildcard pattern "[0-9][0-9].txt" for
in_file(s) works well for merging a set of numbered scripts.


nsadec.exe: nsadec [-offset num] [-o out_dir] arc_file
For extracting files from an nsa archive file (e.g. "arc.nsa");
will place the files under directory out_dir, if provided.
'-offset' replaces the old '-ns2' and '-ns3' options (rarely needed).

ns2dec.exe: ns2dec [-offset num] [-o out_dir] arc_file
For extracting files from an ns2 archive file (e.g. "00.ns2");
will place the files under directory out_dir, if provided

sardec.exe: sardec [-o out_dir] arc_file
For extracting files from a sar archive file (e.g. "arc.sar");
will place the files under directory out_dir, if provided


ns2make.exe: ns2make arc_file -d in_dir
             ns2make arc_file in_file(s)
Creates an ns2 archive arc_file (e.g. "00.ns2").
1) The format with "-d" will process all files (and subfolders) within in_dir,
but won't include in_dir as part of the archived file paths.
2) The next format treats all remaining arguments after arc_file
as files/directories to be added to the archive, recursively.

nsamake.exe: nsamake arc_file [-e] -d in_dir
             nsamake arc_file [-e] in_file(s)
Creates an nsa archive as arc_file (e.g. "arc.nsa").
The usage formats work the same as with ns2make, except that when
the option -e is provided, it will attempt to compress BMP and WAV files
in the archive using NBZ encoding.

sarmake.exe: sarmake arc_file -d in_dir
             sarmake arc_file in_file(s)
Creates a sar archive as arc_file (e.g. "arc.sar").
Usage works the same as with ns2make.


The following tools are intended primarily for quick production of
PDA game conversions.

ns2conv.exe: ns2conv [-offset num] [-p] [-j] [-q quality] [-r rules_file] [-n num_cells] src_width dst_width src_archive_file dst_archive_file
Produces a new ns2 archive file from the provided src_archive_file,
with contained images resized and converted.
Use the '-n num_cells' option to set the default number of cells (and alphas)
it should assume that images contain; use '-r rules_file' to give a file
of num_cell/filepattern pairings that will change the value of num_cells
for files that match a pattern.
Use '-p' to convert bitmap images to png, or '-j' to convert them to jpeg,
for extra space-savings; use '-q' to specify jpeg compression quality.
'-offset' replaces the old '-ns2' and '-ns3' options from nsaconv
(rarely needed).

nsaconv.exe: nsaconv [-offset num] [-e] [-p] [-j] [-q quality] [-r rules_file] [-n num_cells] src_width dst_width src_archive_file dst_archive_file
Produces a new nsa archive file from the provided src_archive_file,
with contained images resized and converted.
Use the '-n num_cells' option to set the default number of cells (and alphas)
it should assume that images contain; use '-r rules_file' to give a file
of num_cell/filepattern pairings that will change the value of num_cells
for files that match a pattern.
Use '-p' to convert bitmap images to png, '-j' to convert them to jpeg
(add '-q' for jpeg quality), or '-e' for enhanced NBZ compression.
'-offset' replaces the old '-ns2' and '-ns3' options (rarely needed).

sarconv.exe: sarconv [-p] [-j] [-q quality] [-r rules_file] [-n num_cells] src_width dst_width src_archive_file dst_archive_file
Produces a new nsa archive file from the provided src_archive_file,
with contained images resized and converted.
Use the '-n num_cells' option to set the default number of cells (and alphas)
it should assume that images contain; use '-r rules_file' to give a file
of num_cell/filepattern pairings that will change the value of num_cells
for files that match a pattern.
Use '-p' to convert bitmap images to png, or '-j' to convert them to jpeg,
for extra space-savings; use '-q' to specify jpeg compression quality.


You can perform batch conversions of image files using the following tool:

batchconv.exe: batchconv [-q quality] [-r rules_file] [-n num_cells] [-o out_dir] src_width dst_width -d in_dir
               batchconv [-q quality] [-r rules_file] [-n num_cells] [-o out_dir] src_width dst_width in_file(s)
Takes a set of files and batch resizes/converts them, using options
similar to those used by *make, *dec and *conv
(except for '-p', '-j', '-q' and '-e').


----------

ONScripter-Tools are maintained by "Uncle" Mion Sonozaki, who may be
contacted on the web at http://onscripter.denpa.mobi or by email
at UncleMion@gmail.com.

ONScripter-Tools are licensed under the GNU Public License;
see GPL.txt for further information.
