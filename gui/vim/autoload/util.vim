""""""""""""""""""""
" global functions "
""""""""""""""""""""
let s:sync_file = "/tmp/vimgdb_sync"
let s:data_file = "/tmp/vimgdb_data"
let s:timeout_s = 5


" \brief	print error message
"
" \param	msg		message to print
function vimgdb#util#error(msg)
	echohl Error
	echom a:msg
	echohl None

	" let user see the message in case other commands follow
	sleep 1
endfunction

" \brief	execute netbeans command
"
" \param	cmd		command to execute
function vimgdb#util#cmd(cmd)
	" delete sync file
	call delete(s:sync_file)

	" issue command
	exec "nbkey " . escape(a:cmd . " " . s:sync_file, '"')

	let l:s = 0

	" wait for sync file to be readable again
	while !filereadable(s:sync_file)
		sleep 1m
		let l:s += 0.001

		if l:s >= s:timeout_s
			echoerr "timeout executing vimgdb command: " . a:cmd
			return -1
		endif
	endwhile

	return 0
endfunction

" \brief	issue vimgdb-command and wait for its response
"
" \param	cmd		vimgdb-command to execute
"
" \return	command response as list
function vimgdb#util#cmd_get_data_list(cmd)
	" delete data file
	call delete(s:data_file)

	" issue command
	if vimgdb#util#cmd(a:cmd . " " . s:data_file) != 0
		return []
	endif

	" read file
	return readfile(s:data_file)
endfunction

" \brief	issue vimgdb-command and wait for its response
"
" \param	cmd		vimgdb-command to execute
"
" \return	command response as string
function vimgdb#util#cmd_get_data_string(cmd)
	exec "return \"" . join(vimgdb#util#cmd_get_data_list(a:cmd)) . "\""
endfunction

" \brief	create exec-mode abbreviation
"
" \param	abbrev		LHS
" \param	expansion	RHS
function vimgdb#util#execabbrev(abbrev, expansion)
	exec 'cabbr ' . a:abbrev . ' <c-r>=getcmdpos() == 1 && getcmdtype() == ":" ? "' . a:expansion . '" : "' . a:abbrev . '"<CR>'
endfunction
