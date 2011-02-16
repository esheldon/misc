#include <cmath>
#include "nr.h"
#include "neu_net.h"
#include "nr_mod.h"

using namespace std;

void dfpmin(Vec_IO_DP &p, const DP gtol, int &iter, DP &fret,
	    neu_net *net)
{
  // input: p = starting guess
  // input: gtol = convergence criteria for minimization
  // output: p = location of the minimum
  // output: iter = number of iterations that took place
  // output: fret = minimum value of the function
  const int ITMAX=20;
  const DP EPS=1.e-16;
  const DP TOLX=4*EPS,STPMX=100.0;
  bool check;
  int i,its,j;
  DP den,fac,fad,fae,fp,stpmax,sum=0.0,sumdg,sumxi,temp,test;

  //misc
  DP t1, t2;

  int n=p.size();
  Vec_DP dg(n),g(n),hdg(n),pnew(n),xi(n);
  Mat_DP hessin(n,n);
  fp=net->EE(p);
  net->dE(p,g);
  for (i=0;i<n;i++) {
    for (j=0;j<n;j++) hessin[i][j]=0.0;
    hessin[i][i]=1.0;
    xi[i] = -g[i];
    sum += p[i]*p[i];
  }
  stpmax=STPMX*MAX(sqrt(sum),DP(n));
  for (its=0;its<ITMAX;its++) {
    iter=its;
    lnsrch(p,fp,g,xi,pnew,fret,stpmax,check,net);
    fp=fret;
    /*
      if (check) {
      cout << "check is true" << endl;
      }
    */
    for (i=0;i<n;i++) {
      //cout << "pnew = " << pnew[i] << ", p = " << p[i] << endl;
      xi[i]=pnew[i]-p[i];
      p[i]=pnew[i];
    }
    test=0.0;
    for (i=0;i<n;i++) {
      temp=fabs(xi[i])/MAX(fabs(p[i]),1.0);
      //cout << "fabs(xi[i]) = " << fabs(xi[i]) << ",  temp = " << temp << endl;
      if (temp > test) test=temp;
    }
    //cout << "test1 = " << test << "  ";
    if (test < TOLX) {
      //cout << "test = " << test << ", TOLX = " << TOLX << endl;
      return;
    }
    for (i=0;i<n;i++) dg[i]=g[i];
    net->dE(p,g);
    test=0.0;
    den=MAX(fret,1.0);
    for (i=0;i<n;i++) {
      temp=fabs(g[i])*MAX(fabs(p[i]),1.0)/den;
      if (temp > test) test=temp;
    }
    //cout << "test2 = " << test << endl;
    if (test < gtol) {
      //for (i=0;i<n;i++)
	//cout << "p[i] = " << p[i] << ", g[i] = " << g[i] << endl;
      //cout << "test = " << test << ", gtol = " << gtol << endl;
      return;
    }
    for (i=0;i<n;i++) dg[i]=g[i]-dg[i];
    for (i=0;i<n;i++) {
      hdg[i]=0.0;
      for (j=0;j<n;j++) hdg[i] += hessin[i][j]*dg[j];
    }
    fac=fae=sumdg=sumxi=0.0;
    for (i=0;i<n;i++) {
      fac += dg[i]*xi[i];
      fae += dg[i]*hdg[i];
      sumdg += SQR(dg[i]);
      sumxi += SQR(xi[i]);
    }
    if (fac > sqrt(EPS*sumdg*sumxi)) {
      fac=1.0/fac;
      fad=1.0/fae;
      for (i=0;i<n;i++) dg[i]=fac*xi[i]-fad*hdg[i];
      for (i=0;i<n;i++) {
	for (j=i;j<n;j++) {
	  hessin[i][j] += fac*xi[i]*xi[j]
	    -fad*hdg[i]*hdg[j]+fae*dg[i]*dg[j];
	  hessin[j][i]=hessin[i][j];
	}
      }
    }
    for (i=0;i<n;i++) {
      xi[i]=0.0;
      for (j=0;j<n;j++) xi[i] -= hessin[i][j]*g[j];
    }
  }
  //cerr << "non congervence" << endl;
}


