begin
    switch path :=
        p1,
        p2;
    integer
        n,
        i;
    i := 0;
    label 1 :;
    i := i + 1;
    if i = 2 then
        go to stop;
    n := 0;
    go to
        if p > 0 then
            path(n)
        else
            label 1;
    p1 := p - 1;
    p2 := p - 1;
    stop :;
end
