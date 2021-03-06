//
// %header%
//

const char *__compiledTaffyUsage =    "Usage: taffy file(s) [options]\n"
    "    Options:\n"
    "\n"
    "    -v or --version           Display version\n"
    "    -f or --file              The file to run. This argument is generally unneeded.\n"
    "                              The following two lines are equivalent:\n"
    "\n"
    "                              $ taffy -a 1 2 3 --file myProgram.ty\n"
    "                              $ taffy myProgram.ty -a 1 2 3\n"
    "    -i or --include           Search-directories for the 'import' directive.\n"
    "                              Examples:\n"
    "                              $ taffy myProgram.ty -i directory1 directory2 directory3\n"
    "    -a or --arguments         Pass arguments into program (accessible via [kernel arguments]). Each argument becomes a string.\n"
    "    -c or --commandLine       Execute code from quoted command line.\n"
    "                              Examples:\n"
    "\n"
    "                              $ taffy -c \"1 + 1\"\n"
    "                              ==> 2\n"
    "\n"
    "                              $ taffy -c \"io putLine: \\\"hi there\\\"\"\n"
    "                              hi there\n"
    "                              ==> nil\n"
    "\n"
    "                              $ taffy -c \"[1, 2, 3, 4] size\"\n"
    "                              ==> 4\n"
    "    --set-max-future-threads  Set the max number of future threads (default 10)\n"
    "    -h or --help              Display this help\n"
;