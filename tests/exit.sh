echo "testing single command exit.  Expected ouptut is exiting out of rshell."
exit
echo "testing echo 1 && exit.  Expected output is \"1\" then exiting rshell."
echo 1 && exit
echo "testing false || exit.  Expected output is exiting out of rshell."
false || exit
