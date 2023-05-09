#!/usr/bin/expect -f

log_user 0
spawn ./pie -a "hi there" 2
expect "pie.1> "

exp_send "kernel arguments\n"

expect {
    "hi there*2" { puts "PASS" }
    "Exception " { puts "FAIL"; exit }
}