void lnsrch(Vec_I_DP &xold, const DP fold, Vec_I_DP &g, Vec_IO_DP &p,
	    Vec_O_DP &x, DP &f, const DP stpmax, bool &check, neu_net *net)
{
  const DP ALF=1.0e-4, TOLX=1.e-16;
  int i;
  DP a,alam,alam2=0.0,alamin,b,disc,f2=0.0;
  DP rhs1,rhs2,slope,sum,temp,test,tmplam;

  int n=xold.size();
  check=false;
  sum=0.0;
  for (i=0;i<n;i++) sum += p[i]*p[i];
  sum=sqrt(sum);
  //cout << "lnsrch: stpmax = " << stpmax << ", sum = " << sum <<  endl;
  if (sum > stpmax)
    for (i=0;i<n;i++) p[i] *= stpmax/sum;
  slope=0.0;
  for (i=0;i<n;i++)
    slope += g[i]*p[i];
  if (slope >= 0.0) {
    cerr << "round off problem in lnsrch" << endl;
    exit(1);
  }
  test=0.0;
  for (i=0;i<n;i++) {
    temp=fabs(p[i])/MAX(fabs(xold[i]),1.0);
    if (temp > test) test=temp;
  }
  alamin=TOLX/test;
  alam=1.0;
  //cout << "slope = " << slope << endl;
  for (;;) {
    //cout << "blah" << endl;
    for (i=0;i<n;i++) x[i]=xold[i]+alam*p[i];
    f=net->EE(x);
    if (alam < alamin) {
      for (i=0;i<n;i++) x[i]=xold[i];
      check=true;
      //cout << "slope = " << slope << ", alamin = " << alamin << ", alam = " << alam << endl;
      return;
    } else if (f <= fold+ALF*alam*slope) {
      //cout << "f = " << f << ", fold = " << fold << ",  fnew = " << fold+ALF*alam*slope << endl;
      return;
    }
    else {
      if (alam == 1.0)
	tmplam = -slope/(2.0*(f-fold-slope));
      else {
	rhs1=f-fold-alam*slope;
	rhs2=f2-fold-alam2*slope;
	a=(rhs1/(alam*alam)-rhs2/(alam2*alam2))/(alam-alam2);
	b=(-alam2*rhs1/(alam*alam)+alam*rhs2/(alam2*alam2))/(alam-alam2);
	if (a == 0.0) tmplam = -slope/(2.0*b);
	else {
	  disc=b*b-3.0*a*slope;
	  if (disc < 0.0) tmplam=0.5*alam;
	  else if (b <= 0.0) tmplam=(-b+sqrt(disc))/(3.0*a);
	  else tmplam=-slope/(b+sqrt(disc));
	}
	if (tmplam>0.5*alam)
	  tmplam=0.5*alam;
      }
    }
    alam2=alam;
    f2 = f;
    alam=MAX(tmplam,0.1*alam);
  }
}


