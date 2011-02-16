#include "KDTree.h"



// SWAPS THE VALUES OF a AND b
void swap(int& a,int& b) //swaps values of a and b
{
    int temp = a;
    a = b;
    b = temp;
}  

//partition elements by quick-sort algorithm using an array of indexes
int partition(int k, double *mags,int *ind, int n)
{
	int parl, parh, up, down, med, vali;
	parh = n-1;	
	parl = 0;
	double val;
	while(true)
	{
		if(parh <= parl + 1)
		{
			if(parh == (parl + 1) && mags[ind[parh]] < mags[ind[parl]])
				swap(ind[parl],ind[parh]);
			return ind[k];
		}
		else
		{
			med = (parh + parl)/2;
			swap(ind[med],ind[parl+1]);
			if(mags[ind[parl]] > mags[ind[parh]])
				swap(ind[parl], ind[parh]);
			if(mags[ind[parl+1]] > mags[ind[parh]])
				swap(ind[parl+1],ind[parh]);
			if(mags[ind[parl]] > mags[ind[parl+1]])
				swap(ind[parl],ind[parl+1]);
			up = parl + 1;
			down = parh;
			vali = ind[parl+1];
			val = mags[vali];
			while(true)
			{
				do up++;
				while(mags[ind[up]] < val);
				do down--;
				while(mags[ind[down]] > val);
				if(down < up) break;
				swap(ind[up], ind[down]);
			}
			ind[parl + 1] = ind[down];
			ind[down] = vali;
			if(down >= k) parh = down - 1;
			if(down <= k) parl = up;
		}

	}
	return 1; 
}


