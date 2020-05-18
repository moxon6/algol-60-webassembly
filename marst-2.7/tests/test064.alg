begin

    comment library 1;
    integer procedure p(b);
        value
            b;
        Boolean
            b;
        p := (if b then 1 else 3.3) % 6;

    outreal(1, p(true));
    outreal(1, p(false))
end
