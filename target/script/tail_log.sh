tail -n 2000 -f /IMAX_USER/log/Error.log | perl -pe 's/.*WARNING.*|.*ERROR.*/\e[1;31m$&\e[0m/g'
