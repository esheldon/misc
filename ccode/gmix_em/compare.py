import images

im=images.image_read("test-image.dat")
model=images.image_read("test-image-fit.dat")

im /= im.sum()
model /= model.sum()

images.compare_images(im, model,label1='image',label2='model')
