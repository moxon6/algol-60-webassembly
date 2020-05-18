begin

    comment library 1;
    integer
        i,
        j;
    i := j := 0;
    i :=
        if i = 0 then

                if j = 0 then
                    1
                else
                    2
        else
            3;
    if i = 1 then
        if j = 0 then
            j := 2
        else
            j := 3
        else
            j := 4;
    outreal(1, i);
    outreal(1, j)
end
