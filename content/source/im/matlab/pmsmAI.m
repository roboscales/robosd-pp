function pmsmAI
syms ws w Fm Kv Ls J
A=[ -ws w 0;..
    -w -ws -Fm/Ls;..
    0 Fm/J -Kv/J];
IA = simplify( inv(A))*(Fm^2*ws + Kv*Ls*w^2 + Kv*Ls*ws^2)


