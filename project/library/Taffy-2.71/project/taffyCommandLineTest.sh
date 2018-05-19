#!/usr/bin/expect -f

log_user 0

spawn ./taffy --version

expect {
    "2016-2017 Arithmagic, LLC" { puts "PASS" }
    "Display this help" { puts "FAIL"; exit }
}

spawn ./taffy -v

expect {
    "2016-2017 Arithmagic, LLC" { puts "PASS" }
    "Display this help" { puts "FAIL"; exit }
}

spawn ./taffy -c "io putLine: \"hi there\""

expect {
    "hi there" { puts "PASS" }
    "==> nil" { puts "FAIL"; exit }
}

spawn ./taffy -c "1 + 1"

expect {
    "==> 2" { puts "PASS" }
    "==> nil" { puts "FAIL"; exit }
}

spawn ./taffy src/tests/ArgumentsPrinter.ty -a 1 2 3 4

expect {
    "*1*2*3*4*" { puts "PASS" }
    "*Exception*" { puts "FAIL"; exit }
}
