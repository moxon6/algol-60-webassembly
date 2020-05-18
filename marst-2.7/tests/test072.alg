begin

    comment library 3;

    integer
        i,
        case;
    Boolean
        b;
    real
        x;
    integer procedure p(k, q);
        value
            k;
        integer
            k;
        integer procedure
            q;
        p :=
            if k = 1 then
                q(i)
            else if k = 2 then
                q
            else if k = 3 then
                q(x)
            else
                q(q(20));

    integer procedure q1(i);
        value
            i;
        integer
            i;
        q1 := 100 + i;

    integer procedure q2;
        q2 := 200 + (if b then 1 else 0);

    integer procedure q3(x);
        value
            x;
        real
            x;
        q3 := 300 + x;

    procedure fail(m, n);
        value
            m,
            n;
        integer
            m,
            n;
        if m != n then
            begin
            outreal(1, m);
            outreal(1, n);
            outreal(1, case)
            end
        else
            case := case + 1;

    i := 1;
    x := 10.4;
    case := 0;
    b := false;
    fail(p(1, q1), 101);
    fail(p(4, q1), 220);
    fail(p(2, q2), 200);
    fail(p(3, q3), 310);
    fail(p(4, q3), 620);
    outreal(1, case)
end
