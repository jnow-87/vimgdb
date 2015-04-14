""""""""""""""""""""
" global functions "
""""""""""""""""""""

" \brief	init completion
function! vimgdb#complete#init()
	let g:vimgdb_cmd_dict = {}
endfunction

" \brief	clean completion
function! vimgdb#complete#cleanup()
	let g:vimgdb_cmd_dict = {}
endfunction

" \brief	lookup completino
"
" \param	arg		argument at cursor
" \param	line	command line
" \param	pos		offset into line
function! vimgdb#complete#lookup(arg, line, pos)
	" create argument list based on a:line[0 .. a:pos - 1]
	let l:argv = split(strpart(a:line, 0, a:pos))
	let l:argc = len(l:argv)

	" initialise dictionary pointer
	let l:dict = g:vimgdb_cmd_dict

	" iterate over arguments (l:argv) checking dictionary for completion
	for l:i in range(0, l:argc - 1)
		" check if l:dict contains key argv[i]
		if has_key(l:dict, l:argv[l:i])
			" cycle to next dictionary
			let l:dict = l:dict[l:argv[l:i]]
		else
			" if key is not present check if this is the last argument
			if l:i == l:argc - 1 && a:line[a:pos - 1] != ' '
				" argv[i] is the last argument, since there is no entry in
				" l:dict it is apperently incomplete, hence break the loop
				" returning the completion for l:dict
				break
			endif
			
			" argv[i] is not the last argument, hence this indicated an
			" invalid subcommand, therefor return empty string, indicating
			" the error
			return ""
		endif
	endfor

	" check for nested completion function
	if has_key(l:dict, "__nested__")
		exec "return " . l:dict["__nested__"] . "(\"" . a:arg . "\")"
	endif

	" create string of keys in l:dict
	return join(keys(l:dict), "\n")
endfunction

" \brief	complete file name
"
" \param	subcmd	current argument supplied in command line
function! vimgdb#complete#file(subcmd)
	" get list of files
	exec "let l:files = glob(\"" . a:subcmd . "*\", 0, 1)"

	" append '/' to directories
	let l:i = 0
	for l:file in l:files
		if isdirectory(l:file) != 0
			let l:files[l:i] .= "/"
		endif

		let l:i += 1
	endfor

	return join(l:files, "\n")
endfunction

" \brief	complete pseudo terminal under /dev/pts
"
" \param	subcmd	current argument supplied in command line
function! vimgdb#complete#pts(subcmd)
	" get list of pseudo terminals
	exec "let l:files = globpath(\"/dev/pts/\", \"" . a:subcmd . "*\")"

	return "internal\n" . l:files
endfunction

" \brief	complete inferior location, i.e. source files and function names
"
" \param	subcmd	current argument supplied in command line
function! vimgdb#complete#sym_location(subcmd)
	return g:vimgdb_inferior_files . g:vimgdb_inferior_functions
endfunction

" \brief	complete inferior variable symbol names
"
" \param	subcmd	current argument supplied in command line
function! vimgdb#complete#sym_var(subcmd)
	return g:vimgdb_inferior_vars
endfunction
