/*

    This is a class similar to ArrayList, but designed to be more space efficient.
    
    The size of the list is exactly as requested on init. 
    
    Also, the reserve method does not allocate 4 times what was requested,
    unlike the ensureCapacity method in ArrayList. This is to save memory; if
    speed is an issue, and you are appending elements, use the reserve method
    before appending.
    
    A new method resize is also added which sets both the capacity and size to
    *exactly* that requested.  Also, an initialization value is optional,
    defaulting to zero.  This can of course be accomplished on construction
    using new(value, size) but we want the ability to resize later.

    Other new methods:
        copy: For copying in and out to other Vectors
        init: func ~fromVec: initialize from an existing Vector
        clear: Reallocates down to a single element and sets size to zero.


*/


Vector: class <T>  {

	data : T*
	capacity : SizeT
	size = 0 : SizeT

	init: func {
        // reserve a capacity of at least one element, but size will still be
        // zero
		this reserve(1)
	}

    // init with the given size, note default value comes from gc_malloc which
    // always inits to zero
	init: func ~withSize (=size) {
        this resize(size)
	}
    
    // init from another Vector
    init: func ~fromVec (vec: Vector<T>) {
        this copy(vec)
    }


    // Init from a data pointer and the size; this one is less safe
    init: func ~fromData (inputData: T*, =size) {
        this resize(size)
        memcpy(this data, inputData, size*(T size))
        this capacity = this size
    }

    // Explicit free of memory.  Useful if you want to re-use the container,
    // but not needed for memory management in general since the GC can deal
    // with it
	clear: func {
        // realloc to capacity of 1 but set size to 0
        this changeCapacity(1)
		this size = 0
	}


	get: inline func(index: SizeT) -> T {
		checkIndex(index)
		return this data[index]
	}
	getNocheck: inline func(index: SizeT) -> T {
		return this data[index]
	}
	/**
	 * Replaces the element at the specified position in this list with
	 * the specified element.
	 */
	set: inline func(index: SizeT, element: T) {
        checkIndex(index)
		this data[index] = element
	}

    setNocheck: inline func(index: SizeT, element: T) -> T {
		this data[index] = element
	}



	/** private */
	checkIndex: inline func (index: SizeT) {
		if (index < 0) {
            Exception new(This, "Index too small! " + index + " < 0") throw()
        }
		if (index >= this size) {
            Exception new(This, "Index too big! " + index + " >= " + size()) throw()
        }
	}


    /**
     * Does an in-place sort, with the given comparison function
     */
    sort: func (greaterThan: Func (T, T) -> Bool) {
        inOrder := false
        tmp: T
        while (!inOrder) {
            inOrder = true
            for (i:SizeT in 0..size - 1) {
                if (greaterThan(data[i], data[i + 1])) {
                    inOrder = false
                    tmp = data[i]
                    data[i] = data[i + 1]
                    data[i + 1] = tmp
                }
            }
        }
    }


	/**
	 * @return the number of elements in this list.
	 */
	size: inline func -> SizeT { size }

	/**
	 * Increases the capacity of this Vector instance, if necessary,
	 * to ensure that it can hold at least the number of elements
	 * specified by the minimum capacity argument.
     
        Note the size member data is not changed
	 */
	reserve: inline func (newCapacity: SizeT) {
		if(newCapacity > capacity) {
            this changeCapacity(newCapacity)
		}
	}


    // If the capacity is not equal to the requested size, a re-allocation
    // occurs.  Othersize just the size member is updated.
    resize: func (newSize : SizeT) {
        if(newSize != capacity) {
            changeCapacity(newSize)
        }
        size=capacity
    }
    resize: func~withValue (newSize: SizeT, val: T) {
        oldSize := size
        this resize(newSize)

        // copy in the value to all the new positions
        if (newSize > oldSize) {
            for (i:SizeT in oldSize..size) {
                data[i] = val
            }
        }
    }

    // This is the memory allocation workhorse note the size member is *not*
    // changed unless size is greater than the new capacity.  Use resize if you
    // want to make sure both are changed

    changeCapacity: func (newCapacity: SizeT) {
        tmpData: T*
        if (data) {
            tmpData = gc_realloc(data, newCapacity*(T size))
        } else {
            tmpData = gc_malloc(newCapacity*(T size))
        }

        if (tmpData) {
            data = tmpData
        } else {
            Exception new(This, "Failed to allocate %zu bytes of memory for array to grow! Exiting..\n" format(newCapacity* T size)) throw()
        }
        capacity = newCapacity

        // only change size if it is larger than the 
        // capacity
        if (size > capacity) {
            size=capacity
        }
    }

    // like push_back in std::vector
	add: func (element: T) {
		reserve(size + 1)
		data[size] = element
		size += 1
	}

    // like insert in std::vector
	add: func ~withIndex (index: SizeT, element: T) {
        // inserting at 0 can be optimized
		if(index == 0) {
            reserve(size + 1)
            dst, src: Octet*
            dst = data + (T size)
            src = data
            memmove(dst, src, T size * size)
            data[0] = element
            size += 1
            return
        }

        if(index == size) {
            add(element)
            return
        }

        checkIndex(index)
		reserve(size + 1)
		dst, src: Octet*
		dst = data + (T size * (index + 1))
		src = data + (T size * index)
		bsize := (size - index) * T size
		memmove(dst, src, bsize)
		data[index] = element
		size += 1
	}

	iterator: func -> VectorIterator<T> { return VectorIterator<T> new(this) }

	backIterator: func -> VectorIterator<T> {
	    iter := VectorIterator<T> new(this)
	    iter index = size()
	    return iter
	}


    //
    // Copying in and out
    // don't know yet how to do second generic, so only <T>
    //

    // copy in
    copy: func~input <T> (vec: Vector<T>) {
        this resize(vec size)
        memcpy(this data, vec data, (vec size)*(T size) )
    }
    // copy out
    copy: func~output <T> (vec: Vector<T>) {
        vec resize(this size)
        memcpy(vec data, this data, (vec size)*(T size) )
    }



	clone: func -> This<T> {
		output := This<T> new(this)
		return output
	}

    emptyClone: func <K> -> This<K> {
        This<K> new()
    }

    /** */
    toArray: func -> Pointer { data }

}

operator [] <T> (vec: Vector<T>, i: SizeT) -> T { vec get(i) }
operator []= <T> (vec: Vector<T>, i: SizeT, element: T) { vec set(i, element) }

VectorIterator: class <T> {

	list: Vector<T>
	index := 0

	init: func ~iter (=list) {}

	hasNext: func -> Bool { index < list size() }

	next: func -> T {
		element := list get(index)
		index += 1
		return element
	}

    hasPrev: func -> Bool { index > 0 }

    prev: func -> T {
        index -= 1
		element := list get(index)
		return element
	}


}



