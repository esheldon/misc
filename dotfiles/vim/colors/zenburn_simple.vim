" Vim color file
" Maintainer:   Jani Nurminen <jani.nurminen@pp4.inet.fi>
" Last Change:  $Id: zenburn.vim,v 1.15 2006/09/07 15:57:46 jnurmine Exp $
" URL:      	http://slinky.imukuppi.org/zenburn/
" License:      GPL
"
" Nothing too fancy, just some alien fruit salad to keep you in the zone.
" This syntax file was designed to be used with dark environments and 
" low light situations. Of course, if it works during a daybright office, go
" ahead :)
"
" Owes heavily to other Vim color files! With special mentions
" to "BlackDust", "Camo" and "Desert".
"
" To install, copy to ~/.vim/colors directory. Then :colorscheme zenburn.  
" See also :help syntax
"
" Credits:
"  - Jani Nurminen - original Zenburn
"  - Steve Hall & Cream posse - higher-contrast Visual selection
"  - Kurt Maier - 256 color console coloring, low and high contrast toggle,
"                 bug fixing
"
" CONFIGURABLE PARAMETERS:
" 
" You can use the default (don't set any parameters), or you can
" set some parameters to tweak the Zenburn colours.
"
" * You can now set a darker background for bright environments. To activate, 
" use:
"   contrast Zenburn, use:
"
"      let g:zenburn_high_Contrast = 1
"
" * To get more contrast to the Visual selection, use
"   
"      let g:zenburn_alternate_Visual = 1
" 
" * To use alternate colouring for Error message, use
"     
"      let g:zenburn_alternate_Error = 1
"
" * The new default for Include is a duller orange. To use the original
"   colouring for Include, use
"     
"      let g:zenburn_alternate_Include = 1
"
" * To turn the parameter(s) back to defaults, use UNLET:
"
"      unlet g:zenburn_alternate_Include
"
"   Setting to 0 won't work!
"
" That's it, enjoy!
" 
" TODO
"   - Visual alternate color is broken? Try GVim >= 7.0.66 if you have trouble
"   - IME colouring (CursorIM)
"   - obscure syntax groups: check and colourize
"   - add more groups if necessary

set background=dark
hi clear          
if exists("syntax_on")
    syntax reset
endif
let g:colors_name="zenburn"

hi Boolean         guifg=#dca3a3
hi Character       guifg=#dca3a3
hi Comment         guifg=#7f9f7f
hi Conditional     guifg=#f0dfaf
hi Constant        guifg=#dca3a3
hi Cursor          guifg=#000d18 guibg=#8faf9f
hi Debug           guifg=#bca3a3
hi Define          guifg=#ffcfaf
hi Delimiter       guifg=#8f8f8f
hi DiffAdd         guifg=#709080 guibg=#313c36
hi DiffChange      guibg=#333333
hi DiffDelete      guifg=#333333 guibg=#464646
hi DiffText        guifg=#ecbcbc guibg=#41363c
hi Directory       guifg=#dcdccc
hi ErrorMsg        guifg=#80d4aa guibg=#2f2f2f
hi Exception       guifg=#c3bf9f
hi Float           guifg=#c0bed1
hi FoldColumn      guifg=#93b3a3 guibg=#3f4040
hi Folded          guifg=#93b3a3 guibg=#3f4040
hi Function        guifg=#efef8f
hi Identifier      guifg=#efdcbc
hi IncSearch       guibg=#f8f893 guifg=#385f38
hi Keyword         guifg=#f0dfaf
hi Label           guifg=#dfcfaf gui=underline
hi LineNr          guifg=#9fafaf guibg=#262626
hi Macro           guifg=#ffcfaf
hi ModeMsg         guifg=#ffcfaf gui=none
hi MoreMsg         guifg=#ffffff
hi NonText         guifg=#404040
hi Number          guifg=#8cd0d3
hi Operator        guifg=#f0efd0
hi PreCondit       guifg=#dfaf8f
hi PreProc         guifg=#ffcfaf
hi Question        guifg=#ffffff
hi Repeat          guifg=#ffd7a7
hi Search          guifg=#ffffe0 guibg=#284f28
hi SpecialChar     guifg=#dca3a3
hi SpecialComment  guifg=#82a282
hi Special         guifg=#cfbfaf
hi SpecialKey      guifg=#9ece9e
hi Statement       guifg=#e3ceab gui=none
hi StatusLine      gui=reverse guifg=#2e4340 guibg=#ccdc90
hi StatusLineNC    gui=reverse guifg=#2e3330 guibg=#88b090
hi StorageClass    guifg=#c3bf9f
hi String          guifg=#cc9393
hi Structure       guifg=#efefaf
hi Tag             guifg=#e89393
hi Title           guifg=#efefef
hi Todo            guifg=#dfdfdf guibg=bg
hi Typedef         guifg=#dfe4cf
hi Type            guifg=#dfdfbf
hi Underlined      guifg=#dcdccc gui=underline
hi VertSplit       guifg=#303030 guibg=#688060
hi VisualNOS       guifg=#333333 guibg=#f18c96 gui=underline
hi WarningMsg      guifg=#ffffff guibg=#333333
hi WildMenu        guibg=#2c302d guifg=#cbecd0 gui=underline

