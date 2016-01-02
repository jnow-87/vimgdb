"""""""""""""""""""
" local variables "
"""""""""""""""""""

let s:cmd_dict = {
	\ "per":{
		\ "__nested__":"vimgdb#complete#file",
		\ "fold":{"__nested__":"vimgdb#per#complete_section"},
		\ "set":{"__nested__":"vimgdb#per#complete_regs"},
		\ "view":{},
		\ "open":{},
	\ }
\ }


""""""""""""""""""""
" global functions "
""""""""""""""""""""

" \brief	init per command
function vimgdb#per#init()
	" update vimgdb completion
	call vimgdb#complete#expand(s:cmd_dict, s:cmd_dict, s:cmd_dict)

	" command
	command! -nargs=+ -complete=custom,vimgdb#complete#lookup Per call s:per(<f-args>)
	call vimgdb#util#execabbrev("per", "Per")

	" autocmd for per window
	exec "autocmd! BufWinEnter " . g:vimgdb_per_name . " silent
		\ setlocal noswapfile |
		\ setlocal noequalalways |
		\ setlocal bufhidden=delete |
		\ setlocal nowrap |
		\ setlocal filetype=vimgdb_per |
		\ setlocal conceallevel=3 |
		\ setlocal concealcursor=nivc |
		\ nnoremap <buffer> <silent> <c-f> :exec 'Per fold ' . line('.')<cr>|
		\ nnoremap <buffer> <silent> + :exec 'Per fold ' . line('.')<cr>|
		\ nnoremap <buffer> <silent> - :exec 'Per fold ' . line('.')<cr>|
		\ nnoremap <buffer> <silent> u :exec 'Per view'<cr>|
		\ nnoremap <buffer> i :Per set <c-r>=substitute(split(getline('.'))[0], '[´`]h[0-9]', '', 'g')<cr> |
		\ nnoremap <buffer> s :Per set <c-r>=substitute(split(getline('.'))[0], '[´`]h[0-9]', '', 'g')<cr>
		\ "
endfunction

" \brief	cleanup per command
function vimgdb#per#cleanup()
	" command
	unabbrev per
	delcommand Per

	" autocmd
	exec "autocmd! BufWinEnter " . g:vimgdb_per_name
endfunction

" \brief	complete vimgdb per buffer lines with ranges
"
" \param	subcmd	current argument supplied in command line
function vimgdb#per#complete_section(subcmd)
	" get list of gdb per buffer lines
	let l:line_lst = split(vimgdb#util#cmd_get_data_string("per complete"), '<regs>')

	if len(l:line_lst) == 0
		return ""
	endif

	return l:line_lst[0]
endfunction

" \brief	complete vimgdb per register names
"
" \param	subcmd	current argument supplied in command line
function vimgdb#per#complete_regs(subcmd)
	" get list of gdb per buffer lines
	let l:line_lst = split(vimgdb#util#cmd_get_data_string("per complete"), '<regs>')

	if len(l:line_lst ) < 2
		return ""
	endif

	return l:line_lst[1]
endfunction



"""""""""""""""""""
" local functions "
"""""""""""""""""""

" \brief	per command implementation
function s:per(...)
	if a:1 == "open"
		call vimgdb#window#open(g:vimgdb_per_name, 1)
		call vimgdb#util#cmd("per view")

	elseif a:1 == "view"
		call vimgdb#window#view(g:vimgdb_per_name)
		call vimgdb#util#cmd("per view")

	else
		call vimgdb#window#open(g:vimgdb_per_name, 1)
		call vimgdb#util#cmd("per " . escape(join(a:000), '"'))
	endif
endfunction
