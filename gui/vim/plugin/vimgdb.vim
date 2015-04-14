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
function! s:vimgdb(state)
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
		call vimgdb#var#init()
		call vimgdb#exec#init()
		call vimgdb#break#init()
		call vimgdb#inferior#init()

		" start netbeans
		exec ":nbstart :127.0.0.1:1235:"

		" init windows
		call vimgdb#window#initial(g:vimgdb_initial_name)
		call vimgdb#window#open(g:vimgdb_userlog_name, g:vimgdb_userlog_width, g:vimgdb_userlog_height, g:vimgdb_userlog_vertical, g:vimgdb_userlog_show)
		call vimgdb#window#open(g:vimgdb_gdblog_name, g:vimgdb_gdblog_width, g:vimgdb_gdblog_height, g:vimgdb_gdblog_vertical, g:vimgdb_gdblog_show)
		call vimgdb#window#open(g:vimgdb_break_name, g:vimgdb_break_width, g:vimgdb_break_height, g:vimgdb_break_vertical, g:vimgdb_break_show)
		call vimgdb#window#open(g:vimgdb_variables_name, g:vimgdb_variables_width, g:vimgdb_variables_height, g:vimgdb_variables_vertical, g:vimgdb_variables_show)
		call vimgdb#window#open(g:vimgdb_inferior_name, g:vimgdb_inferior_width, g:vimgdb_inferior_height, g:vimgdb_inferior_vertical, g:vimgdb_inferior_show)

		call vimgdb#window#focus(1)

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
		call vimgdb#var#cleanup()
		call vimgdb#exec#cleanup()
		call vimgdb#break#cleanup()
		call vimgdb#inferior#cleanup()

		" close windows
		call vimgdb#window#close(g:vimgdb_initial_name)
		call vimgdb#window#close(g:vimgdb_userlog_name)
		call vimgdb#window#close(g:vimgdb_gdblog_name)
		call vimgdb#window#close(g:vimgdb_break_name)
		call vimgdb#window#close(g:vimgdb_variables_name)
		call vimgdb#window#close(g:vimgdb_inferior_name)

		let s:initialised = 0
	else
		echoerr "invalid state"
	endif
endfunction

" \brief	completion for vimgdb()
function! s:complete(arg, line, pos)
	return "start\nstop"
endfunction

""""""""""""
" commands "
""""""""""""

call vimgdb#util#execabbrev("vg", "Vimgdb")
command! -nargs=* -complete=custom,s:complete Vimgdb call s:vimgdb(<f-args>)
