CC=gcc
FAILURE="not found, or not usable\n"

#################
# Check for gcc #
#################

printf "Checking for gcc..."
$CC tests/compile/simple_test.c

if [ $? -ne 0 ]
then
    echo "gcc is $FAILURE"
    exit
else
    printf "check\n"
fi

#####################
# Check for ncurses #
#####################

printf "Checking for ncurses..."
$CC tests/compile/curses_test.c

if [ $? -ne 0 ]
then
    echo "* ncurses is $FAILURE"
else
    printf "check\n"
    echo "USING_NCURSES=yes" > makefile.config
fi

######################
# Check for readline #
######################

printf "Checking for readline..."
$CC tests/compile/readline_test.c

if [ $? -ne 0 ]
then
    echo "* readline is $FAILURE"
else
    printf "check\n"
    echo "USING_READLINE=yes" >> makefile.config
fi

#################
# Check for mps #
#################

printf "Checking for mpfr and gmp..."
$CC tests/compile/mp_test.c

if [ $? -ne 0 ]
then
    echo "* mpfr or gmp are $FAILURE"
else
    printf "check\n"
    echo "USING_MP=yes" >> makefile.config
fi

