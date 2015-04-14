""""""""""""""""""""
" global functions "
""""""""""""""""""""

" \brief	apply configuration
function vimgdb#config#init()
	" parameter
	let g:vimgdb_initial_name = "source"

	let g:vimgdb_userlog_name = "user-log"
	call vimgdb#util#assign("g:vimgdb_userlog_show", 1)
	call vimgdb#util#assign("g:vimgdb_userlog_width", 40)
	call vimgdb#util#assign("g:vimgdb_userlog_height", 10)
	call vimgdb#util#assign("g:vimgdb_userlog_vertical", 0)

	let g:vimgdb_gdblog_name = "gdb-log"
	call vimgdb#util#assign("g:vimgdb_gdblog_show", 1)
	call vimgdb#util#assign("g:vimgdb_gdblog_width", 40)
	call vimgdb#util#assign("g:vimgdb_gdblog_height", 10)
	call vimgdb#util#assign("g:vimgdb_gdblog_vertical", 0)

	let g:vimgdb_break_name = "breakpoints"
	call vimgdb#util#assign("g:vimgdb_break_show", 0)
	call vimgdb#util#assign("g:vimgdb_break_width", 40)
	call vimgdb#util#assign("g:vimgdb_break_height", 10)
	call vimgdb#util#assign("g:vimgdb_break_vertical", 1)

	let g:vimgdb_inferior_name = "inferior"
	call vimgdb#util#assign("g:vimgdb_inferior_show", 1)
	call vimgdb#util#assign("g:vimgdb_inferior_width", 40)
	call vimgdb#util#assign("g:vimgdb_inferior_height", 10)
	call vimgdb#util#assign("g:vimgdb_inferior_vertical", 1)

	let g:vimgdb_variables_name = "variables"
	call vimgdb#util#assign("g:vimgdb_variables_show", 0)
	call vimgdb#util#assign("g:vimgdb_variables_width", 40)
	call vimgdb#util#assign("g:vimgdb_variables_height", 10)
	call vimgdb#util#assign("g:vimgdb_variables_vertical", 1)

	" preliminary mappings
	nnoremap <silent> b :exec "Break add " . fnamemodify(bufname('%'), ":t") . ":" . line('.')<cr>
	nnoremap <silent> B :exec "Break delete " . fnamemodify(bufname('%'), ":t") . ":" . line('.')<cr>
	nnoremap <silent> <F2> :silent Step<cr>
	nnoremap <silent> <F3> :silent Nnext<cr>
	nnoremap <silent> <F5> :silent Return<cr>
	nnoremap <silent> <F6> :silent Run<cr>
	nnoremap <silent> <F7> :silent Continue<cr>
	nnoremap <silent> <F8> :silent Int<cr>
endfunction
