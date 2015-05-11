"""""""""""""""""""
" local variables "
"""""""""""""""""""

let s:cmd_dict = {
	\ "run":{},
	\ "next":{},
	\ "step":{},
	\ "return":{},
	\ "setpc":{"__nested__":"vimgdb#complete#sym_location"},
	\ "goto":{"__nested__":"vimgdb#complete#sym_location"},
	\ "continue":{},
	\ "int":{},
\ }


""""""""""""""""""""
" global functions "
""""""""""""""""""""

" \brief	init exec command
function! vimgdb#exec#init()
	" update vimgdb completion
	call vimgdb#complete#expand(s:cmd_dict, {'exec':s:cmd_dict}, {'exec':s:cmd_dict})

	" commands
	command! -nargs=0 -complete=custom,vimgdb#complete#lookup Run call s:exec("run", <f-args>)
	command! -nargs=0 -complete=custom,vimgdb#complete#lookup Nnext call s:exec("next", <f-args>)
	command! -nargs=0 -complete=custom,vimgdb#complete#lookup Step call s:exec("step", <f-args>)
	command! -nargs=0 -complete=custom,vimgdb#complete#lookup Return call s:exec("return", <f-args>)
	command! -nargs=+ -complete=custom,vimgdb#complete#lookup Setpc call s:exec("jump", <f-args>)
	command! -nargs=+ -complete=custom,vimgdb#complete#lookup Goto call s:exec("goto", <f-args>)
	command! -nargs=0 -complete=custom,vimgdb#complete#lookup Continue call s:exec("continue", <f-args>)
	command! -nargs=0 -complete=custom,vimgdb#complete#lookup Int call s:exec("break", <f-args>)

	call vimgdb#util#execabbrev("run", "Run")
	call vimgdb#util#execabbrev("next", "Nnext")
	call vimgdb#util#execabbrev("step", "Step")
	call vimgdb#util#execabbrev("return", "Return")
	call vimgdb#util#execabbrev("setpc", "Setpc")
	call vimgdb#util#execabbrev("goto", "Goto")
	call vimgdb#util#execabbrev("continue", "Continue")
	call vimgdb#util#execabbrev("int", "Int")
endfunction

" \brief	cleanup exec command
function! vimgdb#exec#cleanup()
	" rm commands
	unabbrev run
	unabbrev next
	unabbrev step
	unabbrev return
	unabbrev setpc
	unabbrev goto
	unabbrev continue
	unabbrev int

	delcommand Run
	delcommand Nnext
	delcommand Step
	delcommand Return
	delcommand Setpc
	delcommand Goto
	delcommand Continue
	delcommand Int
endfunction


"""""""""""""""""""
" local functions "
"""""""""""""""""""

" \brief exec command implementation
function! s:exec(cmd, ...)
	" focus first window, which is assumed to contain the source code buffers
	" avoiding messing up the window contents
	call vimgdb#window#focus(1)

	" exec vimgdb command
	call vimgdb#util#cmd("exec " . a:cmd . " " . join(a:000))
endfunction
