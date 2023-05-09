#!/usr/bin/expect -f

log_user 0

spawn ./taffy src/tests/IOTest.ty

expect {
    "1 2 3" { puts "PASS" }
    default { puts "FAIL"; exit }
}

expect {
    "a b c" { puts "PASS" }
    default { puts "FAIL"; exit }
}

expect {
    "10 11 12" { puts "PASS" }
    default { puts "FAIL"; exit }
}
