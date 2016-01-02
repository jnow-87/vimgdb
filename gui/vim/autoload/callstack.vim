"""""""""""""""""""
" local variables "
"""""""""""""""""""

let s:cmd_dict = {
	\ "callstack":{
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
		\ "set":{
			\ "__nested__":"vimgdb#callstack#complete",
			\ "__nested1__":{"<value>":{}}
		\ },
		\ "view":{},
		\ "open":{},
	\ }
\ }


""""""""""""""""""""
" global functions "
""""""""""""""""""""

" \brief	init callstack command
function vimgdb#callstack#init()
	" update vimgdb completion
	call vimgdb#complete#expand(s:cmd_dict, s:cmd_dict, s:cmd_dict)

	" command
	command -nargs=+ -complete=custom,vimgdb#complete#lookup Callstack call s:callstack(<f-args>)
	call vimgdb#util#execabbrev("callstack", "Callstack")

	" autocmd for callstack window
	exec "autocmd BufWinEnter " . g:vimgdb_callstack_name . " silent
		\ setlocal noswapfile |
		\ setlocal noequalalways |
		\ setlocal bufhidden=delete |
		\ setlocal nowrap |
		\ setlocal filetype=vimgdb_variable |
		\ setlocal conceallevel=3 |
		\ setlocal concealcursor=nivc |
		\ nnoremap <buffer> <silent> <c-f> :exec 'Callstack fold ' . line('.')<cr>|
		\ nnoremap <buffer> <silent> + :exec 'Callstack fold ' . line('.')<cr>|
		\ nnoremap <buffer> <silent> - :exec 'Callstack fold ' . line('.')<cr>|
		\ nnoremap <buffer> <silent> u :exec 'Callstack view'<cr>|
		\ nnoremap <buffer> i :Callstack set <c-r>=line('.')<cr> |
		\ nnoremap <buffer> s :Callstack set <c-r>=line('.')<cr> |
		\ nnoremap <buffer> f :Callstack format <c-r>=line('.')<cr>
		\ "
endfunction

" \brief	cleanup callstack command
function vimgdb#callstack#cleanup()
	" command
	unabbrev callstack
	delcommand Callstack

	" autocmd
	exec "autocmd! BufWinEnter " . g:vimgdb_callstack_name
endfunction

" \brief	complete vimgdb callstack buffer lines
"
" \param	subcmd	current argument supplied in command line
function vimgdb#callstack#complete(subcmd)
	" get list of gdb callstack buffer lines
	return vimgdb#util#cmd_get_data_string("callstack complete")
endfunction


"""""""""""""""""""
" local functions "
"""""""""""""""""""

" \brief	callstack command implementation
function s:callstack(...)
	if a:1 == "open"
		call vimgdb#window#open(g:vimgdb_callstack_name, 1)
		call vimgdb#util#cmd("callstack view")

	elseif a:1 == "view"
		call vimgdb#window#view(g:vimgdb_callstack_name)
		call vimgdb#util#cmd("callstack view")

	else
		call vimgdb#window#open(g:vimgdb_callstack_name, 1)
		call vimgdb#util#cmd("callstack " . join(a:000))
	endif
endfunction
