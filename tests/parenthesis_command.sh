#!/bin/bash
echo "testing (echo A && echo B) || (echo C && echo D). Expected output: \"A\" and \"B\""
(echo A && echo B) || (echo C && echo D)
echo "testing (echo K; echo O) && (echo P || echo T). Expected output: \"K\" and \"O\" and \"P\""
(echo K; echo O) && (echo P || echo T)
echo "testing (echo A || echo B) && (echo C || echo D). Expected output: \"A\" and \"C\""
(echo A || echo B) && (echo C || echo D)