void frprmn(Vec_IO_DP &p, const DP ftol, int &iter, DP &fret,
		neu_net *net)
{
  const int ITMAX=200;
  const DP EPS=1.0e-18;
  int j,its;
  DP gg,gam,fp,dgg;

  int n=p.size();
  Vec_DP g(n),h(n),xi(n);
  fp=net->EE(p);
  net->dE(p,xi);
  for (j=0;j<n;j++) {
    g[j] = -xi[j];
    xi[j]=h[j]=g[j];
  }
  for (its=0;its<ITMAX;its++) {
    iter=its;
    dlinmin(p,xi,fret,net);
    cout << "its = " << its << ", fret = " << fret << ", lhs = " << 2.0*fabs(fret-fp) << ", rhs = " << ftol*(fabs(fret)+fabs(fp)+EPS) << endl;
    if (2.0*fabs(fret-fp) <= ftol*(fabs(fret)+fabs(fp)+EPS))
      return;
    fp=fret;
    net->dE(p,xi);
    dgg=gg=0.0;
    for (j=0;j<n;j++) {
      gg += g[j]*g[j];
      //		  dgg += xi[j]*xi[j];
      dgg += (xi[j]+g[j])*xi[j];
    }
    if (gg == 0.0) {
      cout << "g[0] = " << g[0] << endl;
      return;
    }
    gam=dgg/gg;
    for (j=0;j<n;j++) {
      g[j] = -xi[j];
      xi[j]=h[j]=g[j]+gam*h[j];
    }
  }
  cerr << "too many iterations in frprmn" << endl;
  return;
}

int ncom;
Vec_DP *pcom_p,*xicom_p;

void dlinmin(Vec_IO_DP &p, Vec_IO_DP &xi, DP &fret, neu_net *net)
{
  const DP TOL=2.0e-8;
  int j;
  DP xx,xmin,fx,fb,fa,bx,ax;

  int n=p.size();
  ncom=n;
  pcom_p=new Vec_DP(n);
  xicom_p=new Vec_DP(n);
  Vec_DP &pcom=*pcom_p,&xicom=*xicom_p;
  for (j=0;j<n;j++) {
    pcom[j]=p[j];
    xicom[j]=xi[j];
  }
  ax=0.0;
  xx=1.0;
  mnbrak(ax,xx,bx,fa,fx,fb,f1dim, net);
  fret=dbrent(ax,xx,bx,f1dim,df1dim,TOL,xmin, net);
  for (j=0;j<n;j++) {
    xi[j] *= xmin;
    p[j] += xi[j];
  }
  delete xicom_p;
  delete pcom_p;
}

namespace {
  inline void mov3(DP &a, DP &b, DP &c, const DP d, const DP e,
		   const DP f)
  {
    a=d; b=e; c=f;
  }
}

DP dbrent(const DP ax, const DP bx, const DP cx, DP f(const DP, neu_net *),
	  DP df(const DP, neu_net *), const DP tol, DP &xmin, neu_net *net)
{
  const int ITMAX=100;
  const DP ZEPS=1.e-16*1.0e-3;
  bool ok1,ok2;
  int iter;
  DP a,b,d=0.0,d1,d2,du,dv,dw,dx,e=0.0;
  DP fu,fv,fw,fx,olde,tol1,tol2,u,u1,u2,v,w,x,xm;

  a=(ax < cx ? ax : cx);
  b=(ax > cx ? ax : cx);
  x=w=v=bx;
  fw=fv=fx=f(x, net);
  dw=dv=dx=df(x, net);
  //cout << "fw = " << fw << ", dw = " << dw << endl;
  for (iter=0;iter<ITMAX;iter++) {
    xm=0.5*(a+b);
    tol1=tol*fabs(x)+ZEPS;
    tol2=2.0*tol1;
    if (fabs(x-xm) <= (tol2-0.5*(b-a))) {
      xmin=x;
      return fx;
    }
    if (fabs(e) > tol1) {
      d1=2.0*(b-a);
      d2=d1;
      if (dw != dx) d1=(w-x)*dx/(dx-dw);
      if (dv != dx) d2=(v-x)*dx/(dx-dv);
      u1=x+d1;
      u2=x+d2;
      ok1 = (a-u1)*(u1-b) > 0.0 && dx*d1 <= 0.0;
      ok2 = (a-u2)*(u2-b) > 0.0 && dx*d2 <= 0.0;
      olde=e;
      e=d;
      if (ok1 || ok2) {
	if (ok1 && ok2)
	  d=(fabs(d1) < fabs(d2) ? d1 : d2);
	else if (ok1)
	  d=d1;
	else
	  d=d2;
	if (fabs(d) <= fabs(0.5*olde)) {
	  u=x+d;
	  if (u-a < tol2 || b-u < tol2)
	    d=SIGN(tol1,xm-x);
	} else {
	  d=0.5*(e=(dx >= 0.0 ? a-x : b-x));
	}
      } else {
	d=0.5*(e=(dx >= 0.0 ? a-x : b-x));
      }
    } else {
      d=0.5*(e=(dx >= 0.0 ? a-x : b-x));
    }
    if (fabs(d) >= tol1) {
      u=x+d;
      fu=f(u, net);
    } else {
      u=x+SIGN(tol1,d);
      fu=f(u, net);
      if (fu > fx) {
	xmin=x;
	return fx;
      }
    }
    du=df(u, net);
    if (fu <= fx) {
      if (u >= x) a=x; else b=x;
      mov3(v,fv,dv,w,fw,dw);
      mov3(w,fw,dw,x,fx,dx);
      mov3(x,fx,dx,u,fu,du);
    } else {
      if (u < x) a=u; else b=u;
      if (fu <= fw || w == x) {
	mov3(v,fv,dv,w,fw,dw);
	mov3(w,fw,dw,u,fu,du);
      } else if (fu < fv || v == x || v == w) {
	mov3(v,fv,dv,u,fu,du);
      }
    }
  }
  cerr << "Too many iterations in routine dbrent" << endl;
  return 0.0;
}

