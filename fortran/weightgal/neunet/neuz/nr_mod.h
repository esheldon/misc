void dfpmin(Vec_IO_DP &p, const DP gtol, int &iter, DP &fret, neu_net *net);
void lnsrch(Vec_I_DP &xold, const DP fold, Vec_I_DP &g, Vec_IO_DP &p,
	    Vec_O_DP &x, DP &f, const DP stpmax, bool &check, neu_net *net);

void frprmn(Vec_IO_DP &p, const DP ftol, int &iter, DP &fret,
	    neu_net *net);
void dlinmin(Vec_IO_DP &p, Vec_IO_DP &xi, DP &fret, neu_net *net);
DP dbrent(const DP ax, const DP bx, const DP cx, DP f(const DP, neu_net *),
	  DP df(const DP, neu_net *), const DP tol, DP &xmin, neu_net *net);
void mnbrak(DP &ax, DP &bx, DP &cx, DP &fa, DP &fb, DP &fc,
	    DP func(const DP, neu_net *), neu_net *net);
DP df1dim(const DP x, neu_net *net);
DP f1dim(const DP x, neu_net *net);

