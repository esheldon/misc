pro _compile_admom, dll_path, name, extra_cflags, recompile=recompile

	name = 'admom'
	;extra_cflags = '-O3 -mtune=native'
	;extra_cflags = '-O3'
	srcdir='.'

	if keyword_set(recompile) then begin
		reuse_existing=0
	endif else begin
		reuse_existing=1
	endelse

	make_dll, name, name, $
		reuse_existing=reuse_existing, $
		input_directory=srcdir, $
		output_directory=srcdir, $
		compile_directory=srcdir, $
		dll_path=dll_path, /show_all_output, $
		extra_cflags=extra_cflags
end

pro test_admom

	im=mom2gauss(2.0,0.0,2.0,[20,20])
	x=10.0
	y=10.0
	sky=0.0
	skysig=5.5
	shiftmax=10.0
	wguess=2.0
	admom,im,x,y,sky,skysig,shiftmax,wguess,ixx,iyy,ixy,momerr,rho4,whyflag,/recompile

	print,ixx,iyy,ixy,momerr,rho4,whyflag
end

pro admom, $
		image, x, y, sky, skysig, shiftmax, wguess, $
		ixx, iyy, ixy, momerr, rho4, whyflag, $
		recompile=recompile

	if n_params() lt 13 then begin
		on_error, 2
		print,'usage: admom, image, x, y, sky, skysig, shiftmax, wguess, '
		print,'              ixx, iyy, ixy, momerr, rho4, whyflag, '
		print,'              /recompile'
		message,'halting'
	endif

	_compile_admom, dll_path, name, extra_cflags, recompile=recompile

	if keyword_set(recompile) then begin
		unload=1
		ignore_existing_glue=1
	endif

	nobj = n_elements(x)
	sz=size(image)
	nx=sz[1]
	ny=sz[2]

	defval = -9999.0

	ixx=fltarr(nobj)
	iyy=ixx
	ixy=ixx
	momerr=ixx
	rho4=ixx
	whyflag=lonarr(nobj)
	
    res = call_external( $
        dll_path, name, $
		image,$
		nx, $
		ny, $
		nobj, $
		sky, $
		skysig, $
		shiftmax, $
		wguess, $
		x, $
		y, $
		ixx, $
		iyy, $
		ixy, $
		momerr, $
		rho4, $
		whyflag, $
        unload=unload,$
        ignore_existing_glue=ignore_existing_glue, $
        /auto_glue, $
        extra_cflags=extra_cflags,$
        /show_all_output, $
		value=[0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0])

	w=where(whyflag ne 0,nw)
	if nw ne 0 then begin
		ixx[w] = defval
		iyy[w] = defval
		ixy[w] = defval
		momerr[w] = -defval
		rho4[w] = defval
	endif

end