namespace {
  inline void shft3(DP &a, DP &b, DP &c, const DP d)
  {
    a=b;
    b=c;
    c=d;
  }
}

void mnbrak(DP &ax, DP &bx, DP &cx, DP &fa, DP &fb, DP &fc,
	    DP func(const DP, neu_net *), neu_net *net)
{
  const DP GOLD=1.618034,GLIMIT=100.0,TINY=1.0e-20;
  DP ulim,u,r,q,fu;

  fa=func(ax, net);
  fb=func(bx, net);
  if (fb > fa) {
    SWAP(ax,bx);
    SWAP(fb,fa);
  }
  cx=bx+GOLD*(bx-ax);
  fc=func(cx, net);
  while (fb > fc) {
    r=(bx-ax)*(fb-fc);
    q=(bx-cx)*(fb-fa);
    u=bx-((bx-cx)*q-(bx-ax)*r)/
      (2.0*SIGN(MAX(fabs(q-r),TINY),q-r));
    ulim=bx+GLIMIT*(cx-bx);
    if ((bx-u)*(u-cx) > 0.0) {
      fu=func(u, net);
      if (fu < fc) {
	ax=bx;
	bx=u;
	fa=fb;
	fb=fu;
	return;
      } else if (fu > fb) {
	cx=u;
	fc=fu;
	return;
      }
      u=cx+GOLD*(cx-bx);
      fu=func(u, net);
    } else if ((cx-u)*(u-ulim) > 0.0) {
      fu=func(u, net);
      if (fu < fc) {
	shft3(bx,cx,u,cx+GOLD*(cx-bx));
	shft3(fb,fc,fu,func(u, net));
      }
    } else if ((u-ulim)*(ulim-cx) >= 0.0) {
      u=ulim;
      fu=func(u, net);
    } else {
      u=cx+GOLD*(cx-bx);
      fu=func(u, net);
    }
    shft3(ax,bx,cx,u);
    shft3(fa,fb,fc,fu);
  }
}

DP df1dim(const DP x, neu_net *net)
{
  int j;
  DP df1=0.0;
  Vec_DP xt(ncom),df(ncom);

  Vec_DP &pcom=*pcom_p,&xicom=*xicom_p;
  for (j=0;j<ncom;j++) xt[j]=pcom[j]+x*xicom[j];
  net->dE(xt,df);
  for (j=0;j<ncom;j++) df1 += df[j]*xicom[j];
  return df1;
}

