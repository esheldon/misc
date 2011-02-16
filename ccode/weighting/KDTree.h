#ifndef _KDTREES_H
#define _KDTREES_H
#include <math.h>
#include <vector>
#include <iostream>
#include "Point.h"
#include "Hcube.h"
#include "util.h"

using namespace std;
//const int dim = 5;  //number of dimensions
const double big = 50; //upper bound on the coordinates. Lower bound set to -50.

int partition(int k, double *mags,int *ind, int n);
void swap(int& a,int& b);


//Node of the binary tree
template<int dim> struct treenode: Hcube<dim>
{
    //int currentdim; //Dimension of node 
    int schild;  //Left node or all values with greater mag in currentdim
    int bchild; //right node
    int parent; //parent node 
    int lowind; // lowest number in array of indicies
    int highind;  // highesst number in array of indices
    treenode(Point<dim> hi, 
             Point<dim> lo, 
             int lowi, 
             int highi, 
             int pari, 
             int left, 
             int right): Hcube<dim>(lo,hi)
    {
        lowind = lowi;
        highind = highi;
        parent = pari;
        schild = left;
        bchild = right;
    }
    treenode() {}
};





template<int dim> struct KDTree
{

    vector<treenode<dim> > nodes;
    vector< Point<dim> > &pts;
    vector<double> cords;
    vector<int> ptind;
    vector<int> revind;
    int npts;
    //CONSTRUCTOR takes in a vector of Points
    KDTree(vector< Point<dim> > &mags): pts(mags) {


        int cur, tpar, tdim, curlo, curhi, np, k, boxind;  //Task loop variables
        int boxnum, m;  //Number of box variables
        int pars[50], dims[50];
        int n = mags.size();  //Number of galaxies
        npts = n;
        int *indp;
        double *corp;

        ptind.resize(n);
        for(int i = 0; i < n; i++) //Initializing index array
            ptind[i] = i;

        // Creating an array of all the first magnitudes followed by second, 
        // third, fourth, and fifth
        cords.resize(n*dim);

        for(int i = 0; i < n; i++)
            for(int j = 0; j < dim; j++)
                cords[i + n*j] = pts[i].x[j];

        //Creating Upper and Lower Boundaries
        double temp[dim];
        for(int i = 0; i < dim; i++)
            temp[i] = -big;
        Point<dim> lo(temp);
        for(int i = 0; i < dim; i++)
            temp[i] = big;
        Point<dim> hi(temp);

        //Calculating the number of box neededs: boxnum
        m = 1;
        while(m < n)
            m*= 2;
        boxnum = m -1;
        if(2*n - m/2 -1 < boxnum)
            boxnum= 2*n - m/2 -1;

        //initializing array of nodes
        nodes.resize(boxnum);
        nodes[0] =  treenode<dim>(hi,lo,0,n-1,0,0,0);

        //Task LOOP
        pars[1] = 0;
        dims[1] = 0;
        cur = 1;
        boxind = 0;
        while(cur) {
            tpar = pars[cur];
            tdim = dims[cur];
            cur--;
            curlo = nodes[tpar].lowind;
            curhi = nodes[tpar].highind;
            indp = &ptind[curlo];
            corp = &cords[tdim*n];
            np	 = curhi- curlo + 1; //points 
            k = (np-1)/2; 
            (void) partition(k,corp,indp,np);
            hi = nodes[tpar].high;
            lo = nodes[tpar].low;
            hi.x[tdim] = cords[tdim*n + indp[k]];
            lo.x[tdim] = cords[tdim*n + indp[k]];
            boxind++;

            //Creats the smallest daughter box
            nodes[boxind] = treenode<dim>(hi,nodes[tpar].low,curlo,curlo+k,tpar,0,0);  
            boxind++;

            // Creates the larger daughter box
            nodes[boxind] = treenode<dim>(nodes[tpar].high,lo,curlo+k+1,curhi,tpar,0,0); 
            nodes[tpar].schild = boxind - 1;  //sets the children
            nodes[tpar].bchild = boxind;

            //if left box needs to be subdivided
            if(k > 1) {
                cur++;
                pars[cur] = boxind-1;
                // Increments the dimension. sets back to 0 if tdim = dim
                dims[cur] = (tdim+1)%dim; 
            }
            //if right box needs subdivisions
            if(np-k > 3) {
                cur++;
                pars[cur] = boxind;
                dims[cur] = (tdim+1)%dim; 
            }
        }	

        revind.resize(n);
        for(int j = 0; j < npts; j++)
            revind[ptind[j]] = j; 

    } 

    // returns the index of the cube containig Point p
    int findCube(const Point<dim> & p) {
        int num = 0;
        int curdim = 0;
        int ldau;
        //if the node isn't a leaf
        while(nodes[num].schild != 0) {
            ldau = nodes[num].schild;
            if(p.x[curdim] <= nodes[ldau].high.x[curdim]) 
                num = ldau;
            else
                num = nodes[num].bchild;
            curdim = (curdim+1)%dim;
        }

        return num;
    }
    int findBox(int p) {
        int num = 0;
        int ind = revind[p];
        int ldau;
        int curdim = 0;
        while(nodes[num].schild > 0) {
            ldau = nodes[num].schild;
            if(ind <= nodes[ldau].highind)
                num = ldau;
            else 
                num = nodes[num].bchild;
            curdim = (curdim+1)%dim;
        }
        return num;
    }

    //returns a large distance if the points are the same
    double d2(const Point<dim>& a, const Point<dim>& b) {
        if(a==b)
            return big*dim;
        return a.dist(b);
    }
    double d2(int i, int j) {
        return d2(pts[i],pts[j]);
    }


    // n-nearest neighbors to in tree corresponding to j
    // ds distances with ds[0] biggest, ns corresponding indicies
    // this version uses vectors: ESS
    void nneigh(int j, vector<double>& ds, vector<int>& ns) {

        int boxi;
        double dcur;
        if (ds.size() != ns.size()) {
            throw_runtime("ns and ds must be same size");
        }
        if (ds.size() > (npts -1) ) {
            throw_runtime("Not Enough Points");
        }
        for(int i = 0; i < ds.size(); i++) {
            ds[i] = big*dim;
        }

        boxi = nodes[findBox(j)].parent;
        while(nodes[boxi].highind - nodes[boxi].lowind < ds.size())  
            boxi = nodes[boxi].parent;
        for(int i = nodes[boxi].lowind; i <= nodes[boxi].highind; i++) {
            if(j == ptind[i]) continue;
            dcur = d2(j,ptind[i]);
            if(dcur < ds[0]) {
                ds[0] = dcur;
                ns[0] = ptind[i];
                if(ds.size()>0) fixheap(ds,ns);
            }
        }

        int cur = 1;
        int task[100];
        task[1] = 0;
        int curbox;
        while(cur) {
            curbox = task[cur];
            cur--;
            if(boxi == curbox) continue;
            double dtmp = nodes[curbox].dist(pts[j]);
            if( dtmp < ds[0]) {
                if(nodes[curbox].schild > 0) {	cur++;
                    task[cur] = nodes[curbox].schild;
                    cur++;
                    task[cur] = nodes[curbox].bchild;
                } else {
                    for(int i = nodes[curbox].lowind; i <= nodes[curbox].highind; i++) {
                        dcur = d2(ptind[i],j);
                        if(dcur < ds[0]) {
                            ds[0] = dcur;
                            ns[0] = ptind[i];
                            if(ds.size()>1) fixheap(ds,ns);
                        }
                    }
                }
            }
        }
    }


    // n-nearest neighbors to Point p
    // This one uses vectors, ESS
    void nneigh(const Point<dim>&p, vector<double>& ds, vector<int>& ns)  
    {

        int boxi;
        double dcur;

        if (ds.size() != ns.size()) {
            throw_runtime("ns and ds must be same size");
        }
        // Not enough points to return the n neighbours
        if(ds.size() > (npts-1))
            throw_runtime("Not enough points");
        for(int i = 0; i < ds.size(); i++)
            ds[i] = big*dim;

        //find a box containing n-Points and given point
        boxi = nodes[findCube(p)].parent;
        while(nodes[boxi].highind - nodes[boxi].lowind < ds.size())  
            boxi = nodes[boxi].parent;

        //Keep the n closest Points in this box
        for(int i = nodes[boxi].lowind; i <= nodes[boxi].highind; i++) {
            if(p == pts[ptind[i]]) 
                continue;
            dcur = d2(pts[ptind[i]],p);
            if(dcur < ds[0]) {
                ds[0] = dcur;
                ns[0] = ptind[i];
                if(ds.size() > 1) fixheap(ds,ns);
            }
        }

        int cur = 1;
        int task[100];
        task[1] = 0;
        int curbox;
        while(cur) {
            curbox = task[cur];
            cur--;
            if(boxi == curbox) continue;
            if( nodes[curbox].dist(p) < ds[0]) {
                if(nodes[curbox].schild > 0) {	cur++;
                    task[cur] = nodes[curbox].schild;
                    cur++;
                    task[cur] = nodes[curbox].bchild;
                } else {
                    for(int i = nodes[curbox].lowind; i <= nodes[curbox].highind; i++) {
                        dcur = d2(pts[ptind[i]],p);
                        if(dcur < ds[0]) {
                            ds[0] = dcur;
                            ns[0] = ptind[i];
                            if(ds.size()>1) fixheap(ds,ns);
                        }
                    }
                }
            }
        }
    }




    //fixes a heap where the 0-th element is possibly out of place
    // this version uses vectors: ESS
    void fixheap(vector<double>& ds, vector<int>& ns) 
    {
        int n = ds.size() -1;

        double v = ds[0];
        int vind = ns[0];
        int jhi = 0;
        int jlo = 1;
        while(jlo < ds.size()) {
            // If the right node is larger
            if(jlo < (ds.size()-1) && ds[jlo] < ds[jlo+1])  
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


    // Returns the indices of all points within radius r of Point p
    // This one uses vectors ESS
    void pointsr(double r, Point<dim> & p,  vector<int>& inds)
    {
        inds.clear();

        int box = 0;
        int curdim = 0;
        int oldbox,dau1,dau2;
        while(nodes[box].schild > 0) {
            oldbox = box;
            dau1 = nodes[box].schild;
            dau2 = nodes[box].bchild;
            if(p.x[curdim] + r <= nodes[dau1].high.x[curdim]) 
                box = dau1;
            else if(p.x[curdim] - r >= nodes[dau2].low.x[curdim]) 
                box = dau2;
            curdim++;
            curdim%dim;
            if(box == oldbox) break;

        }
        int task[100];
        int cur = 1;
        task[1] = box;
        int curbox;
        while(cur) {
            box = task[cur];
            cur--;
            if(nodes[box].schild != 0) {
                if(nodes[nodes[box].schild].dist(p) <= r) {
                    cur++;
                    task[cur] = nodes[box].schild;
                }
                if(nodes[nodes[box].bchild].dist(p) <= r) {
                    cur++;
                    task[cur] = nodes[box].bchild;
                }
            } else {
                for(int j= nodes[box].lowind; j <= nodes[box].highind; j++) {
                    if(pts[ptind[j]].dist(p) <= r) {
                        inds.push_back( ptind[j] );
                    }
                }

            }
        }

    }






};

#endif
