pro test_autoglue
  n = 10L
  arr = dindgen(n)
  sumarr = dblarr(n)
    
  ret = call_external('test_autoglue.so','test_autoglue',$
                       arr, sumarr, n, /auto_glue,/unload)
  print,'Running sum: ',sumarr
end 
