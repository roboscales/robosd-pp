syms Ko dK Ao dA y1 x1  x2 y2 g1 g2

s1 = -y1 + (Ao-dA) + (Ko-dK)*x1;
s2 = -y2 + (Ao-dA) + (Ko-dK)*x2;
s3 = -g1 + Ao + Ko*x1;
s4 = -g2 + Ao + Ko*x2;

r = solve(s1,s2,s3,s4,Ao,dA,Ko,dK)

simplify(r.dA/r.Ao)
simplify(r.dK/r.Ko)


syms z m g r x T u Kv dx U

% dx^2 = -Kv*dx 1/(m*r*r)  -


s1 = -dx + (U-x*m*g*r - Kv*dx)/(m*r*r)*T*(z+1)/2/(z-1);
% s3 = -u -x - Kv*dx;
dx = solve(s1,dx);
simplify(dx)

r = collect(dx*T*(z+1)/2/(z-1),z)-x;
x = collect(solve(r,x),z)
m=0.3;
Kv=0.03;
r=0.2;
T=0.05;

g*K +K1*g+K2*g=g
K +K1+K2=1

