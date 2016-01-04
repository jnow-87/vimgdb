"""""""""""""""""""
" local variables "
"""""""""""""""""""

let s:initialised = 0

let s:cmd_dict = {
	\ "vimgdb":{
		\ "start":{},
		\ "stop":{},
		\ "direct":{"<mi-command>":{}},
		\ "export":{"__nested__":"vimgdb#complete#file"},
		\ "help":{},
		\ "__nested__":"vimgdb#complete#file",
	\ }
\ }



"""""""""""""""""""
" local functions "
"""""""""""""""""""

" \brief	vimgdb init and cleanup function
"
" \param	first		first argument
function s:vimgdb(first, ...)
	if a:first == "start"
		call s:init()

	elseif a:first == "stop"
		call s:cleanup()

	elseif a:first == "help"
		call s:help(join(a:000))

	elseif a:first == "direct"
		call vimgdb#util#cmd(join(a:000))

	elseif a:first == "export"
		if a:0 != 1
			echoerr "invalid argument, filename expected"
			return
		endif

		call s:export(a:1)

	else
		if filereadable(a:first) && match(a:first, ".*\.vim") == 0
			" source vim script
			exec "source " . a:first
		else
			call s:init()

			" execute default init
			exec "Inferior " . a:first
			exec "Inferior tty internal"

			if len(join(a:000)) > 0
				exec "Inferior args " . join(a:000)
			endif
		endif
	endif
endfunction

" \brief	initialise vimgdb plugin (windows, commands, vimgdb program)
function s:init()
	if s:initialised != 0
		return
	endif

	try
		let s:initialised = 1

		" configuration
		call vimgdb#config#init()
	
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
		call vimgdb#per#init()
	
		" start vimgdb
		if g:vimgdb_use_xterm
			exec "silent !xterm -geometry 130x20+0+0 -e '" . g:vimgdb_bin . " -l /dev/null -c \"" . g:vimgdb_gdb_cmd . "\" " . getcwd() . "' &"
		else
			exec "silent !" . g:vimgdb_bin . " -d -c \"" . g:vimgdb_gdb_cmd . "\" " . getcwd() . ""
		endif
	
		" start netbeans
		exec "nbstart :127.0.0.1:1235:"
	
		" init windows
		if vimgdb#window#initial(g:vimgdb_initial_name) != 0
			exec "nbclose"
			return
		endif
	
		" signal end of initialisation to vimgdb
		" XXX: do not use vimgdb#util#cmd() since it waits for a response
		"	   whereat vimgdb will not issue a response when receiving
		"	   'init-done'
		exec "nbkey init-done"
	
		call vimgdb#window#open(g:vimgdb_userlog_name, 0)
		call vimgdb#window#open(g:vimgdb_gdblog_name, 0)
		call vimgdb#window#open(g:vimgdb_callstack_name, 0)
		call vimgdb#window#open(g:vimgdb_memory_name, 0)
		call vimgdb#window#open(g:vimgdb_break_name, 0)
		call vimgdb#window#open(g:vimgdb_variables_name, 0)
		call vimgdb#window#open(g:vimgdb_inferior_name, 0)
		call vimgdb#window#open(g:vimgdb_register_name, 0)
		call vimgdb#window#open(g:vimgdb_per_name, 0)
	
		call vimgdb#window#focus(1)
	catch
		echom "error initialising vimgdb, performing cleanup"
		call s:cleanup()
	endtry
endfunction

" \brief	cleanup vimgdb plugin
function s:cleanup()
	if s:initialised == 0
		return
	endif

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
	call vimgdb#window#close(g:vimgdb_per_name)

	" close netbeans
	nbclose

	" cleanup
	call vimgdb#complete#cleanup(s:cmd_dict)
	call vimgdb#variable#cleanup()
	call vimgdb#exec#cleanup()
	call vimgdb#break#cleanup()
	call vimgdb#inferior#cleanup()
	call vimgdb#callstack#cleanup()
	call vimgdb#register#cleanup()
	call vimgdb#memory#cleanup()
	call vimgdb#evaluate#cleanup()
	call vimgdb#per#cleanup()
	call vimgdb#window#cleanup()
	call vimgdb#config#cleanup()

	let s:initialised = 0
endfunction

" \brief	export the current debug session
"
" \param	file	output file name
function s:export(file)
	let l:lst = [ 
		\ "let vimgdb_gdb_cmd = '" . g:vimgdb_gdb_cmd . "'", "",
	 	\ "Vimgdb start", "",
	\ ]

	" export inferior data
	let l:lst += vimgdb#util#cmd_get_data_list("inferior export")
	
	" export breakpoints
	let l:lst += vimgdb#util#cmd_get_data_list("break export")

	" export variables
	let l:lst += vimgdb#util#cmd_get_data_list("variable export")

	" export peripheral data
	let l:lst += vimgdb#util#cmd_get_data_list("per export")

	" write ouptut file
	let l:fullname = a:file

	if l:fullname[0] != "/"
		let l:fullname = getcwd() . "/" . a:file
	endif

	call writefile(l:lst, l:fullname)
endfunction

" \brief	vimgdb help command
"
" \param	line	item for which help is requested
function s:help(line)
	call vimgdb#window#open(g:vimgdb_userlog_name, 1)
	call vimgdb#util#cmd("help " . a:line)
endfunction


"""""""""""""""
" plugin init "
"""""""""""""""

" completion
call vimgdb#complete#init(s:cmd_dict)

" command
call vimgdb#util#execabbrev("vg", "Vimgdb")
call vimgdb#util#execabbrev("vimgdb", "Vimgdb")
command -nargs=* -complete=custom,vimgdb#complete#lookup Vimgdb call s:vimgdb(<f-args>)
