#include "partition.h"

void swap(int* a,int* b) //swaps values of a and b
{
	int temp = *a;
	*a = *b;
	*b = temp;
} 



// partition elements by quick-sort algorithm using an array of indexes
// data and ind must be created before calling
int partition(int k, const double *data, int *ind, int n)
{
	int parl, parh, up, down, med, vali;
	parh = n-1;	
	parl = 0;
	double val;
	while(1)
	{
		if(parh <= parl + 1)
		{
			if(parh == (parl + 1) && data[ind[parh]] < data[ind[parl]])
				swap(&ind[parl], &ind[parh]);
			return ind[k];
		}
		else
		{
			med = (parh + parl)/2;
			swap(&ind[med], &ind[parl+1]);
			if(data[ind[parl]] > data[ind[parh]])
				swap(&ind[parl], &ind[parh]);

			if(data[ind[parl+1]] > data[ind[parh]])
				swap(&ind[parl+1], &ind[parh]);

			if(data[ind[parl]] > data[ind[parl+1]])
				swap(&ind[parl], &ind[parl+1]);

			up = parl + 1;
			down = parh;
			vali = ind[parl+1];
			val = data[vali];
			while(1)
            {
                do up++;
                while(data[ind[up]] < val);

                do down--;
                while(data[ind[down]] > val);

				if(down < up) break;

				swap(&ind[up], &ind[down]);
			}
			ind[parl + 1] = ind[down];
			ind[down] = vali;
			if(down >= k) parh = down - 1;
			if(down <= k) parl = up;
		}

	}
	return 1; 
}


