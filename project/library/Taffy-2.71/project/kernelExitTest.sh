#!/usr/bin/expect -f

log_user 0

spawn ./taffy src/tests/KernelExitTest.ty

expect {
    "one" { puts "PASS" }
    "two" { puts "FAIL"; exit }
}

spawn ./taffy src/tests/KernelExitTestInMethod.ty

expect {
    "one" { puts "PASS" }
    "two" { puts "FAIL"; exit }
}
