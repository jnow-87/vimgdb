""""""""""""""""""""
" highlight groups "
""""""""""""""""""""

if !hlexists("vimgdb_ok")
	highlight default vimgdb_ok	ctermfg=28
endif

if !hlexists("vimgdb_error")
	highlight default vimgdb_error ctermfg=1
endif

if !hlexists("vimgdb_warn")
	highlight default vimgdb_warn ctermfg=3
endif

if !hlexists("vimgdb_content_changed")
	highlight default vimgdb_content_changed ctermfg=202
endif

if !hlexists("vimgdb_mi_cmd")
	highlight default vimgdb_mi_cmd ctermfg=27
endif

if !hlexists("vimgdb_navigation")
	highlight default vimgdb_navigation ctermfg=56
endif

if !hlexists("vimgdb_callstack_filename")
	highlight default vimgdb_callstack_filename ctermfg=255
endif

if !hlexists("vimgdb_callstack_line")
	highlight default vimgdb_callstack_line ctermfg=1
endif

if !hlexists("vimgdb_callstack_function")
	highlight default vimgdb_callstack_function ctermfg=27
endif

if !hlexists("vimgdb_memory_unknown")
	highlight default vimgdb_memory_unknown ctermfg=88
endif

if !hlexists("vimgdb_memory_heading")
	highlight default vimgdb_memory_heading ctermfg=255 cterm=bold,underline
endif

if !hlexists("vimgdb_memory_addr")
	highlight default vimgdb_memory_addr ctermfg=255 cterm=bold
endif

if !hlexists("vimgdb_memory_ascii")
	highlight default vimgdb_memory_ascii ctermfg=8
endif

if !hlexists("vimgdb_per_section")
	highlight default vimgdb_per_section ctermfg=255 cterm=bold,underline
endif

if !hlexists("vimgdb_per_heading")
	highlight default vimgdb_per_heading ctermfg=255 cterm=bold
endif

if !hlexists("vimgdb_per_register")
	highlight default vimgdb_per_register ctermfg=8 cterm=bold
endif

if !hlexists("vimgdb_per_bit")
	highlight default vimgdb_per_bit ctermfg=8
endif


""""""""""""""""""
" user-variables "
"""""""""""""""""" 

call vimgdb#util#cond_assign("g:vimgdb_bin", "vimgdb")
call vimgdb#util#cond_assign("g:vimgdb_gdb_cmd", "gdb -q")
call vimgdb#util#cond_assign("g:vimgdb_use_xterm", 0)

call vimgdb#util#cond_assign("g:vimgdb_userlog_show", 1)
call vimgdb#util#cond_assign("g:vimgdb_userlog_width", 40)
call vimgdb#util#cond_assign("g:vimgdb_userlog_height", 10)
call vimgdb#util#cond_assign("g:vimgdb_userlog_vertical", 0)

call vimgdb#util#cond_assign("g:vimgdb_gdblog_show", 1)
call vimgdb#util#cond_assign("g:vimgdb_gdblog_width", 40)
call vimgdb#util#cond_assign("g:vimgdb_gdblog_height", 10)
call vimgdb#util#cond_assign("g:vimgdb_gdblog_vertical", 0)

call vimgdb#util#cond_assign("g:vimgdb_break_show", 0)
call vimgdb#util#cond_assign("g:vimgdb_break_width", 40)
call vimgdb#util#cond_assign("g:vimgdb_break_height", 10)
call vimgdb#util#cond_assign("g:vimgdb_break_vertical", 1)

call vimgdb#util#cond_assign("g:vimgdb_inferior_show", 1)
call vimgdb#util#cond_assign("g:vimgdb_inferior_width", 40)
call vimgdb#util#cond_assign("g:vimgdb_inferior_height", 10)
call vimgdb#util#cond_assign("g:vimgdb_inferior_vertical", 1)

call vimgdb#util#cond_assign("g:vimgdb_variables_show", 0)
call vimgdb#util#cond_assign("g:vimgdb_variables_width", 40)
call vimgdb#util#cond_assign("g:vimgdb_variables_height", 10)
call vimgdb#util#cond_assign("g:vimgdb_variables_vertical", 1)

call vimgdb#util#cond_assign("g:vimgdb_callstack_show", 1)
call vimgdb#util#cond_assign("g:vimgdb_callstack_width", 40)
call vimgdb#util#cond_assign("g:vimgdb_callstack_height", 10)
call vimgdb#util#cond_assign("g:vimgdb_callstack_vertical", 0)

call vimgdb#util#cond_assign("g:vimgdb_register_show", 0)
call vimgdb#util#cond_assign("g:vimgdb_register_width", 40)
call vimgdb#util#cond_assign("g:vimgdb_register_height", 10)
call vimgdb#util#cond_assign("g:vimgdb_register_vertical", 1)

