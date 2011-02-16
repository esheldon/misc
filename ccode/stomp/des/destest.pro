
function smapread, file

	nline=file_lines(file)

	map = replicate({index:0L, res:0L}, nline)

	openr, lun, file, /get
	readf, lun, map
	free_lun, lun

	return, map
	
end

function densmapread, file

	nline=file_lines(file)

	map = replicate({index:0L, res:0L, density:0d}, nline)

	openr, lun, file, /get
	readf, lun, map
	free_lun, lun

	return, map
	
end



function getang, file

	nline=file_lines(file)

	ang = replicate({lambda:0d, eta:0d}, nline)

	openr, lun, file, /get
	readf, lun, ang
	free_lun, lun

	return, ang
	
end



pro destest

	psfile = 'test_output/destest.eps'
	begplot,psfile,/encap, xsize=8.5, ysize=8.5
	!x.thick=1
	!y.thick=1
	!p.thick=1

	allmap = smapread('test_output/des_footprint.dat')
	nodiskmap = smapread('test_output/des_footprint_minusdisk.dat')
	nodiskpolymap = smapread('test_output/des_footprint_minusdiskpoly.dat')
	nodiskpolymap2 = smapread('test_output/des_footprint_minusdiskpoly2.dat')
	nodiskpoly2rand = getang('test_output/des_footprint_minusdiskpoly2_rand.dat')

	exfootmap = smapread('test_output/des_extrafoot.dat')
	twofeetmap = smapread('test_output/des_twofeet.dat')
	twofeetrand = getang('test_output/des_twofeet_rand.dat')
	twofeetrandw = getang('test_output/des_twofeet_rand_weighted.dat')


	;rand = getang('test_output/leftover_rand.dat')

	erase & multiplot, [2,2], /square
	tickn=replicate(' ', 10)
	display_pixel, allmap.index, res=allmap.res, /iso, $
		xtitle='', ytitle='', xtickn=tickn, ytickn=tickn, xstyle=3, ystyle=3
	multiplot

	display_pixel, nodiskmap.index, res=nodiskmap.res, /iso, $
		xtitle='', ytitle='', xtickn=tickn, ytickn=tickn, xstyle=3, ystyle=3
	multiplot

	display_pixel, nodiskpolymap.index, res=nodiskpolymap.res, /iso, $
		xtitle='', ytitle='', xtickn=tickn, ytickn=tickn, xstyle=3, ystyle=3
	multiplot

	display_pixel, nodiskpolymap2.index, res=nodiskpolymap2.res, /iso, $
		xtitle='', ytitle='', xtickn=tickn, ytickn=tickn, xstyle=3, ystyle=3
	multiplot,/default


	!p.multi=0

	endplot,/trim

	psfile = 'test_output/destest_footprint_minusdiskpoly2_rand.eps'
	begplot,psfile,/encap, xsize=8.5, ysize=8.5, /color

	display_pixel, nodiskpolymap2.index, res=nodiskpolymap2.res, /iso, $
		xtitle='', ytitle='', xtickn=tickn, ytickn=tickn, xstyle=3, ystyle=3
	eq2csurvey, nodiskpoly2rand.lambda, nodiskpoly2rand.eta, lam,eta
	pplot, lam, eta, psym=8, symsize=0.2, /overplot, color=c2i('darkgreen')

	endplot, /trim


	; with another footprint
	psfile = 'test_output/destest_twofeet_rand.eps'
	begplot,psfile,/encap, xsize=8.5, ysize=8.5, /color

	display_pixel, twofeetmap.index, res=twofeetmap.res, /iso, $
		xtitle='', ytitle='', xtickn=tickn, ytickn=tickn, xstyle=3, ystyle=3


;	display_pixel, exfootmap.index, res=exfootmap.res, /iso, $
;		xtitle='', ytitle='', xtickn=tickn, ytickn=tickn, xstyle=3, ystyle=3,$
;		/over, color=c2i('blue')

	eq2csurvey, twofeetrand.lambda, twofeetrand.eta, lam2,eta2
	pplot, lam2, eta2, psym=8, symsize=0.2, /overplot, color=c2i('darkgreen')

	endplot, /trim


	; with another footprint and weighted
	psfile = 'test_output/destest_twofeet_rand_weighted.eps'
	begplot,psfile,/encap, xsize=8.5, ysize=8.5, /color

	display_pixel, twofeetmap.index, res=twofeetmap.res, /iso, $
		xtitle='', ytitle='', xtickn=tickn, ytickn=tickn, xstyle=3, ystyle=3


;	display_pixel, exfootmap.index, res=exfootmap.res, /iso, $
;		xtitle='', ytitle='', xtickn=tickn, ytickn=tickn, xstyle=3, ystyle=3,$
;		/over, color=c2i('blue')

	eq2csurvey, twofeetrandw.lambda, twofeetrandw.eta, lam3,eta3
	pplot, lam3, eta3, psym=8, symsize=0.2, /overplot, color=c2i('darkgreen')

	endplot, /trim



end
