function pmsm
syms ws w Kv J Fm U Ms  Ls

A=[ -ws w 0; -w -ws -Fm; 0 Fm/J -Kv/J]
IA =simplify(A^-1);
IIA = IA*(Fm^2*ws +Kv*w^2 + Kv*ws^2)
pretty(IA)

Xm = simplify(-A^-1*[0; U/Ls; -Ms/J])