call vimgdb#util#cond_assign("g:vimgdb_memory_show", 0)
call vimgdb#util#cond_assign("g:vimgdb_memory_width", 40)
call vimgdb#util#cond_assign("g:vimgdb_memory_height", 10)
call vimgdb#util#cond_assign("g:vimgdb_memory_vertical", 0)

call vimgdb#util#cond_assign("g:vimgdb_per_show", 0)
call vimgdb#util#cond_assign("g:vimgdb_per_width", 40)
call vimgdb#util#cond_assign("g:vimgdb_per_height", 10)
call vimgdb#util#cond_assign("g:vimgdb_per_vertical", 1)


"""""""""""""""""""
" local variables "
"""""""""""""""""""

let s:keymap = {}


""""""""""""""""""""
" global functions "
""""""""""""""""""""

" \brief	apply configuration
function vimgdb#config#init()
	" assign key mappings
	call s:map_key("b", "n", ":exec 'Break add ' . bufname('%') . ':' . line('.')<cr>")
	call s:map_key("B", "n", ":exec 'Break delete ' . bufname('%') . ':' . line('.')<cr>")
	call s:map_key("e", "n", ":exec 'Break enable ' . bufname('%') . ':' . line('.')<cr>")
	call s:map_key("E", "n", ":exec 'Break disable ' . bufname('%') . ':' . line('.')<cr>")
	call s:map_key("j", "n", ":silent exec 'Setpc ' . fnamemodify(bufname('%'), ':t') . ':' . line('.')<cr>")
	call s:map_key("s", "n", ":silent exec 'Setpc ' . fnamemodify(bufname('%'), ':t') . ':' . line('.')<cr>")
	call s:map_key("g", "n", ":silent exec 'Goto ' . fnamemodify(bufname('%'), ':t') . ':' . line('.')<cr>")
	call s:map_key("c", "n", ":silent exec 'Goto ' . fnamemodify(bufname('%'), ':t') . ':' . line('.')<cr>")
	call s:map_key("<F2>", "n", ":silent call g:filetype_exec('[sS]', 'Stepi', 'Step')<cr>")
	call s:map_key("<F3>", "n", ":silent call g:filetype_exec('[sS]', 'Nnexti', 'Nnext')<cr>")
	call s:map_key("<F5>", "n", ":silent Return<cr>")
	call s:map_key("<F6>", "n", ":silent Run<cr>")
	call s:map_key("<F7>", "n", ":silent Continue<cr>")
	call s:map_key("<F8>", "n", ":silent Int<cr>")
endfunction

" \brief	cleanup config
function! vimgdb#config#cleanup()
	" restore key mappings
	for l:key in keys(s:keymap)
		let l:val = s:keymap[l:key]

		let l:restore = l:val['restore']
		exec l:val['mode'] . "unmap " . l:key

		if l:restore != {}
			exec l:restore['mode']
				\ . (l:restore['noremap'] == 1 ? "noremap " : "map ")
				\ . (l:restore['silent'] == 1 ? "<silent> " : "")
				\ . (l:restore['nowait'] == 1 ? "<nowait> " : "")
				\ . (l:restore['expr'] == 1 ? "<expr> " : "")
				\ . (l:restore['buffer'] == 1 ? "<buffer> " : "")
				\ . l:restore['lhs'] . " " . l:restore['rhs']
			let s:keymap[l:key]['restore'] = {}
		endif
	endfor
endfunction

" \brief	exec commands depending on filetype
"
" \param	ftype_pattern	filetypes to execute command for
" \param	true			command to execute on match
" \param	false			command to execute on missmatch
function! g:filetype_exec(ftype_pattern, true, false)
	if match(fnamemodify(bufname('%'), ':e'), a:ftype_pattern) == -1
		exec a:false
	else
		exec a:true
	endif
endfunction


"""""""""""""""""""
" local functions "
"""""""""""""""""""

" \brief	create key map
"
" \param	lhs		left-hand-side
" \param	mode	mode to use, e.g. "n", "i", ...
" \param	rhs		right-hand-side
function! s:map_key(lhs, mode, rhs)
	" create entry in s:keymap if it doesn't exist
	if !has_key(s:keymap, a:lhs)
		let s:keymap[a:lhs] = {}
	endif

	" store mode and current map
	let s:keymap[a:lhs]['mode'] = a:mode
	let s:keymap[a:lhs]['restore'] = maparg(a:lhs, a:mode, 0, 1)

	" apply new map
	exec a:mode . "noremap <silent>" . a:lhs . " " . a:rhs
endfunction
