# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_USER_FAULTS => 1, [<<'EOF']);
(buffer-hit) begin
(buffer-hit) create "test_file"
(buffer-hit) open "test_file"
(buffer-hit) writing random bytes in "test_file"
(buffer-hit) close "test_file"
(buffer-hit) invalidating buffer cache
(buffer-hit) open "test_file"
(buffer-hit) read "test_file"
(buffer-hit) close "test_file"
(buffer-hit) open "test_file"
(buffer-hit) read "test_file"
(buffer-hit) close "test_file"
(buffer-hit) new hit rate is better than old hit rate
(buffer-hit) end
buffer-hit: exit(0)
EOF
pass;