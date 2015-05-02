"""""""""""""""""""
" local variables "
"""""""""""""""""""

let s:cmd_dict = {
	\ "Callstack":{
		\ "fold":{"__nested__":"vimgdb#callstack#complete"},
		\ "format":{
			\ "__nested__":"vimgdb#callstack#complete",
			\ "__nested1__":{
				\ "binary":{},
				\ "decimal":{},
				\ "hexadecimal":{},
				\ "octal":{},
				\ "natural":{},
			\ },
		\ },
		\
		\ "set":{"__nested__":"vimgdb#callstack#complete"},
		\ "view":{},
	\ }
\ }


""""""""""""""""""""
" global functions "
""""""""""""""""""""

" \brief	init callstack command
function! vimgdb#callstack#init()
	" update vimgdb completion
	call extend(g:vimgdb_cmd_dict, s:cmd_dict)

	" command
	command! -nargs=+ -complete=custom,vimgdb#complete#lookup Callstack call s:callstack(<f-args>)
	call vimgdb#util#execabbrev("callstack", "Callstack")

	" autocmd for callstack window
	exec "autocmd! BufWinEnter " . g:vimgdb_callstack_name . " silent
		\ setlocal noswapfile |
		\ setlocal noequalalways |
		\ setlocal bufhidden=delete |
		\ setlocal syntax=vimgdb_variable |
		\ nnoremap <buffer> <c-f> :silent exec \"Callstack fold \" . line('.')<cr>
		\ "
endfunction

" \brief	cleanup callstack command
function! vimgdb#callstack#cleanup()
	" command
	unabbrev callstack
	delcommand Callstack

	" autocmd
	exec "autocmd! BufWinEnter " . g:vimgdb_callstack_name
endfunction

" \brief	complete vimgdb callstack buffer lines
"
" \param	subcmd	current argument supplied in command line
function! vimgdb#callstack#complete(subcmd)
	" get list of gdb callstack buffer lines
	call delete("/tmp/vimgdb_callstack")
	call vimgdb#util#cmd("callstack get /tmp/vimgdb_callstack")
	let l:line_lst = vimgdb#util#file_read("/tmp/vimgdb_callstack", 5)

	return l:line_lst
endfunction


"""""""""""""""""""
" local functions "
"""""""""""""""""""

" \brief	callstack command implementation
function! s:callstack(...)
	exec "Window open " . g:vimgdb_callstack_name

	" wait for vimgdb to assign id to new buffer, otherwise the
	" subsequent vimgdb#util#cmd() is not able to send the command
	sleep 100m

	" exec vimgdb command
	call vimgdb#util#cmd("callstack " . join(a:000))
endfunction
