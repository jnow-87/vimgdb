"""""""""""""""""""
" local variables "
"""""""""""""""""""

let s:cmd_dict = {
	\ "Memory":{
		\ "add":{},
		\ "delete":{},
		\ "set":{},
		\ "view":{},
		\ "open":{},
	\ }
\ }


""""""""""""""""""""
" global functions "
""""""""""""""""""""

" \brief	init memory command
function! vimgdb#memory#init()
	" update vimgdb completion
	call extend(g:vimgdb_cmd_dict, s:cmd_dict)

	" command
	command! -nargs=+ -complete=custom,vimgdb#complete#lookup Memory call s:memory(<f-args>)
	call vimgdb#util#execabbrev("memory", "Memory")

	" autocmd for memory window
	exec "autocmd! BufWinEnter " . g:vimgdb_memory_name . " silent
		\ setlocal noswapfile |
		\ setlocal noequalalways |
		\ setlocal bufhidden=delete |
		\ setlocal nowrap |
		\ setlocal syntax=vimgdb_memory
		\ "
endfunction

" \brief	cleanup memory command
function! vimgdb#memory#cleanup()
	" command
	unabbrev memory
	delcommand Memory

	" autocmd
	exec "autocmd! BufWinEnter " . g:vimgdb_memory_name
endfunction

" \brief	complete vimgdb memory buffer lines
"
" \param	subcmd	current argument supplied in command line
function! vimgdb#memory#complete(subcmd)
	" get list of gdb memory buffer lines
	call delete("/tmp/vimgdb_memory")
	call vimgdb#util#cmd("memory get /tmp/vimgdb_memory")
	let l:line_lst = vimgdb#util#file_read("/tmp/vimgdb_memory", 5)

	return l:line_lst
endfunction


"""""""""""""""""""
" local functions "
"""""""""""""""""""

" \brief	memory command implementation
function! s:memory(...)
	if a:1 == "open"
		call vimgdb#window#open(g:vimgdb_memory_name, 1)
		call vimgdb#util#cmd("memory view")

	elseif a:1 == "view"
		call vimgdb#window#view(g:vimgdb_memory_name)
		call vimgdb#util#cmd("memory view")

	else
		call vimgdb#window#open(g:vimgdb_memory_name, 1)
		call vimgdb#util#cmd("memory " . join(a:000))
	endif
endfunction
