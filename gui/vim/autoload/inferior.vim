"""""""""""""""""""
" local variables "
"""""""""""""""""""

let s:cmd_dict = {
	\ "Inferior": {
		\ "__nested__":"vimgdb#inferior#complete",
		\ "bin":{"__nested__":"vimgdb#complete#file"},
		\ "sym":{"__nested__":"vimgdb#complete#file"},
		\ "args":{},
		\ "tty":{"__nested__":"vimgdb#complete#pts"},
		\ "view":{},
	\ }
\ }


""""""""""""""""""""
" global functions "
""""""""""""""""""""

" \brief	init inferior command
function! vimgdb#inferior#init()
	" update vimgdb completion
	call extend(g:vimgdb_cmd_dict, s:cmd_dict)

	" command
	command! -nargs=+ -complete=custom,vimgdb#complete#lookup Inferior call s:inferior(<f-args>)
	call vimgdb#util#execabbrev("inf", "Inferior")

	" autocmd for inferior window
	exec "autocmd! BufWinEnter " . g:vimgdb_inferior_name . " silent
		\ setlocal noswapfile |
		\ setlocal noequalalways |
		\ setlocal bufhidden=delete
		\ "
endfunction

function! vimgdb#inferior#cleanup()
	" rm command
	unabbrev inf
	delcommand Inferior

	" rm autocmd
	exec "autocmd! BufWinEnter " . g:vimgdb_inferior_name
endfunction

" \brief	complete inferior inferior arguments
"
" \param	subcmd	current argument supplied in command line
function! vimgdb#inferior#complete(subcmd)
	return "bin\nsym\nargs\ntty\n" . vimgdb#complete#file(a:subcmd)
endfunction


"""""""""""""""""""
" local functions "
"""""""""""""""""""

" \brief	inferior command implementation
function! s:inferior(...)
	if a:1 == "view"
		exec "Window open " . g:vimgdb_inferior_name

		" wait for vimgdb to assign id to new buffer, otherwise the
		" subsequent vimgdb#util#cmd() is not able to send the command
		sleep 100m
	elseif a:1 != "bin" && a:1 != "args" && a:1 != "tty"
		let l:file = a:1

		if a:1 == "sym"
			let l:file = a:2
		endif

		" read symbols from file
		let g:vimgdb_inferior_vars = system("nm --demangle " . l:file . " |grep -e '[0-9a-f]* [bBCdDgGrRsS] ' | cut -d ' ' -f 3- | grep '.' | sort | uniq")
		let g:vimgdb_inferior_functions = system("nm --demangle " . l:file . " |grep -e '[0-9a-f]* T ' | cut -d ' ' -f 3- | grep '.' | sort | uniq")
		let g:vimgdb_inferior_files = system("readelf  -s " . l:file . " |grep ' FILE' | tr -s ' ' | cut -d ' ' -f 9 | tr -s ' ' | grep '.' | sort | uniq")
	endif

	" exec vimgdb command
	call vimgdb#util#cmd("inferior " . join(a:000))
endfunction
