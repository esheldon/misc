// fixes a heap where the 0-th element is possibly out of place
void fixheap(double *ds, int *ns, int ni) 
{
    int n = ni -1;
    double v = ds[0];
    int vind = ns[0];
    int jhi = 0;
    int jlo = 1;
    while(jlo <= n)
    {
        if(jlo < n && ds[jlo] < ds[jlo+1])  //If the right node is larger
            jlo++;
        if(v >= ds[jlo]) //it forms a heap already
            break;
        ds[jhi] = ds[jlo]; //promotes the bigger of the branches
        ns[jhi] = ns[jlo];
        jhi = jlo; //move down the heap
        jlo = 2*jlo + 1; // calculates position of left branch
    }
    ds[jhi] = v; //places v, vind at correct position in heap
    ns[jhi] = vind;
}


