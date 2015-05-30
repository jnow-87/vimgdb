""""""""""""""""""""
" global functions "
""""""""""""""""""""

" \brief	read file if it exists, otherwise wait untils its readable with timeout
"
" \param	filename	file to read
" \param	timeout_s	number of seconds to wait for file to become readable
"
" \return	file content as list
function! vimgdb#util#file_read_list(filename, timeout_s)
	let l:s = 0

	while !filereadable(a:filename)
		sleep 1m
		let l:s += 0.001

		if l:s >= a:timeout_s
			return ""
		endif
	endwhile

	return readfile(a:filename)
endfunction

" \brief	read file if it exists, otherwise wait untils its readable with timeout
"
" \param	filename	file to read
" \param	timeout_s	number of seconds to wait for file to become readable
"
" \return	file content as string
function! vimgdb#util#file_read(filename, timeout_s)
	exec "let l:flst = \"" . join(vimgdb#util#file_read_list(a:filename, a:timeout_s)) . "\""
	return l:flst
endfunction

" \brief	print error message
"
" \param	msg		message to print
function! vimgdb#util#error(msg)
	echohl Error
	echom a:msg
	echohl None

	" let user see the message in case other commands follow
	sleep 1
endfunction

" \brief	execute netbeans command
"
" \param	cmd		command to execute
function! vimgdb#util#cmd(cmd)
	exec "nbkey " . escape(a:cmd, '"')
endfunction

" \brief	create exec-mode abbreviation
"
" \param	abbrev		LHS
" \param	expansion	RHS
function! vimgdb#util#execabbrev(abbrev, expansion)
	exec 'cabbr ' . a:abbrev . ' <c-r>=getcmdpos() == 1 && getcmdtype() == ":" ? "' . a:expansion . '" : "' . a:abbrev . '"<CR>'
endfunction

" \brief	conditional variable assignment
"
" \param	var		variable name
" \param	val		value
function! vimgdb#util#cond_assign(var, val)
	if !exists(a:var)
		exec "let " . a:var . " = \"" . a:val . "\""
	endif
endfunction
