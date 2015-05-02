syn match	vimgdb_variable_changed	"[\[\] +-]*`\zs[^,)]*\ze"
syn match	vimgdb_navigation		"^ *\zs\[[+-]\]\ze"
syn match	vimgdb_functionname		"[\[\] +-]*\zs[0-9a-zA-Z-_]\+\ze(.*)\?$"
