" parameter
let s:gdblog_height = 10
let s:userlog_height = 10
let s:break_width = 40

let s:src_name = "source"
let s:gdblog_name = "gdb-log"
let s:userlog_name = "user-log"
let s:break_name = "breakpoints"
let s:inferior_name = "inferior"
let s:variable_name = "variables"

" open buffers when entering a tab
autocmd TabEnter * call s:prepare()

" preliminary mappings
nnoremap <silent> b :exec "nbkey break add " . fnamemodify(bufname('%'), ":t") . ":" . line('.')<cr>
nnoremap <silent> B :exec "nbkey break delete " . fnamemodify(bufname('%'), ":t") . ":" . line('.')<cr>
nnoremap <silent> <F2> :exec "nbkey exec step"<cr>
nnoremap <silent> <F3> :exec "nbkey exec next"<cr>
nnoremap <silent> <F5> :exec "nbkey exec return"<cr>
nnoremap <silent> <F6> :exec "nbkey exec run"<cr>
nnoremap <silent> <F7> :exec "nbkey exec continue"<cr>
nnoremap <silent> <F8> :exec "nbkey exec break"<cr>

"
" \brief	get name of source buffer
"
" \return	name of buffer to display sources
function! s:buffer_initial(name)
	" get name of current buffer
	let bname = bufname(bufnr("%"))

	" if current buffer is empty, create new one
	if bname == ""
		exec "edit " . a:name
		let bname = bufname(bufnr("%"))
	endif

	setlocal bufhidden=delete

	" return name
	return bname
endfunction

"
" \brief	make given buffer the active buffer
"
" \param	name	buffer name to focus
function! s:buffer_focus(name)
	exec bufwinnr(a:name) . " wincmd w"
endfunction

"
" \brief	create split
"
" \param	name		name of buffer to edit
" \param	relative	buffer name to open new buffer relative to
" \param	where		buffer location, e.g. verticl rightbelow
" \param	dimension	buffer width or height, depending on where
"
" \return	name of new buffer
function! s:buffer_split(name, relative, where, dimension, visible, syntax)
	" check if buffer is already visible
	if bufwinnr(a:name) != -1
		return bufname(bufnr("%"))
	endif

	" focus relative buffer
	if a:relative != ""
		call s:buffer_focus(a:relative)
	endif

	" split
	exe a:where . a:dimension . " split " . a:name

	" set buffer parameter
	setlocal noequalalways
	setlocal bufhidden=hide
	setlocal noswapfile
	exe "setlocal syntax=" . a:syntax

	if a:visible == 0
		exe "close"
	endif

	" return name
	return bufname(bufnr("%"))
endfunction

"
" \brief	open vimgdb buffers
function! s:prepare()
	let s:src_name = s:buffer_initial(s:src_name)
	call s:buffer_split(s:gdblog_name, s:src_name, "belowright", s:gdblog_height, 1, "")
	call s:buffer_split(s:userlog_name, s:src_name, "belowright", s:userlog_height, 1, "")
	call s:buffer_split(s:break_name, s:src_name, "vertical rightbelow", s:break_width, 1, "")
	call s:buffer_split(s:variable_name, s:break_name, "rightbelow", "", 1, "vimgdb_var")
	call s:buffer_split(s:inferior_name, s:break_name, "rightbelow", "", 1, "")

	call s:buffer_focus(s:src_name)
endfunction



" init buffers
call s:prepare()

" init netbeans socket
exec ":nbstart :127.0.0.1:1235:"

" trigger netbeans message
call s:buffer_focus(s:src_name)
exec "edit " . s:src_name

" wait until issueing commands
exec "sleep 500m"
exec "source " . expand('<sfile>:h') . "/cmd.vim"
