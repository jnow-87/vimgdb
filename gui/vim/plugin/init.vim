" parameter
let gdblog_height = 10
let userlog_height = 10
let break_width = 40

let src_name = "source"
let gdblog_name = "gdb-log"
let userlog_name = "user-log"
let break_name = "breakpoints"
let inferior_name = "inferior"


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
function! s:buffer_split(name, relative, where, dimension, visible)
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

	if a:visible == 0
		exe "close"
	endif

	" return name
	return bufname(bufnr("%"))
endfunction


" open buffers
let src_name = s:buffer_initial(src_name)
call s:buffer_split(break_name, src_name, "vertical rightbelow", break_width, 1)
call s:buffer_split(inferior_name, break_name, "rightbelow", "", 0)
call s:buffer_split(gdblog_name, src_name, "rightbelow", gdblog_height, 1)
call s:buffer_split(userlog_name, src_name, "rightbelow", userlog_height, 1)

call s:buffer_focus(src_name)

" init netbeans socket
exec ":nbstart :127.0.0.1:1235:"

call s:buffer_focus(src_name)
exec "edit " . src_name
