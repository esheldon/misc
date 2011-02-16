
import structs/ArrayList
import Vector

main: func (args: ArrayList<String>) {

    if (args size < 2) {
        "usage: array method" println()
        exit(45)
    }

    method := args[1] toInt()



    printf("Using method: %d\n", method)

    // these can only have size determined at compile time and are not dynamic
    //farr := Float[num] new();

    fvec := Vector<Float> new(5)

    //num = 10000000 : SizeT
    num = 10 : SizeT

    fvec resize(num)

    tot=0.0 : Float

    printf("fvec size: %ld\n", fvec size);

    "creating range and totaling" println()

    // repeat each experiment repeat times so that the malloc
    // is not the bottleneck
    repeat := 50

    //method := 3
    match method {
        case 1 => 
            // The slow way, about 9 times slower than pointer access
            printf("Using [] access\n");
            for (rep in 0..repeat) {

                tot = 0.0
                for (i in 0..fvec size) {
                    fvec[i] = i as Float
                    // this compiles but doesn't work
                    //fvec[i] = i
                }
                for (i in 0..fvec size) {
                    // can't use += here
                    tot = tot + fvec[i]

                    // both of these give a compile error in the c code
                    // It is not correctly parsing the +=
                    //rock_tmp/./array.c:70: error: void value not ignored 
                    // as it ought to be
                    //tot += fvec[i]
                    //tot += fvec[i] as Float
                }
            }
               
        case 2 => 
            // the fast way, just as fast as C using []
            // although pointer arithmetic is not supported
            // which would be a little faster
            printf("Using pointer\n");


            ptr : Float*
            ptr = fvec toArray()
            for (rep in 0..repeat) {
                tot = 0.0
                for (i:SizeT in 0..fvec size) {
                    ptr[i] = i
                }

                for (i in 0..fvec size) {
                    tot += ptr[i]
                }
            }

        case 3 => 
            // this takes about half as long as the [] access
            // but that's still 4.5 times slower than the pointer
            printf("Using Nocheck access\n");
            for (rep in 0..repeat) {
                tot = 0.0
                for (i in 0..fvec size) {
                    fvec setNocheck(i,i as Float)
                }
                for (i in 0..fvec size) {
                    val := fvec getNocheck(i)
                    tot += val
                }
            }
        case 4 => 
            // this was twice as slow as []
            printf("Using Nocheck access for set and iter for get\n");
            for (rep in 0..repeat) {
                tot = 0.0
                for (i in 0..fvec size) {
                    fvec setNocheck(i,i as Float)
                }
                for (val in fvec) {
                    tot += val
                }
            }

    }

    printf("total: %f\n", tot)

    println()
    printf("fvec[3]: %f\n", fvec[3]);

    println()
    printf("Increasing size by 5 default value 2\n")
    fvec resize((fvec size)+5, 2.0 as Float)
    printf("Making sure element 3 did not change: fvec[3]: %f\n", fvec[3]);

    index := (fvec size) -1
    printf("fvec[%ld]: %f\n", index, fvec[index])

    
    printf("\nTrying a String vector\n")
    svec := Vector<String> new(4)

    svec[0] = "hello"
    svec[1] = "there"
    svec[2] = "world"
    svec[3] = "Wassup Hippees!!!!!"

    for (i in 0..svec size) {
        printf("  svec[%d]: '%s'\n", i, svec[i])
    }

    "iterating backwards" println()
    backit := svec backIterator()
    while (backit hasPrev()) {
        el := backit prev()
        printf("    %s\n", el)
    }

}
