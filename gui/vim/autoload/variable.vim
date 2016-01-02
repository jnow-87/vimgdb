"""""""""""""""""""
" local variables "
"""""""""""""""""""

let s:cmd_dict = {
	\ "variable":{
		\ "add":{"__nested__":"vimgdb#complete#sym_variable"},
		\ "delete":{"__nested__":"vimgdb#variable#complete"},
		\ "fold":{"__nested__":"vimgdb#variable#complete"},
		\ "format":{
			\ "__nested__":"vimgdb#variable#complete",
			\ "__nested1__":{
				\ "binary":{},
				\ "decimal":{},
				\ "hexadecimal":{},
				\ "octal":{},
				\ "natural":{},
			\ },
		\ },
		\ "set":{
			\ "__nested__":"vimgdb#variable#complete",
			\ "__nested1__":{"<value>":{}}
		\ },
		\ "export":{"__nested__":"vimgdb#complete#file"},
		\ "view":{},
		\ "open":{},
	\ }
\ }

let s:var_lst = ""



""""""""""""""""""""
" global functions "
""""""""""""""""""""

" \brief	init variable command
function vimgdb#variable#init()
	" update vimgdb completion
	call vimgdb#complete#expand(s:cmd_dict, s:cmd_dict, s:cmd_dict)

	" command
	command! -nargs=+ -complete=custom,vimgdb#complete#lookup Variable call s:variable(<f-args>)
	call vimgdb#util#execabbrev("variable", "Variable")

	" autocmd for variables window
	exec "autocmd! BufWinEnter " . g:vimgdb_variables_name . " silent
		\ setlocal noswapfile |
		\ setlocal noequalalways |
		\ setlocal bufhidden=delete |
		\ setlocal nowrap |
		\ setlocal filetype=vimgdb_variable |
		\ setlocal conceallevel=3 |
		\ setlocal concealcursor=nivc |
		\ nnoremap <buffer> <silent> <c-f> :exec 'Variable fold ' . line('.')<cr>|
		\ nnoremap <buffer> <silent> + :exec 'Variable fold ' . line('.')<cr>|
		\ nnoremap <buffer> <silent> - :exec 'Variable fold ' . line('.')<cr>|
		\ nnoremap <buffer> <silent> u :exec 'Variable view'<cr>|
		\ nnoremap <buffer> <silent> dd :exec 'Variable delete ' . line('.')<cr>|
		\ nnoremap <buffer> i :Variable set <c-r>=line('.')<cr> |
		\ nnoremap <buffer> s :Variable set <c-r>=line('.')<cr> |
		\ nnoremap <buffer> f :Variable format <c-r>=line('.')<cr>
		\ "
endfunction

" \brief	cleanup variable command
function vimgdb#variable#cleanup()
	" command
	unabbrev variable
	delcommand Variable

	" autocmd
	exec "autocmd! BufWinEnter " . g:vimgdb_variables_name
endfunction

" \brief	complete vimgdb variable buffer lines
"
" \param	subcmd	current argument supplied in command line
function vimgdb#variable#complete(subcmd)
	return s:var_lst
endfunction


"""""""""""""""""""
" local functions "
"""""""""""""""""""

" \brief	variable command implementation
function s:variable(...)
	if a:1 == "open"
		call vimgdb#window#open(g:vimgdb_variables_name, 1)
		call vimgdb#util#cmd("variable view")

	elseif a:1 == "view"
		call vimgdb#window#view(g:vimgdb_variables_name)
		call vimgdb#util#cmd("variable view")

	else
		call vimgdb#window#open(g:vimgdb_variables_name, 1)
		call vimgdb#util#cmd("variable " . join(a:000))
	endif

	" get list of gdb variable buffer lines
	let s:var_lst = vimgdb#util#cmd_get_data_string("variable complete")
endfunction