DP f1dim(const DP x, neu_net *net)
{
  int j;

  Vec_DP xt(ncom);
  Vec_DP &pcom=*pcom_p,&xicom=*xicom_p;
  for (j=0;j<ncom;j++)
    xt[j]=pcom[j]+x*xicom[j];
  return net->EE(xt);
}

void NR::sort(Vec_IO_DP &arr)
{
  const int M=7,NSTACK=50;
  int i,ir,j,k,jstack=-1,l=0;
  DP a;
  Vec_INT istack(NSTACK);
  
  int n=arr.size();
  ir=n-1;
  for (;;) {
    if (ir-l < M) {
      for (j=l+1;j<=ir;j++) {
	a=arr[j];
	for (i=j-1;i>=l;i--) {
	  if (arr[i] <= a) break;
	  arr[i+1]=arr[i];
	}
	arr[i+1]=a;
      }
      if (jstack < 0) break;
      ir=istack[jstack--];
      l=istack[jstack--];
    } else {
      k=(l+ir) >> 1;
      SWAP(arr[k],arr[l+1]);
      if (arr[l] > arr[ir]) {
	SWAP(arr[l],arr[ir]);
      }
      if (arr[l+1] > arr[ir]) {
	SWAP(arr[l+1],arr[ir]);
      }
      if (arr[l] > arr[l+1]) {
	SWAP(arr[l],arr[l+1]);
      }
      i=l+1;
      j=ir;
      a=arr[l+1];
      for (;;) {
	do i++; while (arr[i] < a);
	do j--; while (arr[j] > a);
	if (j < i) break;
	SWAP(arr[i],arr[j]);
      }
      arr[l+1]=arr[j];
      arr[j]=a;
      jstack += 2;
      if (jstack >= NSTACK) nrerror("NSTACK too small in sort.");
      if (ir-i+1 >= j-l) {
	istack[jstack]=ir;
	istack[jstack-1]=i;
	ir=j-1;
      } else {
	istack[jstack]=j-1;
	istack[jstack-1]=l;
	l=i;
      }
    }
  }
}

void NR::indexx(Vec_I_DP &arr, Vec_O_INT &indx)
{
  const int M=7,NSTACK=50;
  int i,indxt,ir,j,k,jstack=-1,l=0;
  DP a;
  Vec_INT istack(NSTACK);
  
  int n=arr.size();
  ir=n-1;
  for (j=0;j<n;j++) indx[j]=j;
  for (;;) {
    if (ir-l < M) {
      for (j=l+1;j<=ir;j++) {
	indxt=indx[j];
	a=arr[indxt];
	for (i=j-1;i>=l;i--) {
	  if (arr[indx[i]] <= a) break;
	  indx[i+1]=indx[i];
	}
	indx[i+1]=indxt;
      }
      if (jstack < 0) break;
      ir=istack[jstack--];
      l=istack[jstack--];
    } else {
      k=(l+ir) >> 1;
      SWAP(indx[k],indx[l+1]);
      if (arr[indx[l]] > arr[indx[ir]]) {
	SWAP(indx[l],indx[ir]);
      }
      if (arr[indx[l+1]] > arr[indx[ir]]) {
	SWAP(indx[l+1],indx[ir]);
      }
      if (arr[indx[l]] > arr[indx[l+1]]) {
	SWAP(indx[l],indx[l+1]);
      }
      i=l+1;
      j=ir;
      indxt=indx[l+1];
      a=arr[indxt];
      for (;;) {
	do i++; while (arr[indx[i]] < a);
	do j--; while (arr[indx[j]] > a);
	if (j < i) break;
	SWAP(indx[i],indx[j]);
      }
      indx[l+1]=indx[j];
      indx[j]=indxt;
      jstack += 2;
      if (jstack >= NSTACK) nrerror("NSTACK too small in indexx.");
      if (ir-i+1 >= j-l) {
	istack[jstack]=ir;
	istack[jstack-1]=i;
	ir=j-1;
      } else {
	istack[jstack]=j-1;
	istack[jstack-1]=l;
	l=i;
      }
    }
  }
}

