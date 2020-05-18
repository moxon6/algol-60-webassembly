real procedure cputime;
inline("my_dsa.retval.u.real_val = (double)clock() / (double)CLOCKS_PER_SEC;");

begin

    comment library 1;


    integer
        i,
        j,
        k,
        k1;
    real
        t1,
        t2;
    integer procedure ackermann(m, n);
        value
            m,
            n;
        integer
            m,
            n;
        ackermann :=
            if m = 0 then
                n + 1
            else if n = 0 then
                ackermann(m - 1, 1)
            else
                ackermann(m - 1, ackermann(m, n - 1));

    k := 16;
    k1 := 1;
    for i := 1 step 1 until 6 do
        begin
        t1 := cputime;
        j := ackermann(3, i);
        t2 := cputime;
        if j != k - 3 then
            begin
            outreal(1, i);
            outreal(1, j)
            end;
        outreal(1, t2 - t1);
        outreal(1, 3 * (t2 - t1) / (512 * k1 - 15 * k + 9 * i + 37) );
        k1 := 4 * k1;
        k := 2 * k;
        end
end
