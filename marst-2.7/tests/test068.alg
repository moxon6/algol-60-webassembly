begin

    comment library 1;

    integer
        i;
    switch s :=
        s1,
        s2,
        s3,
        s4;
    for i := 1 step 1 until 4 do
        begin
        go to s[i];
        s1 :;
        if i != 1 then
            outreal(1, i)
        else
            go to exit;
        s2 :;
        if i != 2 then
            outreal(1, i)
        else
            go to exit;
        s3 :;
        if i != 3 then
            outreal(1, i)
        else
            go to exit;
        s4 :;
        if i != 4 then
            outreal(1, i)
        else
            go to exit;
        exit :;
        end;
end