void NR::indexx(Vec_I_INT &arr, Vec_O_INT &indx)
{
  const int M=7,NSTACK=50;
  int i,indxt,ir,j,k,jstack=-1,l=0;
  int a;
  Vec_INT istack(NSTACK);
  
  int n=arr.size();
  ir=n-1;
  for (j=0;j<n;j++) indx[j]=j;
  for (;;) {
    if (ir-l < M) {
      for (j=l+1;j<=ir;j++) {
	indxt=indx[j];
	a=arr[indxt];
	for (i=j-1;i>=l;i--) {
	  if (arr[indx[i]] <= a) break;
	  indx[i+1]=indx[i];
	}
	indx[i+1]=indxt;
      }
      if (jstack < 0) break;
      ir=istack[jstack--];
      l=istack[jstack--];
    } else {
      k=(l+ir) >> 1;
      SWAP(indx[k],indx[l+1]);
      if (arr[indx[l]] > arr[indx[ir]]) {
	SWAP(indx[l],indx[ir]);
      }
      if (arr[indx[l+1]] > arr[indx[ir]]) {
	SWAP(indx[l+1],indx[ir]);
      }
      if (arr[indx[l]] > arr[indx[l+1]]) {
	SWAP(indx[l],indx[l+1]);
      }
      i=l+1;
      j=ir;
      indxt=indx[l+1];
      a=arr[indxt];
      for (;;) {
	do i++; while (arr[indx[i]] < a);
	do j--; while (arr[indx[j]] > a);
	if (j < i) break;
	SWAP(indx[i],indx[j]);
      }
      indx[l+1]=indx[j];
      indx[j]=indxt;
      jstack += 2;
      if (jstack >= NSTACK) nrerror("NSTACK too small in indexx.");
      if (ir-i+1 >= j-l) {
	istack[jstack]=ir;
	istack[jstack-1]=i;
	ir=j-1;
      } else {
	istack[jstack]=j-1;
	istack[jstack-1]=l;
	l=i;
      }
    }
  }
}



void NR::hpsel(Vec_I_DP &arr, Vec_O_DP &heap)
{
  int i,j,k;
  
  int m=heap.size();
  int n=arr.size();
  if (m > n/2 || m < 1) nrerror("probable misuse of hpsel");
  for (i=0;i<m;i++) heap[i]=arr[i];
  sort(heap);
  for (i=m;i<n;i++) {
    if (arr[i] > heap[0]) {
      heap[0]=arr[i];
      for (j=0;;) {
	k=(j << 1)+1;
	if (k > m-1) break;
	if (k != (m-1) && heap[k] > heap[k+1]) k++;
	if (heap[j] <= heap[k]) break;
	SWAP(heap[k],heap[j]);
	j=k;
      }
    }
  }
}

