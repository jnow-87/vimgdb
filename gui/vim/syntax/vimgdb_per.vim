" use region to allow hiding the start and end patterns
syn region 	vimgdb_content_changed	matchgroup=None start="´c" end="`c" concealends
syn region 	vimgdb_per_section 		matchgroup=None start="´h0" end="`h0" concealends
syn region 	vimgdb_per_heading 		matchgroup=None start="´h1" end="`h1" concealends
syn region 	vimgdb_per_register		matchgroup=None start="´h2" end="`h2" concealends
syn region 	vimgdb_per_bit			matchgroup=None start="´h3" end="`h3" concealends

" navigation item [+] or [-]
syn match	vimgdb_navigation		"^ *\zs\[[+-]\]\ze"
