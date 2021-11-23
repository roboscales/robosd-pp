function dX = pmsm_model(U,X, Rs,Ls,Fm,J, Kv)
id = X(1);
iq = X(2);
w = X(3);
nws =-Rs/Ls;
dX = [ nws*id+w*iq+U(1)/Ls, 	nws*iq-w*id-w*Fm/Ls+U(2)/Ls,  Fm/J*iq-U(3)/J-Kv*w/J];

