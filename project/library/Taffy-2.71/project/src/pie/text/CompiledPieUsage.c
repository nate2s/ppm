//
// %header%
//

const char *__compiledPieUsage =    "Usage: pie [options]\n"
    "    Options:\n"
    "\n"
    "    -v or --version           Display version\n"
    "    -i or --include           Search-directories for the 'import' directive. Example:\n"
    "                              $ pie -i directory1 directory2 directory3\n"
    "    -a or --arguments         Pass arguments into program (accessible via [kernel arguments]). Each argument becomes a string.\n"
    "                              Examples:\n"
    "\n"
    "                              $ pie -a \"this is a string\"\n"
    "                              pie.1> kernel arguments\n"
    "                              ==> [\"this is a string\"]\n"
    "\n"
    "                              $ pie -a 1 2 3 4 \"string\"\n"
    "                              pie.2> kernel arguments\n"
    "                              ==> [\"1\", \"2\", \"3\", \"4\", \"string\"]\n"
    "\n"
    "                              // evaluate an argument\n"
    "                              pie.3> kernel eval: [[kernel arguments] objectAtIndex: 0]\n"
    "                              ==> 1\n"
    "    --set-max-future-threads  Set the max number of future threads (default 10)\n"
    "    -h or --help              Display this help\n"
;