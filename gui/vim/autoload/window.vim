""""""""""""""""
" window names "
""""""""""""""""

let g:vimgdb_initial_name = "source"
let g:vimgdb_userlog_name = "user-log"
let g:vimgdb_gdblog_name = "gdb-log"
let g:vimgdb_break_name = "breakpoints"
let g:vimgdb_inferior_name = "inferior"
let g:vimgdb_variables_name = "variables"
let g:vimgdb_callstack_name = "callstack"
let g:vimgdb_register_name = "registers"
let g:vimgdb_memory_name = "memory"
let g:vimgdb_per_name = "peripherals"


"""""""""""""""""""
" local variables "
"""""""""""""""""""

let s:cmd_dict = {
	\ "window": {
		\ "view":{"__nested__":"vimgdb#window#complete"},
		\ "open":{"__nested__":"vimgdb#window#complete"},
		\ "close":{"__nested__":"vimgdb#window#complete"},
		\ "focus":{"__nested__":"vimgdb#window#complete"},
	\ }
\ }

let s:init_bname = ""


""""""""""""""""""""
" global functions "
""""""""""""""""""""

" \brief	init window command
function vimgdb#window#init()
	" init lists for vertical and horizontal windows
	let s:win_lst_vert = {}
	let s:win_lst_hor = {}

	" set window list
	let s:win_lst = {}

	let s:win_lst[g:vimgdb_userlog_name] = {
		\ "show":g:vimgdb_userlog_show,
		\ "width":g:vimgdb_userlog_width,
		\ "height":g:vimgdb_userlog_height,
		\ "vertical":g:vimgdb_userlog_vertical,
		\ "readonly":0,
		\ "preserve":0,
	\ }

	let s:win_lst[g:vimgdb_gdblog_name] = {
		\ "show":g:vimgdb_gdblog_show,
		\ "width":g:vimgdb_gdblog_width,
		\ "height":g:vimgdb_gdblog_height,
		\ "vertical":g:vimgdb_gdblog_vertical,
		\ "readonly":0,
		\ "preserve":0,
	\ }

	let s:win_lst[g:vimgdb_break_name] = {
		\ "show":g:vimgdb_break_show,
		\ "width":g:vimgdb_break_width,
		\ "height":g:vimgdb_break_height,
		\ "vertical":g:vimgdb_break_vertical,
		\ "readonly":1,
		\ "preserve":0,
	\ }

	let s:win_lst[g:vimgdb_inferior_name] = {
		\ "show":g:vimgdb_inferior_show,
		\ "width":g:vimgdb_inferior_width,
		\ "height":g:vimgdb_inferior_height,
		\ "vertical":g:vimgdb_inferior_vertical,
		\ "readonly":0,
		\ "preserve":0,
	\ }

	let s:win_lst[g:vimgdb_variables_name] = {
		\ "show":g:vimgdb_variables_show,
		\ "width":g:vimgdb_variables_width,
		\ "height":g:vimgdb_variables_height,
		\ "vertical":g:vimgdb_variables_vertical,
		\ "readonly":1,
		\ "preserve":1,
	\ }

	let s:win_lst[g:vimgdb_callstack_name] = {
		\ "show":g:vimgdb_callstack_show,
		\ "width":g:vimgdb_callstack_width,
		\ "height":g:vimgdb_callstack_height,
		\ "vertical":g:vimgdb_callstack_vertical,
		\ "readonly":1,
		\ "preserve":1,
	\ }

	let s:win_lst[g:vimgdb_register_name] = {
		\ "show":g:vimgdb_register_show,
		\ "width":g:vimgdb_register_width,
		\ "height":g:vimgdb_register_height,
		\ "vertical":g:vimgdb_register_vertical,
		\ "readonly":1,
		\ "preserve":1,
	\ }

	let s:win_lst[g:vimgdb_memory_name] = {
		\ "show":g:vimgdb_memory_show,
		\ "width":g:vimgdb_memory_width,
		\ "height":g:vimgdb_memory_height,
		\ "vertical":g:vimgdb_memory_vertical,
		\ "readonly":1,
		\ "preserve":1,
	\ }

	let s:win_lst[g:vimgdb_per_name] = {
		\ "show":g:vimgdb_per_show,
		\ "width":g:vimgdb_per_width,
		\ "height":g:vimgdb_per_height,
		\ "vertical":g:vimgdb_per_vertical,
		\ "readonly":1,
		\ "preserve":1,
	\ }


	" update vimgdb completio
	exec "let s:win_names = \""
		\ . g:vimgdb_userlog_name . "\n"
		\ . g:vimgdb_gdblog_name . "\n"
		\ . g:vimgdb_break_name . "\n"
		\ . g:vimgdb_inferior_name . "\n"
		\ . g:vimgdb_variables_name . "\n"
		\ . g:vimgdb_callstack_name . "\n"
		\ . g:vimgdb_register_name . "\n"
		\ . g:vimgdb_memory_name . "\n"
		\ . g:vimgdb_per_name . "\n"
		\ . "source"
		\ . "\""

	call vimgdb#complete#expand(s:cmd_dict, s:cmd_dict, s:cmd_dict)

	" command
	command -nargs=+ -complete=custom,vimgdb#complete#lookup Window call s:window(<f-args>)
	call vimgdb#util#execabbrev("window", "Window")

	" autocmd for user-log
	exec "autocmd BufWinEnter " . g:vimgdb_userlog_name . " silent
		\ setlocal noswapfile |
		\ setlocal noequalalways |
		\ setlocal bufhidden=delete |
		\ setlocal nowrap |
		\ setlocal filetype=vimgdb_userlog
		\ "

	" autocmd for gdb-log
	exec "autocmd BufWinEnter " . g:vimgdb_gdblog_name . " silent
		\ setlocal noswapfile |
		\ setlocal noequalalways |
		\ setlocal bufhidden=delete |
		\ setlocal nowrap |
		\ setlocal filetype=vimgdb_gdblog
		\ "
