"""""""""""""""""""
" local variables "
"""""""""""""""""""

let s:cmd_dict = {
	\ "Variable":{
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
		\
		\ "set":{"__nested__":"vimgdb#variable#complete"},
		\ "view":{},
		\ "open":{},
	\ }
\ }

let s:var_lst = ""



""""""""""""""""""""
" global functions "
""""""""""""""""""""

" \brief	init variable command
function! vimgdb#variable#init()
	" update vimgdb completion
	call extend(g:vimgdb_cmd_dict, s:cmd_dict)

	" command
	command! -nargs=+ -complete=custom,vimgdb#complete#lookup Variable call s:variable(<f-args>)
	call vimgdb#util#execabbrev("variable", "Variable")

	" autocmd for variables window
	exec "autocmd! BufWinEnter " . g:vimgdb_variables_name . " silent
		\ setlocal noswapfile |
		\ setlocal noequalalways |
		\ setlocal bufhidden=delete |
		\ setlocal nowrap |
		\ setlocal syntax=vimgdb_variable |
		\ nnoremap <buffer> <c-f> :silent exec \"Variable fold \" . line('.')<cr>
		\ "
endfunction

" \brief	cleanup variable command
function! vimgdb#variable#cleanup()
	" command
	unabbrev variable
	delcommand Variable

	" autocmd
	exec "autocmd! BufWinEnter " . g:vimgdb_variables_name
endfunction

" \brief	complete vimgdb variable buffer lines
"
" \param	subcmd	current argument supplied in command line
function! vimgdb#variable#complete(subcmd)
	return s:var_lst
endfunction


"""""""""""""""""""
" local functions "
"""""""""""""""""""

" \brief	variable command implementation
function! s:variable(...)
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
	call delete("/tmp/vimgdb_variable")
	call vimgdb#util#cmd("variable get /tmp/vimgdb_variable")
	let s:var_lst = vimgdb#util#file_read("/tmp/vimgdb_variable", 5)
endfunction
