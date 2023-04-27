syms a b T
A=[a b; 1 0];
D = [1 0; 0 1] + A*T + A^2*T*T/2 + A^3*T*T*T/6;
D = simplify(D)
simplify(A^-1)
J = 1e-4;
Kv = 0.0012;
A=[-Kv/J -1/J; 1 0];
B=[0;1];
Xm=-(A^-1)*B;
T=0.01;
D = expm(A*T);

% a=A(1,1); b= A(1,2);
% [T*a + (T^2*(a^2 + b))/2 + (T^3*(a*b + a*(a^2 + b)))/6 + 1, (T*b*(T^2*a^2 + b*T^2 + 3*T*a + 6))/6]
% [                        T + (T^2*a)/2 + (T^3*(a^2 + b))/6,           (a*b*T^3)/6 + (b*T^2)/2 + 1]