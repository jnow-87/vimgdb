" use region to allow hiding the start and end patterns
syn region 	vimgdb_content_changed	matchgroup=None start="´c" end="`c" concealends
syn region 	vimgdb_memory_heading	matchgroup=None start="´h0" end="`h0" concealends
syn region 	vimgdb_memory_addr		matchgroup=None start="´h1" end="`h1" concealends
syn region 	vimgdb_memory_ascii		matchgroup=None start="´h2" end="`h2" concealends

syn match	vimgdb_memory_unknown	"?? "

" navigation item [+] or [-]
syn match	vimgdb_navigation		"^ *\zs\[[+-]\]\ze"