" Entering Kurt zone
if &t_Co > 255
    hi Boolean         ctermfg=181  
    hi Character       ctermfg=181
    hi Comment         ctermfg=108   
    hi Conditional     ctermfg=223
    hi Constant        ctermfg=181
    hi Cursor          ctermfg=233   ctermbg=109
    hi Debug           ctermfg=181
    hi Define          ctermfg=223
    hi Delimiter       ctermfg=245  
    hi DiffAdd         ctermfg=66    ctermbg=237
    hi DiffChange      ctermbg=236  
    hi DiffDelete      ctermfg=236   ctermbg=238    
    hi DiffText        ctermfg=217   ctermbg=237
    hi Directory       ctermfg=188
    hi ErrorMsg        ctermfg=115   ctermbg=236
    hi Exception       ctermfg=249
    hi Float           ctermfg=251  
    hi FoldColumn      ctermfg=109   ctermbg=238    
    hi Folded          ctermfg=109   ctermbg=238    
    hi Function        ctermfg=228  
    hi Identifier      ctermfg=223  
    "hi IncSearch       ctermbg=228   ctermfg=238    
    "hi IncSearch       ctermbg=228  ctermfg=0
    hi IncSearch       ctermbg=228  ctermfg=25
    hi Keyword         ctermfg=223
    hi Label           ctermfg=187   cterm=underline
    hi LineNr          ctermfg=248   ctermbg=235    
    hi Macro           ctermfg=223
    hi ModeMsg         ctermfg=223   cterm=none
    hi MoreMsg         ctermfg=15
    hi NonText         ctermfg=238  
    hi Number          ctermfg=116  
    hi Operator        ctermfg=230  
    hi PreCondit       ctermfg=180
    hi PreProc         ctermfg=223
    hi Question        ctermfg=15
    hi Repeat          ctermfg=223
    "hi Search          ctermfg=230   ctermbg=236    
    "hi Search          ctermfg=0  ctermbg=174   
    hi Search          ctermfg=228   ctermbg=238    
    hi SpecialChar     ctermfg=181
    hi SpecialComment  ctermfg=108
    hi Special         ctermfg=181  
    hi SpecialKey      ctermfg=151  
    hi Statement       ctermfg=187   ctermbg=234     cterm=none
    hi StatusLine      ctermfg=237   ctermbg=186    
    hi StatusLineNC    ctermfg=236   ctermbg=108    
    hi StorageClass    ctermfg=249
    hi String          ctermfg=174  
    hi Structure       ctermfg=229
    hi Tag             ctermfg=181
    hi Title           ctermfg=7     ctermbg=234
    hi Todo            ctermfg=108   ctermbg=234
    hi Typedef         ctermfg=253
    hi Type            ctermfg=187
    hi Underlined      ctermfg=188   ctermbg=234
    hi VertSplit       ctermfg=236   ctermbg=65 
    hi VisualNOS       ctermfg=236   ctermbg=210
    hi WarningMsg      ctermfg=15    ctermbg=236
    hi WildMenu        ctermbg=236   ctermfg=194

	if version >= 700
		"set cursorline
		"if has("gui_running")
		"	hi CursorLine term=none cterm=none guibg=black
		"else
		"	hi CursorLine term=none cterm=none ctermbg=0
		"endif
	endif

    if exists("g:zenburn_high_Contrast")
        "hi Normal ctermfg=188 ctermbg=234
        hi Normal ctermfg=253 ctermbg=234
    else
        "hi Normal ctermfg=188 ctermbg=237
        hi Normal ctermfg=253 ctermbg=237
        hi Cursor          ctermbg=109
        hi diffadd         ctermbg=237
        hi diffdelete      ctermbg=238
        hi difftext        ctermbg=237
        hi errormsg        ctermbg=237
        hi foldcolumn      ctermbg=238
        hi folded          ctermbg=238
        hi incsearch       ctermbg=228
        hi linenr          ctermbg=238  
        hi search          ctermbg=238
        hi statement       ctermbg=237
        hi statusline      ctermbg=144
        hi statuslinenc    ctermbg=108
        hi title           ctermbg=237
        hi todo            ctermbg=237
        hi underlined      ctermbg=237
        hi vertsplit       ctermbg=65 
        hi visualnos       ctermbg=210
        hi warningmsg      ctermbg=236
        hi wildmenu        ctermbg=236
    endif
endif


if exists("g:zenburn_high_Contrast")
    " use new darker background
    hi Normal          guifg=#dcdccc guibg=#1f1f1f
else
    " Original, lighter background
    hi Normal          guifg=#dcdccc guibg=#3f3f3f
endif

if exists("g:zenburn_alternate_Visual")
    " Visual with more contrast, thanks to Steve Hall & Cream posse
    " gui=none fixes weird highlight problem in at least GVim 7.0.66, thanks to Kurt Maier
    hi Visual          guifg=#000000 guibg=#71d3b4 gui=none
    hi VisualNOS       guifg=#000000 guibg=#71d3b4 gui=none
else
    " use default visual
    hi Visual          guifg=#233323 guibg=#71d3b4 gui=none
    hi VisualNOS       guifg=#233323 guibg=#71d3b4 gui=none
endif

if exists("g:zenburn_alternate_Error")
    " use a bit different Error
    hi Error           guifg=#ef9f9f guibg=#201010
else
    " default
    hi Error           guifg=#e37170 guibg=#332323 gui=none
endif

if exists("g:zenburn_alternate_Include")
    " original setting
    hi Include         guifg=#ffcfaf
else
    " new, less contrasted one
    hi Include         guifg=#dfaf8f
endif
    " TODO check every syntax group that they're ok
	"
