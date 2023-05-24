# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_USER_FAULTS => 1, [<<'EOF']);
(buffer-merge) begin
(buffer-merge) create "test_file"
(buffer-merge) open "test_file"
(buffer-merge) invalidating buffer cache
(buffer-merge) writing 64kB to "test_file"
(buffer-merge) close "test_file"
(buffer-merge) open "test_file"
(buffer-merge) read "test_file"
(buffer-merge) close "test_file"
(buffer-merge) total number of writes is about 128
(buffer-merge) end
buffer-merge: exit(0)
EOF
pass;