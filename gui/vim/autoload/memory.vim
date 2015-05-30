"""""""""""""""""""
" local variables "
"""""""""""""""""""

let s:cmd_dict = {
	\ "memory":{
		\ "add":{
			\ "<addr>":{
				\ "<bytes>":{}
			\ }
		\ },
		\ "delete":{"__nested__":"vimgdb#memory#complete_lines"},
		\ "set":{
			\ "__nested__":"vimgdb#memory#complete_addr",
			\ "__nested1__":{
				\ "<value>":{
					\ "<count>":{}
				\ }
			\ }
		\ },
		\ "fold":{"__nested__":"vimgdb#memory#complete_lines"},
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
	call vimgdb#complete#expand(s:cmd_dict, s:cmd_dict, s:cmd_dict)

	" command
	command! -nargs=+ -complete=custom,vimgdb#complete#lookup Memory call s:memory(<f-args>)
	call vimgdb#util#execabbrev("memory", "Memory")

	" autocmd for memory window
	exec "autocmd! BufWinEnter " . g:vimgdb_memory_name . " silent
		\ setlocal noswapfile |
		\ setlocal noequalalways |
		\ setlocal bufhidden=delete |
		\ setlocal nowrap |
		\ setlocal syntax=vimgdb_memory |
		\ setlocal conceallevel=3 |
		\ setlocal concealcursor=nivc |
		\ nnoremap <buffer> <silent> <c-f> :exec 'Memory fold ' . line('.')<cr>|
		\ nnoremap <buffer> <silent> + :exec 'Memory fold ' . line('.')<cr>|
		\ nnoremap <buffer> <silent> - :exec 'Memory fold ' . line('.')<cr>|
		\ nnoremap <buffer> <silent> dd :exec 'Memory delete ' . line('.')<cr>|
		\ nnoremap <buffer> <silent> u :exec 'Memory view'<cr>|
		\ nnoremap <buffer> i :Memory set <c-r>=split(getline('.'))[0]<cr>|
		\ nnoremap <buffer> s :Memory set <c-r>=split(getline('.'))[0]<cr>
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
function! vimgdb#memory#complete_lines(subcmd)
	" get list of gdb memory buffer lines
	let l:line_lst = split(vimgdb#util#cmd_get_data_string("memory complete"), '<addr>')

	if l:line_lst == []
		return ""
	endif

	return l:line_lst[0]
endfunction

" \brief	complete vimgdb memory addresses
"
" \param	subcmd	current argument supplied in command line
function! vimgdb#memory#complete_addr(subcmd)
	" get list of gdb memory buffer lines
	let l:line_lst = split(vimgdb#util#cmd_get_data_string("memory complete"), '<addr>')

	if l:line_lst == []
		return ""
	endif

	return l:line_lst[1]
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
