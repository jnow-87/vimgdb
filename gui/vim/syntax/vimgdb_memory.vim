" use region for being able to hide the content-changed indicator '`'
syn region 	vimgdb_content_changed matchgroup=None start="`" matchgroup=None end="`" concealends
syn match	vimgdb_memory_unknown	"?? "

" navigation item [+] or [-]
syn match	vimgdb_navigation		"^ *\zs\[[+-]\]\ze"
