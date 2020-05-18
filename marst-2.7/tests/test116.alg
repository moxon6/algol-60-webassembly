begin
    comment library 1;

    comment   use of a procedure;

    real
        a,
        b,
        ;
    procedure example(x, y)result : (r);
        value
            x,
            y;
        real
            x,
            y,
            r;
        begin
        x := x + y;
        r := x^2 + y^2
        end;

    a := 1;
    b := 2;
    example(a, a + b, r);
outreal(1,r)
end
