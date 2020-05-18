real procedure cputime;
inline("my_dsa.retval.u.real_val = (double)clock() / (double)CLOCKS_PER_SEC;");

begin

    comment library 3;


    real procedure a(k, x1, x2, x3, x4, x5);
        value
            k;
        integer
            k;
        real
            x1,
            x2,
            x3,
            x4,
            x5;
        begin
        real procedure b;
            begin
            k := k - 1;
            b := a := a(k, b, x1, x2, x3, x4)
            end;

        if k <= 0 then
            a := x4 + x5
        else if b < 0 then

        end;

    outreal(1, cputime);
    outreal(1, a(10, 1, - 1, - 1, 1, 0));
    outreal(1, cputime);
end
