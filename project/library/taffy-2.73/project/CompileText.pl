##
## This file is part of Taffy, a mathematical programming language.
## Taffy Copyright (C) 2016-2017 Arithmagic, LLC (taffy@arithmagic.com)
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU Lesser General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.
##

$path = $ARGV[0];
$filename = $ARGV[1];
$realFilename = $path . "/" . "$filename" . ".txt";
$compiledCFilename = $path . "/Compiled" . "$filename" . ".c";
$compiledHFilename = $path . "/Compiled" . "$filename" . ".h";

print "C Compiling Text file: $realFilename with ";
print "output: $compiledCFilename and $compiledHFilename\n";

open FILE, "<", $realFilename or die $1;
open OUT_FILE, ">", $compiledCFilename or die $1;
open OUT_HEADER_FILE, ">", $compiledHFilename or die $1;

print OUT_FILE "//\n";
print OUT_FILE "// %header%\n";
print OUT_FILE "//\n";
print OUT_FILE "\n";

print OUT_HEADER_FILE "//\n";
print OUT_HEADER_FILE "// %header%\n";
print OUT_HEADER_FILE "//\n";
print OUT_HEADER_FILE "\n";

print OUT_FILE "const char *__compiled" . $filename . " =";

print OUT_HEADER_FILE "#ifndef __TEXT_COMPILED_INTO_C_FOR_" . $filename . "__\n";
print OUT_HEADER_FILE "#define __TEXT_COMPILED_INTO_C_FOR_" . $filename . "__\n\n";
print OUT_HEADER_FILE "extern const char *__compiled" . $filename . ";\n\n";
print OUT_HEADER_FILE "#endif\n";

# get past the header
$line = <FILE>;

while ($line =~ m/\/\//)
{
    $line = <FILE>;
}

while (my $line = <FILE>) {
  $line =~ s/\"/\\\"/g;
  chomp($line);
  print OUT_FILE '    "';
  print OUT_FILE $line;
  print OUT_FILE "\\n\"\n";
}

print OUT_FILE ';';

close FILE;
close OUT_FILE;
close OUT_HEADER_FILE;
