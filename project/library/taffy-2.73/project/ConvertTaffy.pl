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

$directory = $ARGV[0] or die $1;
$filename = $ARGV[1] or die $1;
$realFilename = $directory . "/" . $filename . ".ty";
$compiledCFilename = $directory . "/Compiled" . "$filename" . ".c";
$compiledHFilename = $directory . "/Compiled" . "$filename" . ".h";

print "C Compiling Taffy file: $realFilename with ";
print "output: $compiledCFilename and $compiledHFilename\n";

open FILE, "<", $realFilename or die $1;
open OUT_FILE, ">", $compiledCFilename or die $1;
open OUT_HEADER_FILE, ">", $compiledHFilename or die $1;

print OUT_HEADER_FILE "//\n";
print OUT_HEADER_FILE "// This file is part of Taffy, a mathematical programming language.\n";
print OUT_HEADER_FILE "// Copyright (C) 2016-2017 Arithmagic, LLC\n";
print OUT_HEADER_FILE "//\n";
print OUT_HEADER_FILE "// Taffy is free software: you can redistribute it and/or modify\n";
print OUT_HEADER_FILE "// it under the terms of the GNU Lesser General Public License as published by\n";
print OUT_HEADER_FILE "// the Free Software Foundation, either version 3 of the License, or\n";
print OUT_HEADER_FILE "// (at your option) any later version.\n";
print OUT_HEADER_FILE "//\n";
print OUT_HEADER_FILE "// Taffy is distributed in the hope that it will be useful,\n";
print OUT_HEADER_FILE "// but WITHOUT ANY WARRANTY; without even the implied warranty of\n";
print OUT_HEADER_FILE "// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n";
print OUT_HEADER_FILE "// GNU Lesser General Public License for more details.\n";
print OUT_HEADER_FILE "//\n";
print OUT_HEADER_FILE "// You should have received a copy of the GNU Lesser General Public License\n";
print OUT_HEADER_FILE "// along with this program.  If not, see <http://www.gnu.org/licenses/>.\n";
print OUT_HEADER_FILE "//\n";
print OUT_HEADER_FILE "\n";

print OUT_FILE "//\n";
print OUT_FILE "// This file is part of Taffy, a mathematical programming language.\n";
print OUT_FILE "// Copyright (C) 2016-2017 Arithmagic, LLC\n";
print OUT_FILE "//\n";
print OUT_FILE "// Taffy is free software: you can redistribute it and/or modify\n";
print OUT_FILE "// it under the terms of the GNU Lesser General Public License as published by\n";
print OUT_FILE "// the Free Software Foundation, either version 3 of the License, or\n";
print OUT_FILE "// (at your option) any later version.\n";
print OUT_FILE "//\n";
print OUT_FILE "// Taffy is distributed in the hope that it will be useful,\n";
print OUT_FILE "// but WITHOUT ANY WARRANTY; without even the implied warranty of\n";
print OUT_FILE "// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n";
print OUT_FILE "// GNU Lesser General Public License for more details.\n";
print OUT_FILE "//\n";
print OUT_FILE "// You should have received a copy of the GNU Lesser General Public License\n";
print OUT_FILE "// along with this program.  If not, see <http://www.gnu.org/licenses/>.\n";
print OUT_FILE "//\n";
print OUT_FILE "\n";

print OUT_FILE "const char *__compiled" . $filename . " =";

print OUT_HEADER_FILE "#ifndef __C_COMPILED_FROM_TAFFY_" . $filename . "__\n";
print OUT_HEADER_FILE "#define __C_COMPILED_FROM_TAFFY_" . $filename . "__\n\n";
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
