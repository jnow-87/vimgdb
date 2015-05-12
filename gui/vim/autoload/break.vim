"""""""""""""""""""
" local variables "
"""""""""""""""""""

let s:cmd_dict = {
	\ "break": {
		\ "add":{
			\ "__nested__":"vimgdb#complete#sym_location",
			\ "-i":{"<count>":{}},
			\ "-c":{"<condition>":{}},
			\ "-t":{},
			\ "-h":{},
		\ },
		\ "delete":{"__nested__":"vimgdb#break#complete_bkpt"},
		\ "enable":{"__nested__":"vimgdb#break#complete_bkpt"},
		\ "disable":{"__nested__":"vimgdb#break#complete_bkpt"},
		\ "export":{"__nested__":"vimgdb#complete#file"},
		\ "view":{},
		\ "open":{},
	\ }
\ }

let s:cmd_dict["break"]["add"]["-i"]["<count>"] = s:cmd_dict["break"]["add"]
let s:cmd_dict["break"]["add"]["-c"]["<condition>"] = s:cmd_dict["break"]["add"]
let s:cmd_dict["break"]["add"]["-t"] = s:cmd_dict["break"]["add"]
let s:cmd_dict["break"]["add"]["-h"] = s:cmd_dict["break"]["add"]

let s:breakpt_lst = ""


""""""""""""""""""""
" global functions "
""""""""""""""""""""

" \brief	init breakpoint command
function! vimgdb#break#init()
	" update vimgdb completion
	call vimgdb#complete#expand(s:cmd_dict, s:cmd_dict, s:cmd_dict)

	" command
	command! -nargs=+ -complete=custom,vimgdb#complete#lookup Break call s:break(<f-args>)
	call vimgdb#util#execabbrev("break", "Break")

	" autocmd for breakpoint window
	exec "autocmd! BufWinEnter " . g:vimgdb_break_name . " silent
		\ setlocal noswapfile |
		\ setlocal noequalalways |
		\ setlocal bufhidden=delete |
		\ setlocal nowrap |
		\ nnoremap <buffer> <silent> e :exec 'Break enable ' . getline('.')<cr>|
		\ nnoremap <buffer> <silent> E :exec 'Break disable ' . getline('.')<cr>|
		\ nnoremap <buffer> <silent> dd :exec 'Break delete ' . getline('.')<cr>|
		\ nnoremap <buffer> <silent> u :exec 'Break view'<cr>|
		\ nnoremap <buffer> <silent> <return> :call vimgdb#window#open_src(getline('.'))<cr>
		\ "
endfunction

" \brief	cleanup breakpoint command
function! vimgdb#break#cleanup()
	" rm command
	unabbrev break
	delcommand Break

	" rm autocmd
	exec "autocmd! BufWinEnter " . g:vimgdb_break_name
endfunction

" \brief	breakpoint completion
"
" \param	subcmd	current argument supplied in command line
function! vimgdb#break#complete_bkpt(subcmd)
	return s:breakpt_lst
endfunction


"""""""""""""""""""
" local functions "
"""""""""""""""""""

" \brief	breakpoint command implementation
"
" \param	...		argument list as required by vimgdb
function! s:break(...)
	if a:1 == "open"
		call vimgdb#window#open(g:vimgdb_break_name, 1)
		call vimgdb#util#cmd("break view")

	elseif a:1 == "view"
		call vimgdb#window#view(g:vimgdb_break_name)
		call vimgdb#util#cmd("break view")

	else
		" exec vimgdb command
		call vimgdb#util#cmd("break " . join(a:000))
	endif

	" ask vimgdb for breakpoint list
	call delete("/tmp/vimgdb_break")
	call vimgdb#util#cmd("break complete /tmp/vimgdb_break")
	let s:breakpt_lst = vimgdb#util#file_read("/tmp/vimgdb_break", 5)
endfunction
