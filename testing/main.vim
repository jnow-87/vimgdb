let vimgdb_gdb_cmd = 'gdb -q'

echom "argc: " . argc
echom "argv: " . string(argv)

Vimgdb start

Inferior /home/jan/dev/vimgdb/build/release/testing/main
Inferior tty internal
Inferior args "1 2" 3 4
Break add main
Variable add glob
