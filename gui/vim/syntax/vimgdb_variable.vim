" use region to allow hiding the start and end patterns
syn region 	vimgdb_content_changed		matchgroup=None start="´c" end="`c" concealends
syn region 	vimgdb_callstack_filename	matchgroup=None start="´fl" end="`fl" concealends
syn region 	vimgdb_callstack_function	matchgroup=None start="´fu" end="`fu" concealends
syn region 	vimgdb_callstack_line 		matchgroup=None start="´ln" end="`ln" concealends

" navigation item [+] or [-]
syn match	vimgdb_navigation		"^ *\zs\[[+-]\]\ze"
