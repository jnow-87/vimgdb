"""""""""""""""""""
" local variables "
"""""""""""""""""""

let s:cmd_dict = {
	\ "inferior": {
		\ "__nested__":"vimgdb#complete#file",
		\ "bin":{"__nested__":"vimgdb#complete#file"},
		\ "sym":{"__nested__":"vimgdb#complete#file"},
		\ "args":{"<args>":{}},
		\ "tty":{"__nested__":"vimgdb#inferior#complete_pts"},
		\ "export":{"__nested__":"vimgdb#complete#file"},
		\ "view":{},
		\ "open":{},
	\ }
\ }

let g:vimgdb_inferior_files = ""
let g:vimgdb_inferior_vars = ""
let g:vimgdb_inferior_functions = ""
let g:vimgdb_symfile = ""


""""""""""""""""""""
" global functions "
""""""""""""""""""""

" \brief	init inferior command
function vimgdb#inferior#init()
	" update vimgdb completion
	call vimgdb#complete#expand(s:cmd_dict, s:cmd_dict, s:cmd_dict)

	" command
	command -nargs=+ -complete=custom,vimgdb#complete#lookup Inferior call s:inferior(<f-args>)
	call vimgdb#util#execabbrev("inferior", "Inferior")

	" autocmd for inferior window
	exec "autocmd BufWinEnter " . g:vimgdb_inferior_name . " silent
		\ setlocal noswapfile |
		\ setlocal noequalalways |
		\ setlocal bufhidden=delete |
		\ setlocal nowrap
		\ "
endfunction

" \brief	cleanup inferior command
function vimgdb#inferior#cleanup()
	" rm command
	unabbrev inferior
	delcommand Inferior

	" rm autocmd
	exec "autocmd! BufWinEnter " . g:vimgdb_inferior_name
endfunction

" \brief	complete pseudo terminal under /dev/pts
"
" \param	subcmd	current argument supplied in command line
function vimgdb#inferior#complete_pts(subcmd)
	" get list of pseudo terminals
	if a:subcmd == "" || match(a:subcmd, "/dev/pts/") != -1
		exec "let l:files = globpath(\"/dev/pts/\", \"" . substitute(a:subcmd, "/dev/pts/", "", "") . "*\")"
	else
		let l:files = vimgdb#complete#file(a:subcmd)
	endif

	return "internal\nexternal\n" . l:files
endfunction

" \brief	update inferior symbols
function vimgdb#inferior#update_sym()
	" read symbols from file
	let g:vimgdb_inferior_vars = system("nm --demangle " . g:vimgdb_symfile . " |grep -e '[0-9a-f]* [bBCdDgGrRsS] ' | cut -d ' ' -f 3- | grep '.' | sort | uniq")
	let g:vimgdb_inferior_functions = system("nm --demangle " . g:vimgdb_symfile . " |grep -e '[0-9a-f]* T ' | cut -d ' ' -f 3- | grep '.' | sort | uniq")
	let g:vimgdb_inferior_files = system("readelf  -s " . g:vimgdb_symfile . " |grep ' FILE' | tr -s ' ' | cut -d ' ' -f 9 | tr -s ' ' | grep '.' | sort | uniq")
endfunction


"""""""""""""""""""
" local functions "
"""""""""""""""""""

" \brief	inferior command implementation
function s:inferior(...)
	if a:1 == "open"
		call vimgdb#window#open(g:vimgdb_inferior_name, 1)
		call vimgdb#util#cmd("inferior view")

	elseif a:1 == "view"
		call vimgdb#window#view(g:vimgdb_inferior_name)
		call vimgdb#util#cmd("inferior view")

	else
		if a:1 == "tty" && a:2 == "internal"
			call vimgdb#window#open(g:vimgdb_inferior_name, 1)
		endif

		if a:1 == "bin" || a:1 == "sym" || filereadable(a:1)
			" in case a new binary or symbol file is supplied make 
			" sure it is given as absolute path

			" get the actual file
			if a:1 == "bin" || a:1 == "sym"
				let l:file = a:2
			else
				let l:file = a:1
			endif

			" make the file an absolute path
			if l:file[0] != '/'
				let l:file = getcwd() . "/" . l:file
			endif

			" update symbols
			if a:1 != "bin"
				let g:vimgdb_symfile = l:file
				call vimgdb#inferior#update_sym()
			endif

			" issue the command
			if a:1 == "bin" || a:1 == "sym"
				call vimgdb#util#cmd("inferior " . a:1 . " " . l:file)
			else
				call vimgdb#util#cmd("inferior " . " " . l:file)
			endif
		else
			call vimgdb#util#cmd("inferior " . join(a:000))
		endif
	endif
endfunction
