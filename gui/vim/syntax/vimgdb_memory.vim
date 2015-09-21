" use region for being able to hide the content-changed indicator '`'
syn region 	vimgdb_content_changed start="`" end="[ (\n\n)]" contains=vimgdb_hidden
syn match	vimgdb_memory_unknown	"?? "

" hide content-changed indicator '`'
syn match	vimgdb_hidden			"`" contained conceal

" navigation item [+] or [-]
syn match	vimgdb_navigation		"^ *\zs\[[+-]\]\ze"

