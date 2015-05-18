" use region for being able to hide the content-changed indicator '`'
" matchgroup is location sensitive, in this case 'start' can be highlighted
" while all 'end' patterns are set to None
syn region 	vimgdb_content_changed start="`" matchgroup=None end=",\|)\|$" contains=vimgdb_hidden

" hide content-changed indicator '`'
syn match	vimgdb_hidden			"`" contained conceal

" navigation item [+] or [-]
syn match	vimgdb_navigation		"^ *\zs\[[+-]\]\ze"
syn match	vimgdb_function_name	"[\[\] +-]*\zs[0-9a-zA-Z-_]\+\ze(.*)\?$"

