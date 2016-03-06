#!/bin/bash
echo "testing (echo A && echo B) || (echo C && echo D). Expected output: \"A\" and \"B\""
(echo A && echo B) || (echo C && echo D)
echo "testing (echo K; echo O) && (echo P || echo T). Expected output: \"K\" and \"O\" and \"P\""
(echo K; echo O) && (echo P || echo T)
echo "testing (echo A || echo B) && (echo C || echo D). Expected output: \"A\" and \"C\""
(echo A || echo B) && (echo C || echo D)
echo "testing (false || echo 1 && echo 3) && (echo 6 || echo 7). Expected output \"1\" and \"6\""
(false || echo 1 && echo 3) && (echo 6 || echo 7)
echo "make sure in root directory. testing ([makefile] && echo 1) || echo 2. Expected output \"(true)\" and \"1\"
([makefile] && echo 1) || echo 2
echo "make sure in root directory. testing ((test -d src || echo 1) && echo 2) && echo 3. Expected output \"(true)\" and \"2\" and \"3\"
((test -d src || echo 1) && echo 2) && echo 3
echo "make sure in root directory. testing ([-e asdfasdf] || echo 1) && echo 2 || echo 3. Expected output \"(false)\" and \"1\" and \"2\"
([-e asdfasdf] || echo 1) && echo 2 && echo 3
