"""""""""""""""""""
" local variables "
"""""""""""""""""""

let s:cmd_dict = {
	\ "inferior": {
		\ "__nested__":"vimgdb#inferior#complete",
		\ "bin":{"__nested__":"vimgdb#complete#file"},
		\ "sym":{"__nested__":"vimgdb#complete#file"},
		\ "args":{"<args>":{}},
		\ "tty":{"__nested__":"vimgdb#complete#pts"},
		\ "view":{},
		\ "open":{},
	\ }
\ }

let g:vimgdb_inferior_files = ""
let g:vimgdb_inferior_vars = ""
let g:vimgdb_inferior_functions = ""


""""""""""""""""""""
" global functions "
""""""""""""""""""""

" \brief	init inferior command
function! vimgdb#inferior#init()
	" update vimgdb completion
	call vimgdb#complete#expand(s:cmd_dict, s:cmd_dict, s:cmd_dict)

	" command
	command! -nargs=+ -complete=custom,vimgdb#complete#lookup Inferior call s:inferior(<f-args>)
	call vimgdb#util#execabbrev("inf", "Inferior")

	" autocmd for inferior window
	exec "autocmd! BufWinEnter " . g:vimgdb_inferior_name . " silent
		\ setlocal noswapfile |
		\ setlocal noequalalways |
		\ setlocal bufhidden=delete |
		\ setlocal nowrap
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
	if a:1 == "open"
		call vimgdb#window#open(g:vimgdb_inferior_name, 1)
		call vimgdb#util#cmd("inferior view")

	elseif a:1 == "view"
		call vimgdb#window#view(g:vimgdb_inferior_name)
		call vimgdb#util#cmd("inferior view")

	else
		if a:1 != "bin" && a:1 != "args" && a:1 != "tty"
			let l:file = a:1

			if a:1 == "sym"
				let l:file = a:2
			endif

			" read symbols from file
			let g:vimgdb_inferior_vars = system("nm --demangle " . l:file . " |grep -e '[0-9a-f]* [bBCdDgGrRsS] ' | cut -d ' ' -f 3- | grep '.' | sort | uniq")
			let g:vimgdb_inferior_functions = system("nm --demangle " . l:file . " |grep -e '[0-9a-f]* T ' | cut -d ' ' -f 3- | grep '.' | sort | uniq")
			let g:vimgdb_inferior_files = system("readelf  -s " . l:file . " |grep ' FILE' | tr -s ' ' | cut -d ' ' -f 9 | tr -s ' ' | grep '.' | sort | uniq")
		endif

		call vimgdb#window#open(g:vimgdb_inferior_name, 1)
		call vimgdb#util#cmd("inferior " . join(a:000))
	endif
endfunction