endfunction

" \brief	cleanup window command
function vimgdb#window#cleanup()
	" rm command
	unabbrev window
	delcommand Window

	" rm autocmd
	exec "autocmd! BufWinEnter " . g:vimgdb_userlog_name
	exec "autocmd! BufWinEnter " . g:vimgdb_gdblog_name

	" open initial buffer
	exec "edit! " . s:init_bname
endfunction

" \brief	window completion
function vimgdb#window#complete(subcmd)
	return s:win_names
endfunction

" \brief	initialise first window
"
" \param	name	default name of initial buffer
function vimgdb#window#initial(name)
	let s:init_bname = bufname(bufnr("%"))

	if s:init_bname == ""
		" if current buffer is empty, create new one
		exec "autocmd BufWinLeave " . a:name . " silent call vimgdb#window#close(\"" . a:name . "\")"

		exec "edit " . a:name
		setlocal bufhidden=delete
		setlocal noswapfile

		let l:bnr = bufnr(a:name)

	else
		if &modified == 1
			call vimgdb#util#error("buffer " . s:init_bname . " is modified, please safe changes and try again")
			return -1
		endif

		exec "edit! " . s:init_bname
		let l:bnr = bufnr("%")
	endif

	" add buffer to list
	let s:win_lst_hor[l:bnr] = 1

	" wait for vimgdb to assign id to new buffer, otherwise the
	" subsequent vimgdb#util#cmd() is not able to send the command
	sleep 100m

	return 0
endfunction

" \brief	open a window and add to window list
"
" \param	name		buffer name
" \param	force_open	overwrite visibility setting for window, forcing it to
"						be displayed
function vimgdb#window#open(name, force_open)
	" get window parameter
	if has_key(s:win_lst, a:name)
		let l:win = s:win_lst[a:name]
	else
		echoerr "window '". a:name . "' not in list"
		return
	endif

	" return if buffer is already visible or shall not be displayer
	if (l:win.show == 0 && a:force_open != 1) || bufwinnr(a:name) != -1
		return
	endif

	let l:relative = -1
	let l:split_arg = "rightbelow"

	if l:win.vertical == 1
		" for vertically split windows
		if len(s:win_lst_vert) > 0
			" focus the last of the vertically split windows
			let l:keys = keys(s:win_lst_vert)
			let l:relative = str2nr(l:keys[len(l:keys) - 1])
		else
			" make the window the first vertical window
			let l:split_arg = "vertical rightbelow" . l:win.width

			" focus the first horizontal window
			if len(s:win_lst_hor) > 0
				let l:keys = keys(s:win_lst_hor)
				let l:relative = str2nr(l:keys[0])
			endif
		endif
	else
		" for horizontally split windows
		let l:split_arg .= l:win.height

		if len(s:win_lst_hor) > 0
			" focus the first horizontal window
			let l:keys = keys(s:win_lst_hor)
			let l:relative = str2nr(l:keys[0])
		else
			" focus the first window, which is assumed to contain the source
			" code buffers
			Window focus source
			let s:win_lst_hor[bufnr("%")] = 1
			let l:relative = bufnr("%")
		endif
	endif

	" focus window specified
	if l:relative != -1
		call vimgdb#window#focus(bufwinnr(bufname(l:relative)))
	endif

	" split
	exe l:split_arg . " split"

	" update respective window list
	if l:win.vertical == 1
		let s:win_lst_vert[bufnr(a:name)] = 1
	else
		let s:win_lst_hor[bufnr(a:name)] = 1
	endif

	" view actual buffer
	call vimgdb#window#view(a:name)
