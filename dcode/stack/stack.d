module stack;

/**
Currently this can only grow in capacity
*/
struct Stack(T) {
    private T[] _data;
    private size_t _capacity;
    private size_t _size;

    this(size_t n) {
        _data.length = n;
    }

    @property size_t length() {
        return _size;
    }
    @property size_t capacity() {
        return _capacity;
    }
    T opIndex(size_t i) {
        assert(i < _size);
        return _data[i];
    }
    void opIndexAssign(T value, size_t i)
    {
        assert(i < _size);
        _data[i] = value;
    }

    @property bool empty() {
        return _size > 0;
    }
    @property T front() {
        assert(_size > 0);
        return _data[0];
    }
    @property T back() {
        assert(_size > 0);
        return _data[_size-1];
    }
    typeof(this) opSlice(size_t lower, size_t upper) {
        //return _data[lower..upper];
        auto ret = this;
        ret._data = _data[lower..upper];
        return ret;
    }


    size_t push(T el) {
        if (this._size == this._capacity) {
            size_t newsize;
            // we need to increase the size
            if (this._capacity == 0) {
                newsize = 1;
            } else {
                newsize = this._capacity*2;
            }
            this.reserve(newsize);
        }
        this._size++;
        this._data[this._size-1] = el;

        return this._size;
    }
    T pop() {
        T el;
        if (this._size > 0) {
            el = this._data[this._size-1];
            this._size--;
        } else {
            throw new Exception("Cannot pop an empty stack");
        }
        return el;
    }
    size_t reserve(size_t n) {
        if (n > this._capacity) {
            _data.length = n; // fills with T.init
            _capacity = n;
        }
        return this._capacity;
    }

    size_t resize(size_t n) {
        if (n > this._capacity) {
            // need to first zero any that exist but are not visible;
            // this happens if we pop() elements
            if (_size != _capacity) {
                _data[_size..$] = T.init;
            }

            // fills new elements with initializers (0,nan,etc)
            this.reserve(n);
            _size=n;
        } else if (n > _size) {
            // zero any now-visible elements that were not visible
            // due to a pop() or decrease in visible resize
            _data[_size..n] = T.init;
            _size=n;
        } else {
            // smaller than existing _size note this leaves values in the
            // existing but not visible elements at the end of the array
            // but enlarging will fill them in, see above
            _size=n;
        }
        return _size;
    }

    void sort() {
        // only sort the visible objects
        if (_size > 0) {
            this._data[0.._size].sort;
        }
    }



    int opApply(int delegate(ref T) dg)
    {
        int result = 0;

        for (size_t i = 0; i < _size; i++)
        {
            result = dg(_data[i]);
            if (result)
                break;
        }
        return result;
    }
    int opApplyReverse(int delegate(ref T) dg)
    {
        int result = 0;

        for (size_t i = (_size-1); i >= 0; i--)
        {
            result = dg(_data[i]);
            if (result)
                break;
        }
        return result;
    }

}


