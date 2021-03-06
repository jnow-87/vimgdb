"""""""""""""""""""
" local variables "
"""""""""""""""""""

let s:cmd_dict = {
	\ "register":{
		\ "fold":{"__nested__":"vimgdb#register#complete"},
		\ "format":{
			\ "__nested__":"vimgdb#register#complete",
			\ "__nested1__":{
				\ "binary":{},
				\ "decimal":{},
				\ "hexadecimal":{},
				\ "octal":{},
				\ "natural":{},
			\ },
		\ },
		\ "set":{
			\ "__nested__":"vimgdb#register#complete",
			\ "__nested1__":{"<value>":{}}
		\ },
		\ "view":{},
		\ "open":{},
	\ }
\ }


""""""""""""""""""""
" global functions "
""""""""""""""""""""

" \brief	init register command
function vimgdb#register#init()
	" update vimgdb completion
	call vimgdb#complete#expand(s:cmd_dict, s:cmd_dict, s:cmd_dict)

	" command
	command -nargs=+ -complete=custom,vimgdb#complete#lookup Register call s:register(<f-args>)
	call vimgdb#util#execabbrev("register", "Register")

	" autocmd for register window
	exec "autocmd BufWinEnter " . g:vimgdb_register_name . " silent
		\ setlocal noswapfile |
		\ setlocal noequalalways |
		\ setlocal bufhidden=delete |
		\ setlocal nowrap |
		\ setlocal filetype=vimgdb_variable |
		\ setlocal conceallevel=3 |
		\ setlocal concealcursor=nivc |
		\ nnoremap <buffer> <silent> <c-f> :exec 'Register fold ' . line('.')<cr>|
		\ nnoremap <buffer> <silent> + :exec 'Register fold ' . line('.')<cr>|
		\ nnoremap <buffer> <silent> - :exec 'Register fold ' . line('.')<cr>|
		\ nnoremap <buffer> <silent> u :exec 'Register view'<cr>|
		\ nnoremap <buffer> i :Register set <c-r>=line('.')<cr> |
		\ nnoremap <buffer> s :Register set <c-r>=line('.')<cr> |
		\ nnoremap <buffer> f :Register format <c-r>=line('.')<cr>
		\ "
endfunction

" \brief	cleanup register command
function vimgdb#register#cleanup()
	" command
	unabbrev register
	delcommand Register

	" autocmd
	exec "autocmd! BufWinEnter " . g:vimgdb_register_name
endfunction

" \brief	complete vimgdb register buffer lines
"
" \param	subcmd	current argument supplied in command line
function vimgdb#register#complete(subcmd)
	" get list of gdb register buffer lines
	return vimgdb#util#cmd_get_data_string("register complete")
endfunction


"""""""""""""""""""
" local functions "
"""""""""""""""""""

" \brief	register command implementation
function s:register(...)
	if a:1 == "open"
		call vimgdb#window#open(g:vimgdb_register_name, 1)
		call vimgdb#util#cmd("register view")

	elseif a:1 == "view"
		call vimgdb#window#view(g:vimgdb_register_name)
		call vimgdb#util#cmd("register view")

	else
		call vimgdb#window#open(g:vimgdb_register_name, 1)
		call vimgdb#util#cmd("register " . join(a:000))
	endif
endfunction
