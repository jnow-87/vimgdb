" TODO: add comments to .vim files
" TODO: add key-map safe and restore

"""""""""""""""""""
" local variables "
"""""""""""""""""""

let s:initialised = 0


"""""""""""""""""""
" local functions "
"""""""""""""""""""

" \brief	vimgdb init and cleanup function
"
" \param	state	"start" | "stop"
function! s:vimgdb(state, ...)
	if a:state == "" || a:state == "start"
		if s:initialised != 0
			return
		endif

		" configuration
		call vimgdb#config#init()

		" completion
		call vimgdb#complete#init()

		" commands
		call vimgdb#window#init()
		call vimgdb#variable#init()
		call vimgdb#exec#init()
		call vimgdb#break#init()
		call vimgdb#inferior#init()
		call vimgdb#callstack#init()
		call vimgdb#register#init()
		call vimgdb#memory#init()
		call vimgdb#evaluate#init()

		" start netbeans
		exec ":nbstart :127.0.0.1:1235:"

		" init windows
		call vimgdb#window#initial(g:vimgdb_initial_name)
		call vimgdb#window#open(g:vimgdb_userlog_name, 0)
		call vimgdb#window#open(g:vimgdb_gdblog_name, 0)
		call vimgdb#window#open(g:vimgdb_callstack_name, 0)
		call vimgdb#window#open(g:vimgdb_memory_name, 0)
		call vimgdb#window#open(g:vimgdb_break_name, 0)
		call vimgdb#window#open(g:vimgdb_variables_name, 0)
		call vimgdb#window#open(g:vimgdb_inferior_name, 0)
		call vimgdb#window#open(g:vimgdb_register_name, 0)

		call vimgdb#window#focus(1)

		" signal end of initialisation to vimgdb
		call vimgdb#util#cmd("init-done")

		let s:initialised = 1

	elseif a:state == "stop"
		if s:initialised == 0
			return
		endif

		" close netbeans
		nbclose

		" cleanup
		call vimgdb#complete#cleanup()
		call vimgdb#window#cleanup()
		call vimgdb#variable#cleanup()
		call vimgdb#exec#cleanup()
		call vimgdb#break#cleanup()
		call vimgdb#inferior#cleanup()
		call vimgdb#callstack#cleanup()
		call vimgdb#register#cleanup()
		call vimgdb#memory#cleanup()
		call vimgdb#evaluate#cleanup()
		call vimgdb#config#cleanup()

		" close windows
		call vimgdb#window#close(g:vimgdb_initial_name)
		call vimgdb#window#close(g:vimgdb_userlog_name)
		call vimgdb#window#close(g:vimgdb_gdblog_name)
		call vimgdb#window#close(g:vimgdb_break_name)
		call vimgdb#window#close(g:vimgdb_variables_name)
		call vimgdb#window#close(g:vimgdb_inferior_name)
		call vimgdb#window#close(g:vimgdb_callstack_name)
		call vimgdb#window#close(g:vimgdb_register_name)
		call vimgdb#window#close(g:vimgdb_memory_name)

		let s:initialised = 0
	elseif a:state == "direct"
		call vimgdb#util#cmd(join(a:000))
	else
		echoerr "invalid state"
	endif
endfunction

" \brief	completion for vimgdb()
function! s:complete(arg, line, pos)
	return "start\nstop\ndirect"
endfunction

""""""""""""
" commands "
""""""""""""

call vimgdb#util#execabbrev("vg", "Vimgdb")
command! -nargs=* -complete=custom,s:complete Vimgdb call s:vimgdb(<f-args>)
