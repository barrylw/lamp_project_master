<<<<<<< HEAD
#!/bin/sh
uncrustify -q -c `dirname $0`/uncrustify.cfg -f $1 | diff -u $1 --to-file=/dev/stdin
=======
#!/bin/sh
uncrustify -q -c `dirname $0`/uncrustify.cfg -f $1 | diff -u $1 --to-file=/dev/stdin
>>>>>>> 1c8a365a388f3826ae65a4404b1caaf07e71bb24
