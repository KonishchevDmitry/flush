" Конфигурационный файл Vim IDE

" Запрещаем восстановление настроек из сессии,
" т. к. тогда при изменении ~/.vimrc даже после
" перезагрузки IDE новые настройки не будут
" вступать в силу.
set sessionoptions-=options

" Добавляем пути к библиотекам
set path+=/usr/include/gtkmm-2.4
"set path+=/usr/include/gtk-2.0
"set path+=./netlog

" Устанавливает правила синтаксиса, специфичные для данного проекта.
" -->
"    function! MySetIdeSyntax()
"        if getfsize(".vim/syntax.vim") >= 0
"            source .vim/syntax.vim
"        endif
"    endfunction
" <--

" Настраиваем работу с ctags -->
"    set tags=.vim/ctags
"
"    function! MyUpdateIdeCtags()
"        !make vim
"    endfunction
"
"    let MyUpdateCtagsFunction = "MyUpdateIdeCtags"
" Настраиваем работу с ctags <--

" Обновляем базу ctags при старте IDE
"call {MyUpdateCtagsFunction}()

" При открытии нового буфера устанавливаем для него
" правила синтаксиса, специфичные для данного проекта.
"au BufReadPost * :call MySetIdeSyntax()

" При закрытии Vim'а сохраняем информацию о текущей сессии
au VimLeave * :mksession! .vim/ide.session

" Загружаем ранее сохраненную сессию -->
    if getfsize(".vim/ide.session") >= 0
        source .vim/ide.session
    endif
" Загружаем ранее сохраненную сессию <--
