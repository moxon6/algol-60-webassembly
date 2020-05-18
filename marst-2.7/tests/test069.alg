begin

    comment library 1;

    integer
        i;
    switch s :=
        s1,
        s2,
        s3,
        s4;
    for i := 1,
        2,
        3 do
        begin
        go to s[i];
        s1 :;
        if i != 1 then
            outreal(1, i)
        else
            s2 : if i != 2 then
                outreal(1, i)
        end;
    s3 :;
    outreal(1, i);
    go to s[i + 1];
    s4 :;
    outreal(1, i)
end
