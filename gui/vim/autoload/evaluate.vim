"""""""""""""""""""
" local variables "
"""""""""""""""""""

let s:cmd_dict = {
	\ "Evaluate":{}
\ }


""""""""""""""""""""
" global functions "
""""""""""""""""""""

" \brief	init evaluate command
function! vimgdb#evaluate#init()
	" update vimgdb completion
	call extend(g:vimgdb_cmd_dict, s:cmd_dict)

	" command
	command! -nargs=+ -complete=custom,vimgdb#complete#lookup Evaluate call s:evaluate(<f-args>)
	call vimgdb#util#execabbrev("evaluate", "Evaluate")
endfunction

" \brief	cleanup memory command
function! vimgdb#memory#cleanup()
	" command
	unabbrev evaluate
	delcommand Evaluate
endfunction


"""""""""""""""""""
" local functions "
"""""""""""""""""""

" \brief	evaluate command implementation
function! s:evaluate(...)
	call vimgdb#window#open(g:vimgdb_userlog_name, 1)
	call vimgdb#util#cmd("evaluate " . join(a:000))
endfunction
