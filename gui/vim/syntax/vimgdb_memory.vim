" use region for being able to hide the content-changed indicator '`'
syn region 	vimgdb_content_changed start="`" end=" " contains=vimgdb_hidden

" hide content-changed indicator '`'
syn match	vimgdb_hidden			"`" contained
syn match	vimgdb_memory_unknown	"?? "
