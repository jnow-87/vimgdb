" use region for being able to hide the content-changed indicator '`'
syn region 	vimgdb_content_changed	matchgroup=None start="`" matchgroup=None end="`" contains=vimgdb_hidden concealends
syn region 	vimgdb_per_heading 		matchgroup=None start="´" matchgroup=None end="´" contains=vimgdb_hidden concealends

" navigation item [+] or [-]
syn match	vimgdb_navigation		"^ *\zs\[[+-]\]\ze"
