"""""""""""""""""""
" local variables "
"""""""""""""""""""

let s:cmd_dict = {
	\ "Window": {
		\ "view":{"__nested__":"vimgdb#window#complete"},
		\ "open":{"__nested__":"vimgdb#window#complete"},
		\ "close":{"__nested__":"vimgdb#window#complete"},
	\ }
\ }


""""""""""""""""""""
" global functions "
""""""""""""""""""""

" \brief	init window command
function! vimgdb#window#init()
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
	\ }

	let s:win_lst[g:vimgdb_gdblog_name] = {
		\ "show":g:vimgdb_gdblog_show,
		\ "width":g:vimgdb_gdblog_width,
		\ "height":g:vimgdb_gdblog_height,
		\ "vertical":g:vimgdb_gdblog_vertical,
	\ }

	let s:win_lst[g:vimgdb_break_name] = {
		\ "show":g:vimgdb_break_show,
		\ "width":g:vimgdb_break_width,
		\ "height":g:vimgdb_break_height,
		\ "vertical":g:vimgdb_break_vertical,
	\ }

	let s:win_lst[g:vimgdb_inferior_name] = {
		\ "show":g:vimgdb_inferior_show,
		\ "width":g:vimgdb_inferior_width,
		\ "height":g:vimgdb_inferior_height,
		\ "vertical":g:vimgdb_inferior_vertical,
	\ }

	let s:win_lst[g:vimgdb_variables_name] = {
		\ "show":g:vimgdb_variables_show,
		\ "width":g:vimgdb_variables_width,
		\ "height":g:vimgdb_variables_height,
		\ "vertical":g:vimgdb_variables_vertical,
	\ }

	let s:win_lst[g:vimgdb_callstack_name] = {
		\ "show":g:vimgdb_callstack_show,
		\ "width":g:vimgdb_callstack_width,
		\ "height":g:vimgdb_callstack_height,
		\ "vertical":g:vimgdb_callstack_vertical,
	\ }

	" update vimgdb completio
	exec "let s:win_names = \"" . g:vimgdb_userlog_name . "\n" . g:vimgdb_gdblog_name . "\n" . g:vimgdb_break_name . "\n" . g:vimgdb_inferior_name . "\n" . g:vimgdb_variables_name . "\""

	call extend(g:vimgdb_cmd_dict, s:cmd_dict)

	" command
	command! -nargs=+ -complete=custom,vimgdb#complete#lookup Window call s:window(<f-args>)
	call vimgdb#util#execabbrev("win", "Window")

	" autocmd for user-log
	exec "autocmd! BufWinEnter " . g:vimgdb_userlog_name . " silent
		\ setlocal noswapfile |
		\ setlocal noequalalways |
		\ setlocal bufhidden=delete
		\ "

	" autocmd for gdb-log
	exec "autocmd! BufWinEnter " . g:vimgdb_gdblog_name . " silent
		\ setlocal noswapfile |
		\ setlocal noequalalways |
		\ setlocal bufhidden=delete
		\ "
endfunction

" \brief	cleanup window command
function! vimgdb#window#cleanup()
	" rm command
	unabbrev win
	delcommand Window

	" rm autocmd
	exec "autocmd! BufWinEnter " . g:vimgdb_userlog_name
	exec "autocmd! BufWinEnter " . g:vimgdb_gdblog_name
endfunction

" \brief	window completion
function! vimgdb#window#complete(subcmd)
	return s:win_names
endfunction

" \brief	initialise first window
"
" \param	name	default name of initial buffer
function! vimgdb#window#initial(name)
	if bufname(bufnr("%")) == ""
		" if current buffer is empty, create new one
		setlocal bufhidden=delete
		exec "autocmd! BufWinLeave " . a:name . " silent call vimgdb#window#close(\"" . a:name . "\")"

		exec "edit " . a:name
		let l:bnr = bufnr(a:name)
	else
		let l:bnr = bufnr("%")
	endif

	" add buffer to list
	let s:win_lst_hor[l:bnr] = 1
endfunction

" \brief	open a window and add to window list
"
" \param	name		buffer name
" \param	width		window width (only applies for vertically split windows)
" \param	height		window height (only applies for horizontally split windows)
" \param	vertical	specify if window shall be horizontal (0) or vertical (1)
" \param	visible		specify if window shall be displayed
function! vimgdb#window#open(name, width, height, vertical, visible)
	" return if buffer is already visible or shall not be displayer
	if a:visible == 0 || bufwinnr(a:name) != -1
		return
	endif

	let l:relative = -1
	let l:split_arg = "rightbelow"

	if a:vertical == 1
		" for vertically split windows
		if len(s:win_lst_vert) > 0
			" focus the last of the vertically split windows
			let l:keys = keys(s:win_lst_vert)
			let l:relative = str2nr(l:keys[len(l:keys) - 1])
		else
			" make the window the first vertical window
			let l:split_arg = "vertical rightbelow" . a:width

			" focus the first horizontal window
			if len(s:win_lst_hor) > 0
				let l:keys = keys(s:win_lst_hor)
				let l:relative = str2nr(l:keys[0])
			endif
		endif
	else
		" for horizontally split windows
		let l:split_arg .= a:height

		if len(s:win_lst_hor) > 0
			" focus the first horizontal window
			let l:keys = keys(s:win_lst_hor)
			let l:relative = str2nr(l:keys[0])
		else
			" focus the first window, which is assumed to contain the source
			" code buffers
			call vimgdb#window#focus(1)
			let s:win_lst_hor[bufnr("%")] = 1
			let l:relative = bufnr("%")
		endif
	endif

	" focus window specified
	if l:relative != -1
		call vimgdb#window#focus(bufwinnr(bufname(l:relative)))
	endif

	" split
	exe l:split_arg . " split " . a:name

	" update respective window list
	if a:vertical == 1
		let s:win_lst_vert[bufnr(a:name)] = 1
	else
		let s:win_lst_hor[bufnr(a:name)] = 1
	endif

	" set autocmd for window close
	exec "autocmd! BufWinLeave " . a:name " silent call vimgdb#window#close(\"" . a:name . "\")"
endfunction

" \brief	close a window and remove from window list
function! vimgdb#window#close(name)
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

		call vimgdb#window#focus(l:cur_win)
	endif
endfunction

" \brief	move cursor to window
"
" \param	winnr	window number to focus
function! vimgdb#window#focus(winnr)
	exec a:winnr . " wincmd w"
endfunction


"""""""""""""""""""
" local functions "
"""""""""""""""""""

" \brief	window command implementation
"
" \param	cmd		window command (open, view, close)
" \param	name	window name
function! s:window(cmd, name)
	if a:cmd == "view" || a:cmd == "open"
		if has_key(s:win_lst, a:name)
			let l:win = s:win_lst[a:name]
			call vimgdb#window#open(a:name, l:win.width, l:win.height, l:win.vertical, 1)
		endif
	elseif a:cmd == "close"
		call vimgdb#window#close(a:name)
	endif
endfunction
