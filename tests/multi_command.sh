#!/bin/bash
echo "testing echo 1 && echo 2. Expected output \"1\" and \"2\""
echo 1 && echo 2
echo "testing echo 1 ; echo 2 && echo 3 || echo 4 && echo 5. Expected output: \"1\",\"2\",and \"3\""
echo 1 ; echo 2 && echo 3
echo "testing false && echo 1 || echo 2. Expected output \"2\""
false && echo 1 || echo 2
