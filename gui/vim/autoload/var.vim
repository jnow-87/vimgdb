"""""""""""""""""""
" local variables "
"""""""""""""""""""

let s:cmd_dict = {
	\ "Var":{
		\ "add":{"__nested__":"vimgdb#complete#sym_var"},
		\ "delete":{"__nested__":"vimgdb#var#complete"},
		\ "fold":{"__nested__":"vimgdb#var#complete"},
		\ "set":{"__nested__":"vimgdb#var#complete"},
		\ "view":{},
	\ }
\ }

let s:var_lst = ""



""""""""""""""""""""
" global functions "
""""""""""""""""""""

" \brief	init variable command
function! vimgdb#var#init()
	" update vimgdb completion
	call extend(g:vimgdb_cmd_dict, s:cmd_dict)

	" command
	command! -nargs=+ -complete=custom,vimgdb#complete#lookup Var call s:var(<f-args>)
	call vimgdb#util#execabbrev("var", "Var")

	" autocmd for variables window
	exec "autocmd! BufWinEnter " . g:vimgdb_variables_name . " silent
		\ setlocal noswapfile |
		\ setlocal noequalalways |
		\ setlocal bufhidden=delete |
		\ setlocal syntax=vimgdb_var |
		\ nnoremap <c-f> :silent exec \"Var fold \" . line('.')<cr>
		\ "
endfunction

" \brief	cleanup variable command
function! vimgdb#var#cleanup()
	" command
	unabbrev var
	delcommand Var

	" autocmd
	exec "autocmd! BufWinEnter " . g:vimgdb_variables_name
endfunction

" \brief	complete vimgdb variable names
"
" \param	subcmd	current argument supplied in command line
function! vimgdb#var#complete(subcmd)
	return s:var_lst
endfunction


"""""""""""""""""""
" local functions "
"""""""""""""""""""

" \brief	variable command implementation
function! s:var(...)
	exec "Window open " . g:vimgdb_variables_name

	" wait for vimgdb to assign id to new buffer, otherwise the
	" subsequent vimgdb#util#cmd() is not able to send the command
	sleep 100m

	" exec vimgdb command
	call vimgdb#util#cmd("var " . join(a:000))

	" get list of gdb variable names
	call delete("/tmp/vimgdb_var")
	call vimgdb#util#cmd("var get /tmp/vimgdb_var")
	let s:var_lst = vimgdb#util#file_read("/tmp/vimgdb_var", 5)
endfunction