void sort2(Vec_IO_DP &arr, Vec_IO_INT &brr)
{
  const int M=7,NSTACK=50;
  int i,ir,j,k,jstack=-1,l=0;
  DP a;
  int b;
  Vec_INT istack(NSTACK);
  
  int n=arr.size();
  ir=n-1;
  for (;;) {
    if (ir-l < M) {
      for (j=l+1;j<=ir;j++) {
	a=arr[j];
	b=brr[j];
	for (i=j-1;i>=l;i--) {
	  if (arr[i] <= a) break;
	  arr[i+1]=arr[i];
	  brr[i+1]=brr[i];
	}
	arr[i+1]=a;
	brr[i+1]=b;
      }
      if (jstack < 0) break;
			ir=istack[jstack--];
			l=istack[jstack--];
    } else {
      k=(l+ir) >> 1;
      SWAP(arr[k],arr[l+1]);
      SWAP(brr[k],brr[l+1]);
      if (arr[l] > arr[ir]) {
	SWAP(arr[l],arr[ir]);
	SWAP(brr[l],brr[ir]);
      }
      if (arr[l+1] > arr[ir]) {
	SWAP(arr[l+1],arr[ir]);
	SWAP(brr[l+1],brr[ir]);
      }
      if (arr[l] > arr[l+1]) {
	SWAP(arr[l],arr[l+1]);
	SWAP(brr[l],brr[l+1]);
      }
      i=l+1;
      j=ir;
      a=arr[l+1];
      b=brr[l+1];
      for (;;) {
	do i++; while (arr[i] < a);
	do j--; while (arr[j] > a);
	if (j < i) break;
	SWAP(arr[i],arr[j]);
	SWAP(brr[i],brr[j]);
      }
      arr[l+1]=arr[j];
      arr[j]=a;
      brr[l+1]=brr[j];
      brr[j]=b;
      jstack += 2;
      if (jstack >= NSTACK) {
	cerr << "NSTACK too small..." << endl;
	return;
      }
      if (ir-i+1 >= j-l) {
	istack[jstack]=ir;
	istack[jstack-1]=i;
	ir=j-1;
      } else {
	istack[jstack]=j-1;
	istack[jstack-1]=l;
	l=i;
      }
    }
  }
}


void hpselndx(Vec_I_DP &arr, Vec_O_INT &heapindx)
{
  int i,j,k;
  
  int m=heapindx.size();
  int n=arr.size();
  Vec_DP heap(m);
  /*
  if (m > n/2 || m < 1) {
    cerr << "probable misuse of hpsel" << endl;
    return;
  }
  */
  for (i=0;i<m;i++) {
    heap[i]=arr[i];
    heapindx[i] = i;
  }
  sort2(heap, heapindx);
  for (i=m;i<n;i++) {
    if (arr[i] > heap[0]) {
      heap[0]=arr[i];
      heapindx[0]=i;
      for (j=0;;) {
	k=(j << 1)+1;
	if (k > m-1) break;
	if (k != (m-1) && heap[k] > heap[k+1]) k++;
	if (heap[j] <= heap[k]) break;
	SWAP(heap[k],heap[j]);
	SWAP(heapindx[k], heapindx[j]);
	j=k;
      }
    }
  }
}

#include "nr.h"

DP NR::ran2(int &idum)
{
  const int IM1=2147483563,IM2=2147483399;
  const int IA1=40014,IA2=40692,IQ1=53668,IQ2=52774;
  const int IR1=12211,IR2=3791,NTAB=32,IMM1=IM1-1;
  const int NDIV=1+IMM1/NTAB;
  const DP EPS=3.0e-16,RNMX=1.0-EPS,AM=1.0/DP(IM1);
  static int idum2=123456789,iy=0;
  static Vec_INT iv(NTAB);
  int j,k;
  DP temp;
  
  if (idum <= 0) {
    idum=(idum==0 ? 1 : -idum);
    idum2=idum;
    for (j=NTAB+7;j>=0;j--) {
      k=idum/IQ1;
      idum=IA1*(idum-k*IQ1)-k*IR1;
      if (idum < 0) idum += IM1;
      if (j < NTAB) iv[j] = idum;
    }
    iy=iv[0];
  }
  k=idum/IQ1;
  idum=IA1*(idum-k*IQ1)-k*IR1;
  if (idum < 0) idum += IM1;
  k=idum2/IQ2;
  idum2=IA2*(idum2-k*IQ2)-k*IR2;
  if (idum2 < 0) idum2 += IM2;
  j=iy/NDIV;
  iy=iv[j]-idum2;
  iv[j] = idum;
  if (iy < 1) iy += IMM1;
  if ((temp=AM*iy) > RNMX) return RNMX;
  else return temp;
}