endfunction

" \brief	open buffer in current window if not already displayed elsewhere
"
" \param	name	window name
function vimgdb#window#view(name)
	" return if buffer is already displayed
	if bufwinnr(a:name) != -1
		return
	endif

	if has_key(s:win_lst, a:name)
		let l:win = s:win_lst[a:name]
	else
		echoerr "window '". a:name . "' not in list"
		return
	endif

	" open buffer
	exec "edit " . a:name

	" set autocmd for window close
	exec "autocmd BufWinLeave " . a:name " silent call vimgdb#window#close(\"" . a:name . "\")"

	" wait for vimgdb to assign id to new buffer, otherwise the
	" subsequent vimgdb#util#cmd() is not able to send the command
	sleep 100m

	" set window readonly state
	if l:win.readonly
		call vimgdb#util#cmd('ui ' . a:name . ' ro 1')
	endif

	" set window preserve cursor state
	if l:win.preserve
		call vimgdb#util#cmd('ui ' . a:name . ' pc 1')
	endif
endfunction

" \brief	close a window and remove from window list
function vimgdb#window#close(name)
	let l:bnr = bufnr(a:name)
	let l:tgt_win = bufwinnr(l:bnr)

	" remove window from respectiv window list
	if has_key(s:win_lst_vert, l:bnr)
		call remove(s:win_lst_vert, l:bnr)
	elseif has_key(s:win_lst_hor, l:bnr)
		call remove(s:win_lst_hor, l:bnr)
	endif

	" remove autocmd
	exec "autocmd! BufWinLeave " . a:name

	" close window retaining buffer focus
	if l:tgt_win != -1
		let l:cur_win = winnr()

		call vimgdb#window#focus(l:tgt_win)
		quit

		if l:cur_win != l:tgt_win
			call vimgdb#window#focus(l:cur_win)
		endif
	endif
endfunction

" \brief	move cursor to window
"
" \param	winnr	window number to focus
function vimgdb#window#focus(winnr)
	exec a:winnr . " wincmd w"
endfunction

function vimgdb#window#open_src(line)
	" assumed format: <filename>:<line number>
	let l:lst = split(a:line, ' ')

	if l:lst != []
		let l:file = split(l:lst[0], ':')[0]
		let l:line = split(l:lst[0], ':')[1]

		" focus first window, which is assumed to be the source window
		Window focus source

		if bufwinnr(l:file) != 1
			" open file
			exec "edit " . l:file

			" wait for buffer to become available, otherwise jump to line
			" doesn't work
"			sleep 10m
		endif

		" jump to line
		exec ":" . l:line
	endif
endfunction


"""""""""""""""""""
" local functions "
"""""""""""""""""""

" \brief	window command implementation
"
" \param	cmd		window command (open, view, close)
" \param	name	window name
function s:window(cmd, name)
	if a:cmd == "open"
		call vimgdb#window#open(a:name, 1)

	elseif a:cmd == "view"
		call vimgdb#window#view(a:name)
	
	elseif a:cmd == "close"
		call vimgdb#window#close(a:name)

	elseif a:cmd == "focus"
		if a:name == "source"
			let tab_bufs = tabpagebuflist()

			" look for the first buffer in the current tab that contains a
			" file suffix (indicated by '.')
			let i = 1
			for bnum in tab_bufs
				if stridx(bufname(bnum), ".") != -1
					call vimgdb#window#focus(i)
					break
				endif

				let i += 1
			endfor
		else
			call vimgdb#window#focus(bufwinnr(a:name))
		endif
	endif
endfunction
