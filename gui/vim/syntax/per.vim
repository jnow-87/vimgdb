set foldnestmax=2
set foldmethod=syntax

syn keyword per_key	range register bits heading emptyline

syn region per_block start="{" end="}" fold transparent
syn region	per_string start=+L\="+ skip=+\\\\\|\\"+ end=+"+


hi def link per_key		mblue
hi def link per_string	white
